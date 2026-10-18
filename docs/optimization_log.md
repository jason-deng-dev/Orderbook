# benchmark setup
```bash
cmake --preset default -DCMAKE_BUILD_TYPE=Release
cmake --build build
taskset -c 2,4 ./build/benchmarks --benchmark_repetitions=25 --benchmark_report_aggregates_only=true
```

# Log 1 (git tag: v1-baseline)
```bash
taskset -c 2,4 ./build/benchmarks --benchmark_repetitions=25 --benchmark_report_aggregates_only=true
2026-10-18T11:46:56+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.14, 0.11, 0.15
-----------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                               Time             CPU   Iterations UserCounters...
-----------------------------------------------------------------------------------------------------------------------------------------
BM_Orderbook_Map_Throughput/iterations:121556_mean                                   80.8 ns         79.0 ns           25 items_per_second=12.6909M/s
BM_Orderbook_Map_Throughput/iterations:121556_median                                 78.0 ns         78.0 ns           25 items_per_second=12.8202M/s
BM_Orderbook_Map_Throughput/iterations:121556_stddev                                 10.1 ns         5.02 ns           25 items_per_second=626.942k/s
BM_Orderbook_Map_Throughput/iterations:121556_cv                                    12.51 %          6.36 %            25 items_per_second=4.94%
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_mean                   73.0 ns         73.0 ns           25 items_per_second=13.7039M/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_median                 72.9 ns         72.9 ns           25 items_per_second=13.7262M/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_stddev                0.713 ns        0.713 ns           25 items_per_second=131.308k/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_cv                     0.98 %          0.98 %            25 items_per_second=0.96%
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_mean                   75.2 ns         75.2 ns           25 items_per_second=13.3039M/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_median                 75.1 ns         75.1 ns           25 items_per_second=13.3227M/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_stddev                0.279 ns        0.280 ns           25 items_per_second=49.4325k/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_cv                     0.37 %          0.37 %            25 items_per_second=0.37%
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_mean         73.7 ns         73.7 ns           25 items_per_second=13.566M/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_median       73.7 ns         73.7 ns           25 items_per_second=13.5626M/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_stddev      0.406 ns        0.401 ns           25 items_per_second=73.6885k/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_cv           0.55 %          0.54 %            25 items_per_second=0.54%
BM_FeedHandler_Throughput/iterations:282229685_mean                                  6.03 ns         6.03 ns           25 items_per_second=165.908M/s
BM_FeedHandler_Throughput/iterations:282229685_median                                6.03 ns         6.03 ns           25 items_per_second=165.876M/s
BM_FeedHandler_Throughput/iterations:282229685_stddev                               0.040 ns        0.040 ns           25 items_per_second=1.09215M/s
BM_FeedHandler_Throughput/iterations:282229685_cv                                    0.66 %          0.66 %            25 items_per_second=0.66%
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_mean                     2.45 ns         2.44 ns           25 items_per_second=410.291M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_median                   2.43 ns         2.43 ns           25 items_per_second=411.694M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_stddev                  0.165 ns        0.156 ns           25 items_per_second=26.0302M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_cv                       6.74 %          6.39 %            25 items_per_second=6.34%
BM_Fifo_Cached_Throughput/iterations:100000000/real_time/threads:2_mean              3.04 ns         3.00 ns           25 items_per_second=342.537M/s
BM_Fifo_Cached_Throughput/iterations:100000000/real_time/threads:2_median            2.92 ns         2.76 ns           25 items_per_second=342.112M/s
BM_Fifo_Cached_Throughput/iterations:100000000/real_time/threads:2_stddev           0.651 ns        0.628 ns           25 items_per_second=65.0368M/s
BM_Fifo_Cached_Throughput/iterations:100000000/real_time/threads:2_cv               21.45 %         20.95 %            25 items_per_second=18.99%
```

The cached FIFO row in Log 1 was measured with bounded cursors that reset at capacity. That implementation was later found to be incorrect; see Log 2. The orderbook and feed handler rows are unaffected.

## Summary table

