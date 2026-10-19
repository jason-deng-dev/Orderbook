# Orderbook — Matching Engine (C++23)

Price-time priority matching engine with self-trade prevention, partial-fill, and
cancellation semantics, written from scratch as a study in container choice, memory
layout, and the measurement discipline behind low-latency claims.

## Quick start

```bash
cmake --preset default
cmake --build build
ctest --test-dir build --output-on-failure
```

## Benchmark
```bash
cmake --preset default -DCMAKE_BUILD_TYPE=Release
cmake --build build
taskset -c 2,4 ./build/benchmarks --benchmark_filter=BM_Orderbook --benchmark_repetitions=25 --benchmark_report_aggregates_only=true
```
