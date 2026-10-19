# benchmark setup
```bash
cmake --preset default -DCMAKE_BUILD_TYPE=Release
cmake --build build
taskset -c 2,4 ./build/benchmarks --benchmark_repetitions=25 --benchmark_report_aggregates_only=true
```

# Log 1 (git tag: v1-baseline)

## Full Suite (25 iterations)
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

Throughput is the mean `items_per_second`


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

## Analysis

**Vector-Based Implementation Outperform `std::map`**

All contiguous vector implementations achieved lower median latency (72.9 - 75.1 ns) compared to the `std::map` baseline (78.0 ns median).
- Linear Search achieved 6.54% lower median latency
- Binary Search achieved 3.72% lower median latency
- Branchless Binary Search achieved 5.51% lower median latency

Our Linear Search vector implementation's throughput (13.70 M/s) had a 8% increase in performance compared to `std::map` baseline (12.69 M/s).

Since `std::map` is a node-based red-black tree and thus requires heap-allocated nodes, we encounter pointer chasing.

Where each node lookup likely incurs an L1/L2 cache miss, forcing CPU to fetch data from slower L3 cache or main memory. Converslely, our `std::vector` implementations stores PriceLevel structs in contiguous memory.

Thus even with the additional overhead of maintaining sorted order, the contiguous layout allows the CPU's hardware prefetcher to load subsequent price levels into the L1 cache ahead of time, signficantly reducing memory latency.

**Linear Search > Branchless Binary Search > Binary Search**
For our filtered, single-stock dataset, Linear Search (72.9 ns median) proved fastest, followed by Branchless Binary (73.7 ns), with Standard Binary Search being the slowest (75.1 ns).

Since we filter the ITCH feed to a single liquid ticker, the number of active price levels (N) is small (typically < 100).

Why Linear wins:
- For small N, the CPU's branch predictor easily predicts the linear loop. Furthermore, since linear search accesses memory strictly sequentially, perfectly, we are able to perfectly saturate our L1 cache bandwidth and maximize hardware prefetching.

Why Branchless beats Standard Binary:
- Standard Binary Search relies on if/else branches to narrow the search space. On a small, unpredictable dataset, this causes branch mispredictions, which flushes CPU pipeline (~15-20 cycle penalty). 
- Branchless binary search replaces these branches with conditional moves, `first += comp(first[half].first, price) * (length - half);` eliminating the misprediction penalty and making it faster than standard binary. 

However both binary approaches still suffer from random memory acces jumps, which breaks hardware prefetching, keeping them strictly slower than the seqeuntial scan of Linear Search.

**Cached FIFO performed worse than baseline FIFO**
Under saturated throughput benchmarking, the cached implementation performed worse than the baseline:
- 3.04 ns mean latency (2.92 ns median) for the cached Fifo versus 2.45 ns mean (2.43 ns median) for the uncached FIFO
- Throughput drops from 410.29 M/s for uncached implementation to 342.54 M/s for cached implementation dropping 16.5%
- CV increased from 6.74% for uncached implementation to 21.45% for cached implementation

This is in opposition to our hypothesis: without caching, push loads `popCursor_` with acquire and pop loads `pushCursor_` with acquire on every operation. Those lines are written by the other thread, so each operation bounces them across cores. Caching the peer cursor in a thread-exclusive member should let the fast path skip that load, paying it only when the cached view says full or empty.

Result: the cached version was 24.1% slower (3.04 vs 2.45 ns mean) with 3.18x higher CV. That looked like the caching idea failing.

**Cached Fifo : Branch misses increased by 326% (14.1M → 60.3M).**
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