|Benchmark|Throughput|mean|median|stddev|Coefficient of Variation|
|---------|----------|----|------|------|------------------------|
| BM_Fifo_Throughput | 410.29 M/s | 2.45 ns | 2.43 ns | 0.165 ns | 6.74% |
| BM_Fifo_Cached_Throughput | 342.54 M/s | 3.04 ns | 2.92 ns | 0.651 ns | 21.45% |
| BM_FeedHandler_Throughput | 165.91 M/s | 6.03 ns | 6.03 ns | 0.040 ns | 0.66% |
| BM_Orderbook_Map_Throughput | 12.69 M/s | 80.8 ns | 78.0 ns | 10.1 ns | 12.51% |
| BM_Orderbook_Vector_LinearSearch_Throughput | 13.70 M/s | 73.0 ns | 72.9 ns | 0.713 ns | 0.98% |
| BM_Orderbook_Vector_BinarySearch_Throughput | 13.30 M/s | 75.2 ns | 75.1 ns | 0.279 ns | 0.37% |
| BM_Orderbook_Vector_BranchlessBinarySearch_Throughput | 13.57 M/s | 73.7 ns | 73.7 ns | 0.406 ns | 0.55% |

Times and throughput are from the full-suite snapshot above (25 repetitions). Throughput is the mean `items_per_second`. Use medians when CV is high; `std::map` (12.51%) and the cached FIFO (21.45%) vary enough in this run that their means are not reliable. Perf counters are excluded because process-wide counters include static fixture loading, which dominates the orderbook benchmarks' totals.

# Vector-Based Implementation Outperform `std::map`

All contiguous vector implementations achieved lower median latency (72.9 - 75.1 ns) compared to the `std::map` baseline (78.0 ns median).
- Linear Search achieved 6.54% lower median latency
- Binary Search achieved 3.72% lower median latency
- Branchless Binary Search achieved 5.51% lower median latency

Our Linear Search vector implementation's throughput (13.70 M/s) had a 8% increase in performance compared to `std::map` baseline (12.69 M/s).

Since `std::map` is a node-based red-black tree and thus requires heap-allocated nodes, we encounter pointer chasing.

Where each node lookup likely incurs an L1/L2 cache miss, forcing CPU to fetch data from slower L3 cache or main memory. Converslely, our `std::vector` implementations stores PriceLevel structs in contiguous memory.

Thus even with the additional overhead of maintaining sorted order, the contiguous layout allows the CPU's hardware prefetcher to load subsequent price levels into the L1 cache ahead of time, signficantly reducing memory latency.

# Linear Search > Branchless Binary Search > Binary Search
For our filtered, single-stock dataset, Linear Search (72.9 ns median) proved fastest, followed by Branchless Binary (73.7 ns), with Standard Binary Search being the slowest (75.1 ns).

Since we filter the ITCH feed to a single liquid ticker, the number of active price levels (N) is small (typically < 100).

Why Linear wins:
- For small N, the CPU's branch predictor easily predicts the linear loop. Furthermore, since linear search accesses memory strictly sequentially, perfectly, we are able to perfectly saturate our L1 cache bandwidth and maximize hardware prefetching.

Why Branchless beats Standard Binary:
- Standard Binary Search relies on if/else branches to narrow the search space. On a small, unpredictable dataset, this causes branch mispredictions, which flushes CPU pipeline (~15-20 cycle penalty). 
- Branchless binary search replaces these branches with conditional moves, `first += comp(first[half].first, price) * (length - half);` eliminating the misprediction penalty and making it faster than standard binary. 

However both binary approaches still suffer from random memory acces jumps, which breaks hardware prefetching, keeping them strictly slower than the seqeuntial scan of Linear Search.

## Cached FIFO with cursor reset at capacity

Hypothesis: without caching, push loads `popCursor_` with acquire and pop loads `pushCursor_` with acquire on every operation. Those lines are written by the other thread, so each operation bounces them across cores. Caching the peer cursor in a thread-exclusive member should let the fast path skip that load, paying it only when the cached view says full or empty.

Implementation: bound cursors to `0..capacity` and reset to 0 when the cursor reaches `capacity`. The cached peer cursor is refreshed only on the full/empty slow path.

Result: the cached version was 24.1% slower (3.04 vs 2.45 ns mean) with 3.18x higher CV. That looked like the caching idea failing.

