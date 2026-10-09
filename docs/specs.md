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
Initially the shape of the AddOrder and HandleFill interaction was that I added the order first, and passed the iterator of the order to HandleFill
- as a result of this if there was a partial fill or a full fill I would have to remove the order I just added
- decided that the better approach is to FillOrder first based on hte incoming order, and then use the MatchResult returned from FillOrder to determine whether or not to add the order


