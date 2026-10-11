# Exception handling

Have orderbook API return status code to represent success / failure

```c
[[nodiscard]] OrderbookError AddOrder(...);
```

- so that call sites have to check the success of the operation

# Benchmark

During benchmark store timestamp in array indexed by OrderId

```c
// in Benchmark
using OrderId = int64_t;
using Timestamp = int64_t;

std::unordered_map<OrderId, Timestamp> order_ingress_times;

// when creating order
order_ingress_times[order.id] = std::chrono::steady_clock::now().time_since_epoch().count();

// when order matches and returns Execution report:
uint64_t latency = current_time - order_ingress_times[exec_report.order_id];
```

Why?

- by not storing timestamp inside Orderbook implementation data structure, we avoid reading the timestamp from memory during the matching loop, which wastes CPU cycles and pollutes the cache
- Order struct shrinks 8 bytes, as a result our `std::list` nodes are smaller, thus less memory footprint
- so that matching engine loop becomes purely about price/volume, with fewer cache lines touched during the matching loop

# AddOrder / HandleFill Interaction

Initially the shape of the AddOrder and HandleFill interaction was to add the order first, and passed the iterator of the added order to HandleFill

- as a result of this if there was a partial fill or a full fill I would have to remove the order I just added
- implemneted a better approach to FillOrder first based on the incoming order, and then use the MatchResult returned from FillOrder to determine whether or not to add the order

# Self trade prevention

The engine implements a "Skip" (ignore) Self-Trade prevention Policy during the matching loop, prioritizing absolute zero hot-path overhead over strict price-time priority preservation for self-matching accounts.

- During HandleFill, if a resting order's trader_id matches incoming order's trader_id (whether incoming order is Bid or Ask), the engine advances the iterator without executing a fill
- The skipped resting orders remain intact and visible in the orderbook, allowing other market participants to match against it
- While it leaves a "phantom" liquidity for the incoming order and slightly deviates from strict price-time priority, it guarantees maximum matching throughput

# Event log

maintain unified "Event Log" that audits all operations such as Add/Cancel/ModifyOrder, as well as all the fills that happened and any OrderbookErrors that happened for a given event

```c
struct alignas(64) OrderEvent {
  EventType type;
  uint64_t ts;
  OrderId orderId;
  TraderId traderId;
  Side side;
  Price price;
  Volume volume;
  OrderbookError orderbookError;
  OrderStatus orderStatus;
  Volume filledVolume;
  Volume restingVolume;
};
```

Why?
Performance & Cache Locality

- instead of 2 separate logs for operations and execution fills, maintaining a single append-only log is significantly faster and more cache-friendly than managing and syncrhonizing 2 separate data structures

Deterministic Replay:

- by replaying this single stream of events, can perfect reconstruct the entire orderbook state at any point of time

`alignas(64)`
one event per cache line. The engine is single-threaded, so this is a scan property, not a false-sharing claim — no claim is made about concurrent writers, because there are none.

- modern CPUs transfer data between main RAM and L1/L2/L3 caches in fixed 64-byte cache lines
- an event smaller than a line shares that line with its neighbour, so reading one event fetches bytes it does not own
- padded to a line, one fetch is one event, and no event is ever split across two lines

Use Factory functions to give compile-time safety 

AddEvent stores:
:

```c
 OrderEvent{.ts = ts,
                    .orderId = id,
                    .price = price,
                    .traderId = traderId,
                    .volume = volume,
                    .filledVolume = matchResult.filledVolume,
                    .restingVolume = matchResult.restingVolume,
                    .type = EventType::Add,
                    .side = side,
                    .orderbookError = matchResult.error,
                    .orderStatus = matchResult.status};
```

DeleteEvent stores:

```c
OrderEvent{.ts = ts, .orderId = id, .type = EventType::Cancel, .orderbookError = orderbookError};
```

ModifyEvent stores

```c
OrderEvent{.ts = ts,
                    .orderId = id,
                    .volume = newVolume,
                    .type = EventType::Modify,
                    .orderbookError = orderbookError};
```

# Orderbook API (Vector and Map implementation)
```c
[[nodiscard]] MatchResult AddOrder(OrderId orderId, Side side, Price price, Volume volume, TraderId traderId);
[[nodiscard]] OrderbookError ModifyOrder(OrderId orderId, Volume newVolume);
[[nodiscard]] OrderbookError DeleteOrder(OrderId orderId);
```