## Benchmark results
Under saturated throughput benchmarking, the reset implementation performed worse than the baseline:
- 3.04 ns mean latency (2.92 ns median) for the cached Fifo versus 2.45 ns mean (2.43 ns median) for the uncached FIFO
- Throughput drops from 410.29 M/s for uncached implementation to 342.54 M/s for cached implementation dropping 16.5%
- CV increased from 6.74% for uncached implementation to 21.45% for cached implementation

## Perf stat results
```bash
taskset -c 2,4 perf stat -e 'cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock'   ./build/benchmarks   --benchmark_filter=BM_Fifo_Throughput   --benchmark_repetitions=10   --benchmark_report_aggregates_only=true
2026-10-18T12:41:57+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.54, 0.43, 0.25
-----------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                   Time             CPU   Iterations UserCounters...
-----------------------------------------------------------------------------------------------------------------------------
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_mean         2.38 ns         2.38 ns           10 items_per_second=419.311M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_median       2.38 ns         2.38 ns           10 items_per_second=419.615M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_stddev      0.016 ns        0.016 ns           10 items_per_second=2.72436M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_cv           0.65 %          0.68 %            10 items_per_second=0.65%

 Performance counter stats for './build/benchmarks --benchmark_filter=BM_Fifo_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true':

       34776533844      cycles
       79872038284      instructions
         585936595      cache-misses
        3837421105      cache-references
          14149974      branch-misses
       16613258706      branches
           6721.24 msec task-clock

       4.340782056 seconds time elapsed

       6.676198000 seconds user
       0.043948000 seconds sys
```

```bash
taskset -c 2,4 perf stat -e 'cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock'   ./build/benchmarks   --benchmark_filter=BM_Fifo_Cached_Throughput   --benchmark_repetitions=10   --benchmark_report_aggregates_only=true
2026-10-18T12:41:25+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.65, 0.43, 0.24
------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                          Time             CPU   Iterations UserCounters...
------------------------------------------------------------------------------------------------------------------------------------
BM_Fifo_Cached_Throughput/iterations:100000000/real_time/threads:2_mean         3.01 ns         2.98 ns           10 items_per_second=337.145M/s
BM_Fifo_Cached_Throughput/iterations:100000000/real_time/threads:2_median       2.83 ns         2.81 ns           10 items_per_second=353.461M/s
BM_Fifo_Cached_Throughput/iterations:100000000/real_time/threads:2_stddev      0.422 ns        0.363 ns           10 items_per_second=37.9595M/s
BM_Fifo_Cached_Throughput/iterations:100000000/real_time/threads:2_cv          14.04 %         12.17 %            10 items_per_second=11.26%

 Performance counter stats for './build/benchmarks --benchmark_filter=BM_Fifo_Cached_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true':

       40902097696      cycles
       85235639526      instructions
         893783901      cache-misses
        4745095287      cache-references
          60377789      branch-misses
       19247519923      branches
           7975.43 msec task-clock

       5.025478094 seconds time elapsed

       7.929523000 seconds user
       0.043964000 seconds sys
```

## Perf stat comparison
| Metric | Uncached | Cached | Change |
|--------|----------|--------|--------|
| mean latency | 2.45 ns | 3.04 ns | +24.1% |
| median latency | 2.43 ns | 2.92 ns | +20.2% |
| throughput | 410.29 M/s | 342.54 M/s | -16.5% |
| coefficient of variation | 6.74% | 21.45% | 3.18x |
| cycles | 34.78 G | 40.90 G | +17.6% |
| instructions | 79.87 G | 85.24 G | +6.7% |
| IPC | 2.30 | 2.08 | -9.3% |
| branches | 16.61 G | 19.25 G | +15.9% |
| branch misses | 14.15 M | 60.38 M | +326.7% |
| branch miss rate | 0.0852% | 0.3137% | +0.2285 pp |
| cache references | 3.84 G | 4.75 G | +23.7% |
| cache misses | 585.94 M | 893.78 M | +52.5% |
| cache miss rate | 15.27% | 18.84% | +3.57 pp |
| task-clock | 6.72 s | 7.98 s | +18.7% |
| instructions per item | 79.9 | 85.2 | +6.7% |
| cycles per item | 34.8 | 40.9 | +17.6% |
| branch misses per item | 0.0141 | 0.0604 | +326.7% |
| cache misses per item | 0.586 | 0.894 | +52.5% |

