# Exception handling

Have orderbook API return status code to represent success / failure

```c
[[nodiscard]] OrderbookError AddOrder(...);
```

- so that call sites have to check the success of the operation

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

## What is the issue with std::map implemenation
std::map is a node-based red-black tree, which causes 3 major hardware bottlenecks on the hot path
- pointer chasing: 
  - leads to L1/L2 cache misses because nodes are scattered across the heap
- TLB (Translation Lookaside Buffer) misses
  - because scattered allocations span multiple virtual memory pages, forcing expensive page-table walks
- branch mispredictions
  - due to the unpredictible if/else tree traversal

A std::vector provides a contiguous memory layout, which maximizes spatial locality, keeping the working set in L1 cache and allowing the CPU's hardware prefetcher to antipicipate the next memory access, virtually eliminating those bottlenecks for the top of the book.

## Why reverse Vector?
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

# Orderbook Slab Allocator Intrusive containers Implementations
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

## Slab allocator
- pre-allocating massive contiguous blocks of memory (slabs/pages) at system startup
- carves these slabs into fixed-size chunks exactly the size of Order struct
  - to allocate an order, pop a pointer off a free-list
  - to deallocate (cancel/fill), push teh pointer back onto the free-list

Result: Zero heap allocation on hot path, zero fragmentation and incredible cache locality because all orders live in contiguous memory arrays

Minimal allocator needs to provide:
1. value_type
2. converting copy constructor (templated)
3. allocate
4. deallocate
5. operator== / operator!=
