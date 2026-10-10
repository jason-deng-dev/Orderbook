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
by forcing the compielr to pad our struct so its total memory footprint is a multiple of 64 bytes (exact size of a standard CPU cache line), we guarantee that multi-threaded operations do not trigger hardware-level performance penalties

- since modern CPUs transfer data between main RAM and CPU's L1/L2/L3 caches in fixed 64-byte cache lines
- if 2 separate threads write to different variables that happen to sit on the same 64-byte cache line, the CPU cores will constantly invalidate each other's cache to maintain coherence.
- By padding the struct we ensure that each threads' write occupies its own dedicated cache line

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