## Hypothesis recorded at the time
This explanation was written before the correctness check. It treats the reset implementation as a valid cached queue, which Log 2 shows was false. Kept here as the original reasoning.

**Branch misses increased by 326% (14.1M → 60.3M).**
In a saturated spin as in our benchmark, the consumer drains the queue faster than the producer fills it, keeping occupancy near 0%.
The cached implementation if check:
```c++
if (empty(cachedPushCursor_, popCursor)) { // Branch A
  cachedPushCursor_ = pushCursor_.load(std::memory_order::acquire);
  if (empty(cachedPushCursor_, popCursor)) return false; // Branch B
}
```
Because the queue is almost always empty, the CPU branch predictor learns to predict Branch B as "taken". The exact moment the producer pushes an Item, Branch B evaluates to "not taken", guaranteeing a branch misprediction and a ~15-cycle pipeline flush. The uncached version lacks this nested check
```c++
if (full(pushCursor, popCursor)) {
  return false;
}
```
resulting in much lower misprediciton rate.

**Cache misses increased by 52% (585M → 893M).**
I believe this is caused by the logical flow of the "slow path". When the queue is empty which is most of the time in this benchmark, the cached version simply does more memory operation than the baseline
- Baseline: loads the atomic `pushCursor_` (1 memory read)
- Cached version: Loads class member `cachedPushCursor_ (1 memory read), sees it's stale, loads the atomic pushCursor_ (1 memory read) and writes the new value back to cachedPushCursor_ (1 memory write)

Because the queue is always empty, the cache is always stale. As a result the cached implementation pays the penalty of extra memory reads and writes on every single iteration, without ever reaping the reward of the fast path. More memory operations directly result in more cache misses

## Hypothesis
The cached implementation's true performance benefits only show if the queue is in a state where there are values in it, so that our cached implementation can benefit from skipping the expensive cross-core atomic load on the fast path. 

I believe if I implement a benchmark scenrio where our SPSC Fifo is already filled, and we conduct our throughput testing for the duration where our SPSC Fifo is non-empty we will be able to better see the performance benefits of the cached SPSC FIFO.




# Log 2: Monotonic cached FIFO (git tag: v2-fifo-monotonic)

## Correctness check first

Capacity 4, fill, drain, push 4 and 5, pop twice:

```text
pop=5 pop=5 (expected 4,5)
```

Destructor after drain plus one wrapped push hangs.

Root cause: cursors used `capacity + 1` values, but the ring has `capacity` slots. `cursor & (capacity - 1)` maps both `0` and `capacity` to slot 0. After writing the item at cursor `capacity`, the producer stored cursor 0, which points at the same live slot. The next push overwrote it. The destructor assumed monotonic cursors, so `pushCursor - popCursor` underflowed when `popCursor > pushCursor`.

Log 1 measured a broken queue. Its cached-vs-uncached comparison cannot judge the optimization.

## Fix

Remove the reset. Cursors stay monotonic. `cursor & (capacity - 1)` handles slot wrap. Unsigned subtraction handles `2^64` wrap. No extra branch.

## Full suite (25 repetitions)

```bash
taskset -c 2,4 ./build/benchmarks --benchmark_repetitions=25 --benchmark_report_aggregates_only=true
2026-10-18T16:20:55+08:00
```

|Benchmark|Throughput|mean|median|stddev|Coefficient of Variation|
|---------|----------|----|------|------|------------------------|
| BM_Fifo_Throughput | 373.31 M/s | 2.71 ns | 2.52 ns | 0.324 ns | 11.95% |
| BM_Fifo_Cached_Throughput | 1.217 G/s | 0.911 ns | 0.723 ns | 0.342 ns | 37.53% |
| BM_FeedHandler_Throughput | 161.78 M/s | 6.20 ns | 6.13 ns | 0.383 ns | 6.17% |
| BM_Orderbook_Map_Throughput | 12.54 M/s | 79.8 ns | 79.7 ns | 0.512 ns | 0.64% |
| BM_Orderbook_Vector_LinearSearch_Throughput | 13.47 M/s | 76.2 ns | 74.2 ns | 10.0 ns | 13.17% |
| BM_Orderbook_Vector_BinarySearch_Throughput | 13.09 M/s | 76.4 ns | 76.3 ns | 0.536 ns | 0.70% |
| BM_Orderbook_Vector_BranchlessBinarySearch_Throughput | 13.30 M/s | 75.2 ns | 75.2 ns | 0.338 ns | 0.45% |
| BM_Fifo_Throughput_Slacked | 458.48 M/s | 2.30 ns | 2.11 ns | 0.873 ns | 38.02% |
| BM_Fifo_Cached_Throughput_Slacked | 921.67 M/s | 1.24 ns | 1.02 ns | 0.885 ns | 71.53% |

Throughput is the mean `items_per_second`. CV is high for the FIFO rows and for linear search's real-time column, so use medians when comparing those. The slacked benchmark pre-fills half of the 4,194,304-slot ring before timing.

## Cached vs uncached (perf stat)

```bash
taskset -c 2,4 perf stat -e cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock \
  ./build/benchmarks --benchmark_filter='^BM_Fifo_Throughput/' \
  --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
