# Exception handling

For "Imposible" Logic bugs 
- use Standard assert `<cassert>`
- when checking for bug in own code (eg: "this pointer should never be null", "this list should never be empty here" ...)
Why: when compile with `-O3 -DNDEBUG preprocessor replaces assert(...) with ((void)0), so CPU doesn't even know it's there
```c
void Orderbook::match(Order& incoming) {
  assert(!mBidLevels.empty() && "Bid ladder should not be empty here!");
  ...
}
```

For Bad Data from Network (Data feeder)
- If checking for duplicate Order ID or invalid price from client, not bug in code, can't use `assert` sinece can't crash engine in production for bad input
- instead use C++20's `[[unlikely]]` attribute
- it tells CPU's branch predictor "assume this conditiion is false 99.9% of the time, Keep the 'happy path' code in the fast instruction cache"
```c
#include <expected>

std::expected<ExecutionReport, OrderError> Orderbook::processOrder(const Order& order) {
    
    // The [[unlikely]] hint tells the CPU to optimize for the case where 
    // the order ID is NOT found. This prevents branch misprediction penalties.
    if (idMap.contains(order.id)) [[unlikely]] {
        return std::unexpected(OrderError::DuplicateOrderId);
    }

    // ... rest of the hot path ...
}
```

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
