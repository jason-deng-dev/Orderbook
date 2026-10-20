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

**event logging inside orderbook incurs heavy hot path run time cost**
Currently, we perform eventLog.emplace_back() across Add/Delete/Modify operations. On the hotpath this incurs the cost of consuming resources to constructing an event in place in our eventLog, across every single operation, which I believe adds a meaningful penalty to our performance.

I believe that if we offload the logging to a dedicated thread via a lock-free SPSC ring buffer, we can eliminate hot-path memory writes and increase our Orderbook throughput.

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


# Log 3: Remove total_volume from PriceLevel (git tag: v3-price-level-refactor)

## Changes
Remove total_volume from PriceLevel
Remove the bookkeeping of the variable from Orderbook_Map and Orderbook_Vector Implemenations

## Hypothesis
```c++
HandleFill(){
  priceLevel.total_volume -= matchQty;
}
AddOrder() {
  priceLevel.total_volume += matchResult.restingVolume;
}
DeleteOrder() {
 priceLevel.total_volume -= orderIt->volume;
}

ModifyOrder() {
  priceLevel.total_volume += volumeChange;
}
```
I believe that by removing total_volume from PriceLevel, and by removing the bookkeeping from Add/Modify/Delete orders, as well as from HandleFill, that we will see a slight and noticable increase in throughput thanks to the decreased overhead from having to update total volume.

## Snapshot before change
```bash
taskset -c 2,4 perf stat -e 'cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock' ./build/benchmarks --benchmark_filter=BM_Orderbook --benchmark_repetitions=100 --benchmark_report_aggregates_only=true
2026-10-19T14:48:32+08:00
Running ./build/benchmarks
Run on (16 X 4691.07 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.10, 0.30, 0.19
-----------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                               Time             CPU   Iterations UserCounters...
-----------------------------------------------------------------------------------------------------------------------------------------
BM_Orderbook_Map_Throughput/iterations:121556_mean                                   77.7 ns         77.7 ns          100 items_per_second=12.8792M/s
BM_Orderbook_Map_Throughput/iterations:121556_median                                 77.6 ns         77.6 ns          100 items_per_second=12.8878M/s
BM_Orderbook_Map_Throughput/iterations:121556_stddev                                 1.01 ns         1.01 ns          100 items_per_second=153.374k/s
BM_Orderbook_Map_Throughput/iterations:121556_cv                                     1.30 %          1.29 %           100 items_per_second=1.19%
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_mean                   74.1 ns         74.1 ns          100 items_per_second=13.514M/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_median                 73.7 ns         73.7 ns          100 items_per_second=13.5747M/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_stddev                 2.91 ns         2.87 ns          100 items_per_second=440.605k/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_cv                     3.93 %          3.88 %           100 items_per_second=3.26%
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_mean                   77.1 ns         77.1 ns          100 items_per_second=12.9835M/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_median                 76.8 ns         76.8 ns          100 items_per_second=13.0287M/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_stddev                 2.88 ns         2.88 ns          100 items_per_second=399.192k/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_cv                     3.73 %          3.73 %           100 items_per_second=3.07%
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_mean         74.7 ns         74.7 ns          100 items_per_second=13.3955M/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_median       74.6 ns         74.6 ns          100 items_per_second=13.398M/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_stddev      0.485 ns        0.477 ns          100 items_per_second=85.3791k/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_cv           0.65 %          0.64 %           100 items_per_second=0.64%

 Performance counter stats for './build/benchmarks --benchmark_filter=BM_Orderbook --benchmark_repetitions=100 --benchmark_report_aggregates_only=true':

       32604097722      cycles                                                                
       58935751355      instructions                                                          
          62500335      cache-misses                                                          
        1578835982      cache-references                                                      
         260056155      branch-misses                                                         
       12528626113      branches                                                              
           6780.53 msec task-clock                                                            

       6.787662535 seconds time elapsed

       5.557717000 seconds user
       1.219499000 seconds sys
```