```

```bash
taskset -c 2,4 perf stat -e cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock \
  ./build/benchmarks --benchmark_filter='^BM_Fifo_Cached_Throughput/' \
  --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
```

| Metric | Uncached | Cached | Change |
|--------|----------|--------|--------|
| mean latency | 3.55 ns | 1.45 ns | -59.2% |
| throughput | 297.75 M/s | 758.32 M/s | +154.7% |
| coefficient of variation | 26.49% | 24.94% | -1.55 pp |
| cycles | 47.18 G | 25.25 G | -46.5% |
| instructions | 82.49 G | 79.81 G | -3.3% |
| IPC | 1.75 | 3.16 | +80.8% |
| branches | 17.26 G | 17.76 G | +2.9% |
| branch misses | 39.45 M | 29.99 M | -24.0% |
| branch miss rate | 0.2286% | 0.1689% | -0.0597 pp |
| cache references | 4.50 G | 3.84 G | -14.6% |
| cache misses | 673.85 M | 477.26 M | -29.2% |
| cache miss rate | 14.97% | 12.42% | -2.55 pp |
| task-clock | 9.17 s | 4.92 s | -46.3% |
| instructions per item | 82.5 | 79.8 | -3.3% |
| cycles per item | 47.2 | 25.2 | -46.5% |

Counters are raw perf stat totals for the whole process, including static fixture loading. Both runs pay the same setup cost; the deltas are still meaningful, but per-item values are upper bounds.

## Analysis

Correctness: `Spsc_Fifo.*` tests pass 20/20. The wrap sequence now pops `4, 5`. The destructor-after-wrap test exits cleanly.

The cached queue processes the same instruction volume with roughly half the cycles:

- Instructions: 82.49 G vs 79.81 G, about equal.
- Cycles: 47.18 G vs 25.25 G, -46.5%.
- IPC: 1.75 vs 3.16, +80.8%.
- Cache misses: 673.85 M vs 477.26 M, -29.2%.

That is the cached-cursor mechanism working. The uncached push loads `popCursor_` and the uncached pop loads `pushCursor_` on every operation, so the peer's cache line bounces between cores. The cached version keeps the owner cursor line local and refreshes the peer cursor only at the full/empty boundary. With 65,536 slots, both threads run long stretches without touching each other's line.

Mean throughput rises from 373.31 M/s to 1.217 G/s, 3.26x. Median latency drops from 2.52 ns to 0.723 ns. The pre-filled slacked benchmark shows the same direction: 458.48 M/s to 921.67 M/s.

Log 1's branch-misprediction explanation was based on the broken reset implementation, so it is not the explanation for this result. The run-to-run variance is high (cached CV 37.53%), so repeated runs and medians are required before quoting a speedup.

# Planned optimizations
Orderbook optimization
- intrusive slab orderbook
- event log spsc