**Cached Fifo : Cache misses increased by 52% (585M → 893M).**
I believe this is caused by the logical flow of the "slow path". When the queue is empty which is most of the time in this benchmark, the cached version simply does more memory operation than the baseline
- Baseline: loads the atomic `pushCursor_` (1 memory read)
- Cached version: Loads class member `cachedPushCursor_ (1 memory read), sees it's stale, loads the atomic pushCursor_ (1 memory read) and writes the new value back to cachedPushCursor_ (1 memory write)

Because the queue is always empty, the cache is always stale. As a result the cached implementation pays the penalty of extra memory reads and writes on every single iteration, without ever reaping the reward of the fast path. More memory operations directly result in more cache misses

## Hypothesis
**Creating a slacked benchmark environment**

The cached implementation's advantage only appears when the queue sits away from both boundaries. In the current saturated benchmark, the consumer drains faster than the producer fills, so occupancy stays near zero. That forces the consumer's cached push cursor to be stale on every pop: it takes the slow path, reloads the producer's cursor with acquire, updates the cached copy, and retries. The producer almost never takes its slow path because the queue is never full. The benchmark therefore measures the cached consumer's slow-path cost without exercising the fast path that caching is meant to enable.

I believe the result changes if the queue stays partially full for the whole timed loop. The idea is to allocate a large ring, pre-fill it to half capacity, then run an equal number of timed pushes and pops. With pushes and pops balanced around the half-full mark, the queue stays away from both boundaries. The producer's cached consumer cursor remains valid because the queue never becomes full, and the consumer's cached producer cursor remains valid because the queue never becomes empty. After the initial cache fill, both threads should stay on the fast path and skip the cross-core acquire loads that the uncached implementation performs on every operation.

I expect the slacked cached FIFO to beat the slacked uncached FIFO in latency, cycles per item, and cache misses, unlike the saturated benchmark. Running the uncached FIFO through the same pre-filled setup gives the control. That comparison would show whether cursor caching pays off once the queue is not pinned against the empty boundary.

**Changing pushCursor/popCursor to be monotonic**
The cursors currently stay in `0..capacity` and reset to 0 when they reach `capacity`. I believe the wrap-around handling is costing performance on the hot path. Every push and pop pays a comparison and a conditional store, and `full()`, `empty()`, and `size()` each handle two possible cursor orderings. After a reset, `pushCursor < popCursor` is a normal state, so every occupancy check carries the extra comparisons.

Proposed: keep cursors monotonically increasing and derive the slot index with `cursor & (capacity - 1)`. Unsigned subtraction keeps `full()`, `empty()`, and `size()` correct across the eventual 2^64 wrap, so no reset is needed at capacity. The fast path becomes one subtraction and one comparison, and publishing a cursor becomes a single store. The uncached FIFO has the same reset pattern, so the change applies to both implementations.

The slot index stays `cursor & (capacity - 1)` in both versions, so only cursor arithmetic changes.

I expect this to reduce instructions and conditional operations per push and pop. The benchmark loop spends its time in these checks, so the saving should appear as lower cycles per item and higher throughput.


# Log 2: Monotonic cached FIFO (git tag: v2-fifo-monotonic)

## Changes
**Add explicit CPU pinning inside of benchmark**
In addition to using taskset -c 2,4 we must pin consumer/producer threads inside of SPSC Fifo benchmarks via `pthread_setaffinity_np()`, to ensure that our consumer/producer threads pin to different CPUs

**Removed the cursor reset**
- Cursors stay monotonic. `cursor & (capacity - 1)` handles slot wrap.

resulting in simpler popCursor update
```c
popCursor_.store(popCursor + 1, std::memory_order_release);
```
instead of the previous if/else loop to handle wrap around
```c
if (popCursor == capacity_) {
  popCursor_.store(0, std::memory_order_release);
} else {
  popCursor_.store(popCursor + 1, std::memory_order_release);
}
```

**Added Slacked SPSC Fifo benchmark**
- We first choose a large slackFifoCapacity = 4,194,304
- and then fill the SPSC Fifo up to slackCapacity/2
- and we run out benchmark based on the initial slack size

This is to ensure that every iteration of our SPSC Fifo will be operating in the scenerio where it is not empty.
## Full suite (50 repetitions)
```bash
 taskset -c 2,4 ./build/benchmarks --benchmark_filter=BM_Fifo*  --benchmark_repetitions=50 --benchmark_report_aggregates_only=true