## Snapshot after change
```bash
taskset -c 2,4 perf stat -e 'cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock' ./build/benchmarks --benchmark_filter=BM_Orderbook --benchmark_repetitions=100 --benchmark_report_aggregates_only=true
2026-10-19T17:06:45+08:00
Running ./build/benchmarks
Run on (16 X 4691.07 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.16, 0.12, 0.04
-----------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                               Time             CPU   Iterations UserCounters...
-----------------------------------------------------------------------------------------------------------------------------------------
BM_Orderbook_Map_Throughput/iterations:121556_mean                                   77.5 ns         77.5 ns          100 items_per_second=12.9305M/s
BM_Orderbook_Map_Throughput/iterations:121556_median                                 77.0 ns         77.0 ns          100 items_per_second=12.9882M/s
BM_Orderbook_Map_Throughput/iterations:121556_stddev                                 4.25 ns         4.24 ns          100 items_per_second=467.908k/s
BM_Orderbook_Map_Throughput/iterations:121556_cv                                     5.48 %          5.47 %           100 items_per_second=3.62%
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_mean                   73.2 ns         73.1 ns          100 items_per_second=13.6735M/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_median                 73.3 ns         73.2 ns          100 items_per_second=13.6542M/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_stddev                0.816 ns        0.657 ns          100 items_per_second=123.328k/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_cv                     1.12 %          0.90 %           100 items_per_second=0.90%
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_mean                   75.3 ns         75.3 ns          100 items_per_second=13.2739M/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_median                 75.3 ns         75.3 ns          100 items_per_second=13.2761M/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_stddev                0.539 ns        0.536 ns          100 items_per_second=94.5346k/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_cv                     0.72 %          0.71 %           100 items_per_second=0.71%
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_mean         73.1 ns         73.1 ns          100 items_per_second=13.6819M/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_median       73.2 ns         73.1 ns          100 items_per_second=13.6763M/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_stddev      0.594 ns        0.566 ns          100 items_per_second=106.023k/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_cv           0.81 %          0.77 %           100 items_per_second=0.77%

 Performance counter stats for './build/benchmarks --benchmark_filter=BM_Orderbook --benchmark_repetitions=100 --benchmark_report_aggregates_only=true':

       32102554507      cycles                                                                
       58803641676      instructions                                                          
          95033949      cache-misses                                                          
        1905246759      cache-references                                                      
         262368441      branch-misses                                                         
       12586637248      branches                                                              
           9004.98 msec task-clock                                                            

       9.063867765 seconds time elapsed

       4.706466000 seconds user
       4.279694000 seconds sys
```












## Stat comparison

### Per-benchmark results (100 repetitions)

| Benchmark | Mean latency (before → after) | Δ | Median latency (before → after) | Δ | Mean throughput (before → after) | Δ |
|-----------|-------------------------------|---|-------------------------------|------|----------------------------------|---|
| BM_Orderbook_Map | 77.7 → 77.5 ns | -0.26% | 77.6 → 77.0 ns | -0.77% | 12.879 → 12.931 M/s | +0.40% |
| BM_Orderbook_Vector_LinearSearch | 74.1 → 73.2 ns | -1.22% | 73.7 → 73.3 ns | -0.54% | 13.514 → 13.674 M/s | +1.18% |
| BM_Orderbook_Vector_BinarySearch | 77.1 → 75.3 ns | -2.34% | 76.8 → 75.3 ns | -1.95% | 12.984 → 13.274 M/s | +2.24% |
| BM_Orderbook_Vector_BranchlessBinarySearch | 74.7 → 73.1 ns | -2.14% | 74.6 → 73.2 ns | -1.88% | 13.396 → 13.682 M/s | +2.14% |

### Perf stat comparison

| Metric | Before | After | Change |
|--------|--------|-------|--------|
| cycles | 32.60 G | 32.10 G | -1.54% |
| instructions | 58.94 G | 58.80 G | -0.22% |
| IPC | 1.81 | 1.83 | +1.34% |
| branches | 12.53 G | 12.59 G | +0.46% |
| branch misses | 260.06 M | 262.37 M | +0.89% |
| branch miss rate | 2.0757% | 2.0845% | +0.0088 pp |
| cache references | 1.579 G | 1.905 G | +20.67% |
| cache misses | 62.50 M | 95.03 M | +52.05% |
| cache miss rate | 3.9586% | 4.9880% | +1.0294 pp |
| task-clock | 6780.53 ms | 9004.98 ms | +32.81% |
| time elapsed | 6.79 s | 9.06 s | +33.53% |

