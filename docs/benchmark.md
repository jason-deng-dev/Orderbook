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
- Build flags verified as `-O3 -DNDEBUG` (Release) via `build/build.ninja` — asserts stripped, optimized code; results are meaningless on a Debug build.
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
| Compiler | clang++ 22.1.5 (from `CMAKE_CXX_COMPILER` in CMake cache) |
| Language standard | C++23 |
| Build | CMake 4.3.3 + Ninja; `cmake --preset default -DCMAKE_BUILD_TYPE=Release` (`-O3 -DNDEBUG`) |
| Benchmark lib | Google Benchmark v1.9.5, `benchmark::benchmark_main` |
| Run date | 2026-09-03T10:28:23+08:00 |

## Results
Measured with custom percentile functions p50, p99, p99_9 defined in `benchmark/main.cpp`
- across 100 repetitions
- raw result output in `benchmark_results.md` for each test run
```c

static double p50(const std::vector<double>& v) {
    std::vector<double> data = v;
    std::sort(data.begin(), data.end());
    if (data.empty()) return 0.0;
    return data[static_cast<size_t>(0.5 * (data.size() - 1))];
}

BENCHMARK(BM_BuyEmptyBook)
    ->Repetitions(100)
    ->ComputeStatistics("p50", p50)
    ->ComputeStatistics("p99", p99)
    ->ComputeStatistics("p99.9", p99_9);
```

```
./build/orderbook_bench --benchmark_min_time=0.5s --benchmark_report_aggregates_only=true
```

| Benchmark | p50 (ns) | p99 (ns) | p99.9 (ns) | mean (ns) |
|---|---|---|---|---|
| Buy into empty book |142|145|145|142|
| Sell into empty book |147|150|150|147|
| Buy cross single level |234|239|239|235|
| Sell cross single level |243|248|248|243|
| Buy sweep 3 levels |357|364|364|357|
| Sell sweep 3 levels |369|376|376|369|
| Buy partial fill |202|207|207|203|
| Sell partial fill |210|217|217|211|
| Cancel, level kept (buy) |125|126|126|125|
| Cancel, level kept (sell) |129|131|131|129|
| Cancel, level erased (buy) |126|128|128|126|
| Cancel, level erased (sell) |133|134|134|133|

## Interpretation

Sell operations consistently have slower operation time than buy operations, on average +3.9% slower.

| op | buy p50 | sell p50 | Δ |
|---|---|---|---|
| empty book insert | 142 | 147 | +3.5% |
| cross 1 level | 234 | 243 | +3.8% |
| sweep 3 levels | 357 | 369 | +3.4% |
| partial fill | 202 | 210 | +4.0% |
| cancel keep level | 125 | 129 | +3.2% |
| cancel erase level | 126 | 133 | +5.6% |
| **avg** | | | **+3.9%** |

```c
// buy path
trader->changeBalance(-quantity * price);   // int member +=

// sell path
trader->changeInventoryAmount(this, -quantity);
//   = inventory[orderbook] += amount;      // operator[]: hash + possible insert
//     if (inventory[orderbook] == 0)       // second operator[]: second hash
//       inventory.erase(orderbook);
```

- comparing the full implementation of `Orderbook::buy` and `Orderbook::sell`, the two code paths differ only in the trader's account bookkeeping:
  - buy: `trader->changeBalance()` — plain `int` member read/write
  - sell: `trader->changeInventoryAmount()` — position stored in `std::unordered_map<Orderbook*, int>`, each `operator[]` does a pointer hash lookup
- `addBuyOrder`/`addSellOrder`, the fill engine (`handleFill`), and the map/queue maintenance are mirror implementations between the sides — the matching engine itself is buy/sell symmetric
- the sell side also pays hash cost on every entry guard: `getInventoryAmount(this)` (`count()` + `at()`) where buy only reads the `balance_` member
- so sell incurs ~4-5 unordered_map hash lookups per call that buy does not, matching the consistent +3.2-5.6% across every op pair
- note the `inventory[orderbook] == 0` erase check is *not* the driver: in these benchmarks inventory never reaches zero during the timed section, so the erase branch never fires — the cost is the double `operator[]` hashing itself
- the cancel pair is the cleanest isolation: `cancelBuy`/`cancelSell` are identical except `changeBalance` vs `changeInventoryAmount`, yet still show +3.2%/+5.6%

The asymmetry is a `Trader` bookkeeping artifact, not an engine property: cash is a flat `int` member while positions live in a hash map keyed by book pointer. A flat representation (array/struct keyed by book id) would make sell-side cost match buy-side.