2026-10-18T16:50:38+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.35, 0.33, 0.27
------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                Time             CPU   Iterations UserCounters...
------------------------------------------------------------------------------------------------------------------------------------------
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_mean                      3.32 ns         3.32 ns           50 items_per_second=305.998M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_median                    3.41 ns         3.41 ns           50 items_per_second=293.307M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_stddev                   0.412 ns        0.412 ns           50 items_per_second=38.7737M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_cv                       12.41 %         12.41 %            50 items_per_second=12.67%
BM_Fifo_Cached_Throughput/iterations:100000000/real_time/threads:2_mean              0.851 ns        0.851 ns           50 items_per_second=1.20748G/s
BM_Fifo_Cached_Throughput/iterations:100000000/real_time/threads:2_median            0.827 ns        0.827 ns           50 items_per_second=1.20953G/s
BM_Fifo_Cached_Throughput/iterations:100000000/real_time/threads:2_stddev            0.147 ns        0.147 ns           50 items_per_second=197.82M/s
BM_Fifo_Cached_Throughput/iterations:100000000/real_time/threads:2_cv                17.32 %         17.33 %            50 items_per_second=16.38%
BM_Slacked_Fifo_Throughput/iterations:2097152/real_time/threads:2_mean                2.65 ns         2.64 ns           50 items_per_second=397.815M/s
BM_Slacked_Fifo_Throughput/iterations:2097152/real_time/threads:2_median              2.47 ns         2.47 ns           50 items_per_second=404.416M/s
BM_Slacked_Fifo_Throughput/iterations:2097152/real_time/threads:2_stddev              1.21 ns         1.21 ns           50 items_per_second=44.9496M/s
BM_Slacked_Fifo_Throughput/iterations:2097152/real_time/threads:2_cv                 45.59 %         45.65 %            50 items_per_second=11.30%
BM_Slacked_Fifo_Cached_Throughput/iterations:2097152/real_time/threads:2_mean         1.30 ns         1.30 ns           50 items_per_second=907.357M/s
BM_Slacked_Fifo_Cached_Throughput/iterations:2097152/real_time/threads:2_median       1.06 ns         1.06 ns           50 items_per_second=942.403M/s
BM_Slacked_Fifo_Cached_Throughput/iterations:2097152/real_time/threads:2_stddev       1.44 ns         1.44 ns           50 items_per_second=147.669M/s
BM_Slacked_Fifo_Cached_Throughput/iterations:2097152/real_time/threads:2_cv         110.61 %        110.71 %            50 items_per_second=16.27%
```
## Benchmark Summary Table
|Benchmark|Throughput|mean|median|stddev|Coefficient of Variation|
|---------|----------|----|------|------|------------------------|
| BM_Fifo_Throughput | 306.00 M/s | 3.32 ns | 3.41 ns | 0.412 ns | 12.41% |
| BM_Fifo_Cached_Throughput | 1.207 G/s | 0.851 ns | 0.827 ns | 0.147 ns | 17.32% |
| BM_FeedHandler_Throughput | 161.78 M/s | 6.20 ns | 6.13 ns | 0.383 ns | 6.17% |
| BM_Orderbook_Map_Throughput | 12.54 M/s | 79.8 ns | 79.7 ns | 0.512 ns | 0.64% |
| BM_Orderbook_Vector_LinearSearch_Throughput | 13.47 M/s | 76.2 ns | 74.2 ns | 10.0 ns | 13.17% |
| BM_Orderbook_Vector_BinarySearch_Throughput | 13.09 M/s | 76.4 ns | 76.3 ns | 0.536 ns | 0.70% |
| BM_Orderbook_Vector_BranchlessBinarySearch_Throughput | 13.30 M/s | 75.2 ns | 75.2 ns | 0.338 ns | 0.45% |
| BM_Slacked_Fifo_Throughput | 397.82 M/s | 2.65 ns | 2.47 ns | 1.21 ns | 45.59% |
| BM_Slacked_Fifo_Cached_Throughput | 907.36 M/s | 1.30 ns | 1.06 ns | 1.44 ns | 110.61% |

Throughput is the mean `items_per_second`
## Perf stat results
```bash
taskset -c 2,4 perf stat -e cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock   ./build/benchmarks --benchmark_filter='BM_Fifo_Throughput'   --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
2026-10-18T19:58:05+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.25, 0.22, 0.11
-----------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                   Time             CPU   Iterations UserCounters...
-----------------------------------------------------------------------------------------------------------------------------
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_mean         2.95 ns         2.94 ns           10 items_per_second=339.986M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_median       2.92 ns         2.92 ns           10 items_per_second=342.367M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_stddev      0.113 ns        0.113 ns           10 items_per_second=12.759M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_cv           3.83 %          3.85 %            10 items_per_second=3.75%

 Performance counter stats for './build/benchmarks --benchmark_filter=BM_Fifo_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true':

       42140874611      cycles
       72034858353      instructions
         556178876      cache-misses
        3924368584      cache-references
          24057583      branch-misses
       14686015858      branches
           8641.99 msec task-clock

       5.698884686 seconds time elapsed

       8.446744000 seconds user
       0.191789000 seconds sys

