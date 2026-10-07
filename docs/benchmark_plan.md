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
- Order struct shrinks 8 bytes, as a result our `std::list` nodes are smaller, meaning more orders are able to fit in a single CPU cache line
- so that matching engine loop becomes purely about price/volume, with zero branch mispredictions or cache misses related to timestamps