# Orderbook Reverse Vector implementation

## Data structure
```c
std::vector<std::pair<Price, PriceLevel>> bidLevels; // best bid at end
std::vector<std::pair<Price, PriceLevel>> askLevels; // best ask at end
std::unordered_map<OrderId, std::list<Order>::iterator> idMap;
```
why reverse vector with best bid/ask at end of vector?
- because actions are happening most on the top of the book (highest for bid, lowest for ask)
- this means for the data structure if we put best price at index 0, then we are going to have to constantly shift our elements of our vector in memory all the time
- by putting our "best price at the end of the vector, we minimize the number of copies

## Binary Search vs Linear Search implementation
To find where in our bidLevels/askLevels to insert a new <Price, PriceLevel> pair, or where to insert a new order (by finding the price that the order it belongs to)
- we can perform binary search or linear search

I will implement different look behaviors in AddOrder through template param
- Binary Search via std::lower_bound()
- Branchless Binary Search
  - no early exit anymore, going to go through the entire collection no matter what, touching more memory
- Linear Search 

and test their Latency Distribution, to see the behavioral differences of these approaches and verify David Gross's assertion from his CppCon 2024 talk "When Nanoseconds Matter: Ultrafast Trading Systems in C++"
- where he showed that linear search achieved the fastest performance due to the algorithim being in harmony with the hardware
  - cache locality / the way it accesses the memory 


For our search policies
```c
enum class SearchResult{
  found;
  notFound;
}
```

- given a searchPrice and a std::vector<std::pair<Price, PriceLevel>> Level returns <SearchResult, iterator> 
- if searchPrice exists: it should return <SearchResult::found, std::pair<Price, PriceLevel>::iterator>
- if searchPrice doesn't exist: return <SearchResult::notFound, std::pair<Price, PriceLevel>::iterator>
  - where if need to insert the price, insert it before the returned iterator

# Performance refactor plan

## Intrusive
- intrusive container stores its linkage pointers inside the element itself, not in a separate node
Current implementation: Non-intrusive `std::list<Order>`
```c
node {Order data; prev*; next*; }
```
- node allocated separately, holds a copy/move of Order

Intrusive:
```c
struct Order {
  OrderId id;
  Price price;
  Side side;
  TraderId trader_id;
  Volume volume;
  Order* prev;
  Order* next;
}
```
- linkage lives in the Order

`std::list<Order> orders` becomes a pair of pointers (head, tail) to Orders
- no wrapper node, no separate allocation for the linkage

Why?
- One allocation per order, not two. 
- No copying: The order allocated is the order in the queue
- O(1) removal from middle (Given an Order*, can unlink in O(1) without searching the queue)
- Cache-friendly: Pointer travels with the data

## Polymorphic Memory Resource (C++17) (std::pmr)
way to plug a custom allocator into a container without changing the container type
```c
std::pmr::monotonic_buffer_resource pool { size };
std::pmr::list<Order> queue{&pool};
```
Why?
- hot-path allocation avoidance. By pre-allocating a big buffer once, every order allocation comes from that buffer in O(1) with no malloc, no locks, no syscalls

## Pooled
create a pool memory resource: a pre-allocated slab of memory that hands out fixed-size chunks.
- For an intrusive queue of Orders, the pool is sized to hold N orders
```c
Order* order = pool.allocate(); // to allocate
```

# Workload & Validation Harness
NASDAQ ITCH 5.0 feed handler: parser → SPSC ring → book builder; measured handoff cost at [X]ns and pipelined throughput at [N]M msgs/sec vs. [M]M single-threaded; book-state hashes identical across modes

```
ITCH file → Parser thread → [SPSC ring] → Book builder thread → Orderbook
```
Why SPSC?
- single-producer (parser) single consumer (orderbook)
- parsing is burst (message-type dependent); book updates are steady-state, the ring can absorb bursts
- parser can run ahead while book thread applies the previous batch  
- Zero allocation on handoff: preallocated ring, fixed-size slots. No malloc on hot path

What to measure:
|Measurement|Why|
|-----------|--|
Parse-to-book p50/p99/p99.9, single-threaded vs SPSC-pipelined| Does ring actually help?
Ring handoff cost (ns/msg) | overhead paid
Throughput (msg/sec) both modes | does pipelining improve throughput
Ring occupancy distribution | is the producer or consumer the bottleneck?
Cycles/msg producer, cycles/msg consumer | where the time goes
Book-state hashes, both modes | correctness
