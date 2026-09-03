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

## Project layout

| Path | Contents |
|---|---|
| `include/` | headers — `Orderbook`, `Trader`, `Order`/`Trade` |
| `src/` | implementation |
| `tests/` | gtest suites |
| `benchmarks/` | Google Benchmark harness |
| `docs/` | design rationale, benchmark methodology + results |

## Docs

- [`docs/design-rational.md`](docs/design-rational.md) — design decisions and build-through process
- [`docs/benchmark.md`](docs/benchmark.md) — methodology, measured results, interpretation (incl. root-cause of the sell-vs-buy latency asymmetry)
- [`docs/benchmark_results.md`](docs/benchmark_results.md) — raw per-run output

## Results snapshot

Single-threaded, steady-state, `-O3 -DNDEBUG`, AMD Ryzen 7 9800X3D, clang++ 22. Google Benchmark v1.9.5, 100 repetitions. Latency in ns; buy and sell sides shown separately.

| op | buy p50 | buy p99 | sell p50 | sell p99 |
|---|---|---|---|---|
| add into empty book | 142 | 145 | 147 | 150 |
| taker fill, 1 level | 234 | 239 | 243 | 248 |
| taker sweep, 3 levels | 357 | 364 | 369 | 376 |
| partial fill, remainder rests | 202 | 207 | 210 | 217 |
| cancel, price level kept | 125 | 126 | 129 | 131 |
| cancel, price level erased | 126 | 128 | 133 | 134 |

p99 within ~3% of p50 across all ops.  — full analysis in [`docs/benchmark.md`](docs/benchmark.md).