## Analysis
We observe decreases in mean and median latency across all implementations. Map is effectively flat (-0.26% mean, -0.77% median). The vector implementations improve: Linear Search -1.22% mean / -0.54% median, Binary Search -2.34% / -1.95%, and Branchless Binary Search -2.14% / -1.88%.

Binary and Branchless are the largest movers, and both exceed the noise level of this run (after-change CV 0.72% and 0.81%). Linear Search's mean gain is larger than its median gain, so part of that delta may be outlier-driven. Map shows no meaningful change.

The perf counters do not corroborate a 2% per-item improvement: cycles and instructions are nearly unchanged (-1.54% and -0.22%) and IPC rises slightly, while task-clock and elapsed time rise by about a third because system time increased from 1.22 s to 4.28 s. 

In conclusion I believe that in our hotpath which is dominated by emplace, tree traversal (in map implementation), and linear/binary/branchless search (in vector implementation), as well as other O(1) operations, our removal of a single O(1) operation from our hotpath is not able to meaningfully produce any delta.

# Log 4: Slab allocator + Intrusive Order (git tag: v4-slab-intrusive-orderbook)
## Snapshot
```bash
taskset -c 2,4 ./build/benchmarks --benchmark_filter=BM_Orderbook --benchmark_repetitions=1000 --benchmark_report_aggregates_only=true
2026-10-20T09:49:46+08:00
Running ./build/benchmarks
Run on (16 X 4691.07 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.09, 0.04, 0.07
-----------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                               Time             CPU   Iterations UserCounters...
-----------------------------------------------------------------------------------------------------------------------------------------
BM_Orderbook_Map_Throughput/iterations:121556_mean                                   78.1 ns         78.1 ns         1000 items_per_second=12.8094M/s
BM_Orderbook_Map_Throughput/iterations:121556_median                                 78.0 ns         78.0 ns         1000 items_per_second=12.8274M/s
BM_Orderbook_Map_Throughput/iterations:121556_stddev                                 1.88 ns         1.88 ns         1000 items_per_second=279.352k/s
BM_Orderbook_Map_Throughput/iterations:121556_cv                                     2.41 %          2.41 %          1000 items_per_second=2.18%
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_mean                   74.4 ns         74.3 ns         1000 items_per_second=13.4575M/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_median                 74.2 ns         74.2 ns         1000 items_per_second=13.4728M/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_stddev                 1.59 ns         1.17 ns         1000 items_per_second=193.966k/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_cv                     2.13 %          1.58 %          1000 items_per_second=1.44%
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_mean                   76.6 ns         76.6 ns         1000 items_per_second=13.0617M/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_median                 76.8 ns         76.8 ns         1000 items_per_second=13.0232M/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_stddev                 2.52 ns         2.52 ns         1000 items_per_second=311.706k/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_cv                     3.29 %          3.28 %          1000 items_per_second=2.39%
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_mean         74.7 ns         74.7 ns         1000 items_per_second=13.3907M/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_median       75.1 ns         75.1 ns         1000 items_per_second=13.3194M/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_stddev       1.41 ns         1.41 ns         1000 items_per_second=239.135k/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_cv           1.89 %          1.89 %          1000 items_per_second=1.79%
BM_Orderbook_Slab_LinearSearch_Throughput/iterations:121556_mean                     63.5 ns         63.5 ns         1000 items_per_second=15.7392M/s
BM_Orderbook_Slab_LinearSearch_Throughput/iterations:121556_median                   63.5 ns         63.5 ns         1000 items_per_second=15.7473M/s
BM_Orderbook_Slab_LinearSearch_Throughput/iterations:121556_stddev                  0.569 ns        0.569 ns         1000 items_per_second=137.139k/s
BM_Orderbook_Slab_LinearSearch_Throughput/iterations:121556_cv                       0.90 %          0.90 %          1000 items_per_second=0.87%
BM_Orderbook_Slab_BinarySearch_Throughput/iterations:121556_mean                     65.3 ns         65.3 ns         1000 items_per_second=15.3074M/s
BM_Orderbook_Slab_BinarySearch_Throughput/iterations:121556_median                   65.3 ns         65.3 ns         1000 items_per_second=15.3152M/s
BM_Orderbook_Slab_BinarySearch_Throughput/iterations:121556_stddev                  0.585 ns        0.581 ns         1000 items_per_second=135.511k/s
BM_Orderbook_Slab_BinarySearch_Throughput/iterations:121556_cv                       0.90 %          0.89 %          1000 items_per_second=0.89%
BM_Orderbook_Slab_BranchlessBinarySearch_Throughput/iterations:121556_mean           62.9 ns         62.9 ns         1000 items_per_second=15.9066M/s
BM_Orderbook_Slab_BranchlessBinarySearch_Throughput/iterations:121556_median         62.8 ns         62.8 ns         1000 items_per_second=15.9245M/s
BM_Orderbook_Slab_BranchlessBinarySearch_Throughput/iterations:121556_stddev        0.657 ns        0.651 ns         1000 items_per_second=162.599k/s
BM_Orderbook_Slab_BranchlessBinarySearch_Throughput/iterations:121556_cv             1.04 %          1.04 %          1000 items_per_second=1.02%
```