```

```bash
taskset -c 2,4 perf stat -e cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock \
  ./build/benchmarks --benchmark_filter='BM_Fifo_Cached_Throughput' \
  --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
2026-10-18T19:58:23+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.38, 0.25, 0.13
------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                          Time             CPU   Iterations UserCounters...
------------------------------------------------------------------------------------------------------------------------------------
BM_Fifo_Cached_Throughput/iterations:100000000/real_time/threads:2_mean        0.928 ns        0.928 ns           10 items_per_second=1.10009G/s
BM_Fifo_Cached_Throughput/iterations:100000000/real_time/threads:2_median      0.962 ns        0.961 ns           10 items_per_second=1.03982G/s
BM_Fifo_Cached_Throughput/iterations:100000000/real_time/threads:2_stddev      0.124 ns        0.124 ns           10 items_per_second=188.885M/s
BM_Fifo_Cached_Throughput/iterations:100000000/real_time/threads:2_cv          13.41 %         13.41 %            10 items_per_second=17.17%

 Performance counter stats for './build/benchmarks --benchmark_filter=BM_Fifo_Cached_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true':

       20299199623      cycles
       68309079542      instructions
         265188890      cache-misses
        3503424385      cache-references
          20455428      branch-misses
       14806036151      branches
           4477.72 msec task-clock

       3.555114621 seconds time elapsed

       4.302112000 seconds user
       0.172244000 seconds sys
```

```bash
taskset -c 2,4 perf stat -e cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock \
  ./build/benchmarks --benchmark_filter='BM_Slacked_Fifo_Throughput' \
  --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
2026-10-18T19:58:38+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.38, 0.26, 0.13
-----------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                         Time             CPU   Iterations UserCounters...
-----------------------------------------------------------------------------------------------------------------------------------
BM_Slacked_Fifo_Throughput/iterations:2097152/real_time/threads:2_mean         3.28 ns         3.28 ns           10 items_per_second=392.367M/s
BM_Slacked_Fifo_Throughput/iterations:2097152/real_time/threads:2_median       2.35 ns         2.35 ns           10 items_per_second=424.95M/s
BM_Slacked_Fifo_Throughput/iterations:2097152/real_time/threads:2_stddev       2.96 ns         2.95 ns           10 items_per_second=107.907M/s
BM_Slacked_Fifo_Throughput/iterations:2097152/real_time/threads:2_cv          90.14 %         90.16 %            10 items_per_second=27.50%

 Performance counter stats for './build/benchmarks --benchmark_filter=BM_Slacked_Fifo_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true':

       11268579470      cycles
       27641822322      instructions
          11771283      cache-misses
        1184272389      cache-references
          15694841      branch-misses
        6528247745      branches
           2656.80 msec task-clock

       2.611647605 seconds time elapsed

       2.461601000 seconds user
       0.191502000 seconds sys
```

```bash
taskset -c 2,4 perf stat -e cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock \
  ./build/benchmarks --benchmark_filter='BM_Slacked_Fifo_Cached_Throughput'\
  --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
2026-10-18T19:58:54+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.37, 0.26, 0.13
------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                Time             CPU   Iterations UserCounters...
------------------------------------------------------------------------------------------------------------------------------------------
BM_Slacked_Fifo_Cached_Throughput/iterations:2097152/real_time/threads:2_mean         2.14 ns         2.14 ns           10 items_per_second=814.53M/s
BM_Slacked_Fifo_Cached_Throughput/iterations:2097152/real_time/threads:2_median       1.12 ns         1.12 ns           10 items_per_second=895.756M/s
BM_Slacked_Fifo_Cached_Throughput/iterations:2097152/real_time/threads:2_stddev       3.20 ns         3.20 ns           10 items_per_second=273.38M/s
BM_Slacked_Fifo_Cached_Throughput/iterations:2097152/real_time/threads:2_cv         149.19 %        149.28 %            10 items_per_second=33.56%

 Performance counter stats for './build/benchmarks --benchmark_filter=BM_Slacked_Fifo_Cached_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true':

       11081778810      cycles
       27556935444      instructions
           7970588      cache-misses
        1185916935      cache-references
          16023443      branch-misses
        6528036951      branches
           2663.31 msec task-clock

       2.642208228 seconds time elapsed

       2.424343000 seconds user
       0.236423000 seconds sys
