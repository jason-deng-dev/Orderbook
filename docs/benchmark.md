# Matching Engine Benchmarks

Per-op latency of the order book matching engine: single-threaded, steady-state, AMD Ryzen 7 9800X3D. Source: [`benchmarks/main.cpp`](../benchmarks/main.cpp). Covers the core ops: add-only (no match), taker full-fill (1-level and 3-level sweep), partial fill with remainder resting, and cancellation (price level kept vs. erased).

## Scope

Table: op | scenario | code path exercised

| Benchmark | Book state before op | Timed op — code path exercised |
|---|---|---|
| `BM_BuyEmptyBook` | empty book | insert: new price level into `std::map`, order into level's order queue, best-bid cache update |
| `BM_BuyCrossSingleLevel` | 1 resting ask at best price | taker full-fill: match one order, fill, erase now-empty level, best-ask update |
| `BM_BuyCrossThreeLevels` | 3 resting ask levels (3 @ 10/20/30) | multi-level walk: taker sweeps all 3 levels in one op — 3 matches, 3 level erases, per-unit refunds |
| `BM_BuyPartialFill` | 1 resting ask, 3 @ 10 | partial fill: 5-unit buy takes 3, remainder rests — match + fill, level survives with leftover order |
| `BM_CancelBuyKeepLevel` | 2 orders resting at same price | cancel front-of-queue order while another order keeps the level alive |
| `BM_CancelBuyEmptyLevel` | 1 order resting at price | cancel last order at level — level erased from `std::map` |

Every benchmark exists as a buy-side and sell-side pair; rows show the buy side. Sells are symmetric against the mirror map.

## Methodology

- Harness: Google Benchmark v1.9.5 (via `FetchContent`, see [`CMakeLists.txt`](../CMakeLists.txt)).
- Steady-state loop: `state.PauseTiming()` between iterations restores balances/inventory/resting orders to the exact pre-op state, so each iteration times the op on an identical book. Setup and state reset are excluded from the measurement.
- `benchmark::DoNotOptimize` guards the book so the optimizer cannot elide the call.
- Latency reported as CPU time (Google Benchmark default); percentiles configured per run (see [Re-running](#re-running)).

The pause/resume pattern is the whole harness; one representative example:

```cpp
for (auto _ : state) {
  ob.buy(&b, 10, 10);                    // timed: taker full-fill
  state.PauseTiming();                   // -- untimed reset begins --
  b.changeBalance(100);                  // refund buyer's locked cash
  s.changeBalance(-100);
  s.changeInventoryAmount(&ob, 10);      // restock seller
  ob.sell(&s, 10, 10);                   // re-post the resting ask
  state.ResumeTiming();                  // -- untimed reset ends --
}
```

## Environment

| | |
|---|---|
| CPU | AMD Ryzen 7 9800X3D (8-core / 16 threads) |
| RAM | 32 GB |
| OS | Arch Linux, kernel 7.0.9 |
| Compiler | g++ 16.1.1 |
| Language standard | C++23 |
| Build | CMake 4.3.3 + Ninja, Release preset (`-O3 -DNDEBUG`) |
| Benchmark lib | Google Benchmark v1.9.5, `benchmark::benchmark_main` |
| Run date | _fill in_ |

No CPU pinning and default DVFS/turbo mean absolute numbers drift between runs; relative ordering of ops and the shape of the p50/p99/p99.9 spread are the stable takeaways.

## Results

Measured with (Google Benchmark v1.9.5 has no percentile flag — median from 3 runs = p50):

```
./build/orderbook_bench --benchmark_min_time=2s --benchmark_repetitions=3
```

| Benchmark | p50 (ns) | p99 (ns) | p99.9 (ns) | mean (ns) |
|---|---|---|---|---|
| Buy into empty book | | | | |
| Sell into empty book | | | | |
| Buy cross single level | | | | |
| Sell cross single level | | | | |
| Buy sweep 3 levels | | | | |
| Sell sweep 3 levels | | | | |
| Buy partial fill | | | | |
| Sell partial fill | | | | |
| Cancel, level kept (buy) | | | | |
| Cancel, level kept (sell) | | | | |
| Cancel, level erased (buy) | | | | |
| Cancel, level erased (sell) | | | | |

## Interpretation

_Replace with real observations once numbers land. Expected shape / things to look for:_
- cancel-empty-level should cost more than cancel-keep-level (map erase + rebalancing vs. queue pop).
- partial fill and multi-level sweep do the most work (match loop + balance math + refunds) → highest ns/op.
- p99/p99.9 tail: single-threaded, tiny working set — expect allocation (order/trade objects, `chrono` timestamp per fill) to dominate the spread.

## Caveats

- Synthetic, uniform scenarios: one op type per run at fixed price/size — no mix, no queue-depth distribution, no realistic message arrival pattern.
- Single-threaded; no contention measured.
- Each iteration re-runs on a freshly re-set book; cold-cache effects included but amortized state distributions are not.
- All timestamps and trade-history writes (per fill, per order) run inside the timed region — numbers include logging-to-memory, which a production engine would offload or drop.