## Stat comparison

### All implementations (1000 repetitions, 121556 iterations)

| Benchmark | Mean latency | Median latency | Stddev | CV | Mean throughput |
|-----------|--------------|----------------|--------|----|-----------------|
| Map | 78.1 ns | 78.0 ns | 1.88 ns | 2.41% | 12.8094 M/s |
| Vector — LinearSearch | 74.4 ns | 74.2 ns | 1.59 ns | 2.13% | 13.4575 M/s |
| Vector — BinarySearch | 76.6 ns | 76.8 ns | 2.52 ns | 3.29% | 13.0617 M/s |
| Vector — BranchlessBinarySearch | 74.7 ns | 75.1 ns | 1.41 ns | 1.89% | 13.3907 M/s |
| Slab — LinearSearch | 63.5 ns | 63.5 ns | 0.569 ns | 0.90% | 15.7392 M/s |
| Slab — BinarySearch | 65.3 ns | 65.3 ns | 0.585 ns | 0.90% | 15.3074 M/s |
| **Slab — BranchlessBinarySearch** | **62.9 ns** | **62.8 ns** | 0.657 ns | 1.04% | **15.9066 M/s** |

### Slab vs existing implementations

Vector comparison is per search policy. Map has a single configuration (tree lookup), so it is compared against the fastest Slab variant.

| Search policy | Vector latency | Slab latency | Δ latency | Vector throughput | Slab throughput | Δ throughput |
|---------------|----------------|--------------|-----------|-------------------|-----------------|--------------|
| LinearSearch | 74.4 ns | 63.5 ns | -14.7% | 13.4575 M/s | 15.7392 M/s | +17.0% |
| BinarySearch | 76.6 ns | 65.3 ns | -14.8% | 13.0617 M/s | 15.3074 M/s | +17.2% |
| BranchlessBinarySearch | 74.7 ns | 62.9 ns | -15.8% | 13.3907 M/s | 15.9066 M/s | +18.8% |
| Map (reference, fastest Slab) | 78.1 ns | 62.9 ns | -19.5% | 12.8094 M/s | 15.9066 M/s | +24.2% |

### Analysis

Every Slab variant still beats every non-Slab implementation on both mean latency and throughput. Slab BranchlessBinarySearch is the fastest configuration overall at 62.9 ns mean (15.9066 M/s), 15.5% faster than the fastest non-Slab implementation (Vector LinearSearch, 74.4 ns) and 19.5% faster than Map.

Within the Slab family, BranchlessBinarySearch (62.9 ns) now edges out LinearSearch (63.5 ns) by 0.6 ns, or 0.9%. This ordering is not stable across runs: the previous snapshot had LinearSearch ahead by 0.1 ns (63.2 vs 63.3), so the two remain effectively tied. BinarySearch is consistently the slowest Slab variant at 65.3 ns, 3.8% slower than Branchless and 2.8% slower than Linear.

The Slab family is also the most stable this run: CVs are 0.90%, 0.90%, and 1.04%, versus 1.89-3.29% for the Vector variants and 2.41% for Map. The smallest Slab-vs-Vector gain (+17.0% throughput) is still more than five times the worst CV in the table (3.29%), so the improvement is not explained by run noise.
