# Orderbook — Matching Engine (C++23)

Price-time priority matching engine with self-trade prevention, partial-fill, and cancellation semantics. Built as a learning project to practice C++23 (containers, maps, memory layout) and the domain of market microstructure / matching engines.

## Quick start

```bash
# configure + build (Release)
cmake --preset default -DCMAKE_BUILD_TYPE=Release
cmake --build build

# run tests
ctest --test-dir build --output-on-failure

# run benchmarks (steady-state latency)
./build/orderbook_bench --benchmark_min_time=0.5s --benchmark_report_aggregates_only=true
```