```
## Perf Stat comparison

Saturated pair:

| Metric | Uncached | Cached | Change |
|--------|----------|--------|--------|
| mean latency | 2.95 ns | 0.928 ns | -68.5% |
| median latency | 2.92 ns | 0.962 ns | -67.1% |
| throughput | 339.99 M/s | 1.100 G/s | +223.6% |
| coefficient of variation | 3.83% | 13.41% | +9.58 pp |
| cycles | 42.14 G | 20.30 G | -51.8% |
| instructions | 72.03 G | 68.31 G | -5.2% |
| IPC | 1.71 | 3.37 | +96.9% |
| branches | 14.69 G | 14.81 G | +0.8% |
| branch misses | 24.06 M | 20.46 M | -15.0% |
| branch miss rate | 0.1638% | 0.1382% | -0.0257 pp |
| cache references | 3.92 G | 3.50 G | -10.7% |
| cache misses | 556.18 M | 265.19 M | -52.3% |
| cache miss rate | 14.17% | 7.57% | -6.60 pp |
| task-clock | 8.64 s | 4.48 s | -48.2% |

Slacked pair, Log 2 only:

| Metric | Slacked uncached | Slacked cached | Change |
|--------|------------------|----------------|--------|
| mean latency | 3.28 ns | 2.14 ns | -34.8% |
| median latency | 2.35 ns | 1.12 ns | -52.3% |
| throughput | 392.37 M/s | 814.53 M/s | +107.6% |
| coefficient of variation | 90.14% | 149.19% | +59.05 pp |
| cycles | 11.27 G | 11.08 G | -1.7% |
| instructions | 27.64 G | 27.56 G | -0.3% |
| IPC | 2.45 | 2.49 | +1.4% |
| branches | 6.53 G | 6.53 G | ~0.0% |
| branch misses | 15.69 M | 16.02 M | +2.1% |
| branch miss rate | 0.2404% | 0.2455% | +0.0050 pp |
| cache references | 1.18 G | 1.19 G | +0.1% |
| cache misses | 11.77 M | 7.97 M | -32.3% |
| cache miss rate | 0.994% | 0.672% | -0.322 pp |
| task-clock | 2.66 s | 2.66 s | +0.2% |

## Analysis
In a saturated benchmark run, the cached queue processes teh same instruction volume with roughly half the cycles:
- Instructions: 72.03 G vs 68.31 G, -5.2%.
- Cycles: 42.14 G vs 20.30 G, -51.8%.
- IPC: 1.71 vs 3.37, +96.9%.
- Cache misses: 556.18 M vs 265.19 M, -52.3%.

That is the cached-cursor mechanism working. The uncached push loads `popCursor_` and the uncached pop loads `pushCursor_` on every operation, so the peer's cache line bounces between cores. The cached version keeps the owner cursor line local and refreshes the peer cursor only at the full/empty boundary. With 65,536 slots, both threads run long stretches without touching each other's line.

Mean throughput rises from 339.99 M/s to 1.100 G/s, 3.24x. Median latency drops from 2.92 ns to 0.962 ns. The pre-filled slacked benchmark shows the same direction: 392.37 M/s to 814.53 M/s.

Against Log 1's reset implementation, cached mean latency fell from 3.01 ns to 0.928 ns and mean throughput rose from 337.15 M/s to 1.100 G/s. That delta is directional, not controlled: Log 1 predates per-thread pinning and measured the broken reset queue. Even the unchanged uncached implementation slowed from 419.31 M/s to 339.99 M/s between logs, so the cross-log shift reflects pinning and run conditions as much as the cursor change.

Log 1's branch-misprediction explanation was based on the broken reset implementation, so it is not the explanation for this result. Run-to-run variance remains high (cached CV 13.41%), so repeated runs and medians are required before quoting a speedup.

# Planned optimizations
Orderbook optimization
- intrusive slab orderbook
- event log spsc
