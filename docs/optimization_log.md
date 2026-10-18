# benchmark setup
```bash
cmake --preset default -DCMAKE_BUILD_TYPE=Release
cmake --build build
taskset -c 2,4 ./build/benchmarks --benchmark_repetitions=25 --benchmark_report_aggregates_only=true
```

# Log 1
git tag: bench-1-baseline
## Snapshot
```bash
taskset -c 2,4 ./build/benchmarks --benchmark_repetitions=25 --benchmark_report_aggregates_only=true
2026-10-18T07:36:22+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.48, 0.96, 0.65
-----------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                               Time             CPU   Iterations UserCounters...
-----------------------------------------------------------------------------------------------------------------------------------------
BM_Orderbook_Map_Throughput/iterations:121556_mean                                   78.2 ns         78.2 ns           25 items_per_second=12.7862M/s
BM_Orderbook_Map_Throughput/iterations:121556_median                                 78.1 ns         78.1 ns           25 items_per_second=12.8018M/s
BM_Orderbook_Map_Throughput/iterations:121556_stddev                                0.758 ns        0.756 ns           25 items_per_second=121.555k/s
BM_Orderbook_Map_Throughput/iterations:121556_cv                                     0.97 %          0.97 %            25 items_per_second=0.95%
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_mean                   72.4 ns         72.4 ns           25 items_per_second=13.8217M/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_median                 72.3 ns         72.3 ns           25 items_per_second=13.8323M/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_stddev                0.267 ns        0.268 ns           25 items_per_second=51.2204k/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_cv                     0.37 %          0.37 %            25 items_per_second=0.37%
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_mean                   76.8 ns         76.8 ns           25 items_per_second=13.0798M/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_median                 74.9 ns         74.9 ns           25 items_per_second=13.3443M/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_stddev                 5.48 ns         5.48 ns           25 items_per_second=784.014k/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_cv                     7.13 %          7.14 %            25 items_per_second=5.99%
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_mean         73.8 ns         73.8 ns           25 items_per_second=13.5552M/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_median       73.7 ns         73.7 ns           25 items_per_second=13.5768M/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_stddev      0.366 ns        0.366 ns           25 items_per_second=67.2655k/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_cv           0.50 %          0.50 %            25 items_per_second=0.50%
BM_FeedHandler_Throughput/iterations:282229685_mean                                  6.04 ns         6.03 ns           25 items_per_second=165.726M/s
BM_FeedHandler_Throughput/iterations:282229685_median                                6.03 ns         6.03 ns           25 items_per_second=165.919M/s
BM_FeedHandler_Throughput/iterations:282229685_stddev                               0.032 ns        0.033 ns           25 items_per_second=898.149k/s
BM_FeedHandler_Throughput/iterations:282229685_cv                                    0.53 %          0.54 %            25 items_per_second=0.54%
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_mean                     2.38 ns         2.37 ns           25 items_per_second=421.965M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_median                   2.32 ns         2.32 ns           25 items_per_second=430.477M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_stddev                  0.175 ns        0.171 ns           25 items_per_second=26.2752M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_cv                       7.37 %          7.21 %            25 items_per_second=6.23%
```
## Summary table

|Benchmark|Throughput|mean|median|stddev|Coefficient of Variation|
|---------|----------|----|------|------|------------------------|
| BM_Fifo_Throughput | 421.97 M/s | 2.38 ns | 2.32 ns | 0.175 ns | 7.37% |
| BM_FeedHandler_Throughput | 165.73 M/s | 6.04 ns | 6.03 ns | 0.032 ns | 0.53% |
| BM_Orderbook_Map_Throughput | 12.79 M/s | 78.2 ns | 78.1 ns | 0.758 ns | 0.97% |
| BM_Orderbook_Vector_LinearSearch_Throughput | 13.82 M/s | 72.4 ns | 72.3 ns | 0.267 ns | 0.37% |
| BM_Orderbook_Vector_BinarySearch_Throughput | 13.08 M/s | 76.8 ns | 74.9 ns | 5.48 ns | 7.13% |
| BM_Orderbook_Vector_BranchlessBinarySearch_Throughput | 13.56 M/s | 73.8 ns | 73.7 ns | 0.366 ns | 0.50% |

Times and throughput are from the full-suite snapshot above (25 repetitions). Throughput is the mean `items_per_second`. Binary search has high run-to-run variation; use its median for comparison. Perf counters are excluded because process-wide counters include static fixture loading, which dominates the orderbook benchmarks' totals.

## Analysis
  **Vector-Based Implementation Outperform `std::map`**

All contiguous vector implementations achieved lower mean latency (72.4 - 76.8ns) compared to `std::map` baseline (78.2ns).
- Linear Search achieved 8.011% lower mean latency
- Binary Search achieved 1.823% lower mean latency
- Branchless Binary Search achieved 5.962% lower mean latency

Since `std::map` is a node-based red-black tree and thus requires heap-allocated nodes, we encounter pointer chasing.

Where each node lookup likely incurs an L1/L2 cache miss, forcing CPU to fetch data from slower L3 cache or main memory. Converslely, our `std::vector` implementations stores PriceLevel structs in contiguous memory.

Thus even with the additional overhead of maintaining sorted order, the contiguous layout allows the CPU's hardware prefetcher to load subsequent price levels into the L1 cache ahead of time, signficantly reducing memory latency.

**Linear Search > Branchless Binary Search > Binary Search**
For our filtered, single-stock dataset, Linear Search (72.4 ns) proved fastest, followed by Branchless Binary (73.8 ns), with Standard Binary Search being the slowest (76.8 ns).

Since we filter the ITCH feed to a single liquid ticker, the number of active price levels (N) is small (typically < 100).

Why Linear wins:
- For small N, the CPU's branch predictor easily predicts the linear loop. Furthermore, since linear search accesses memory strictly sequentially, perfectly, we are able to perfectly saturate our L1 cache bandwidth and maximize hardware prefetching.

Why Branchless beats Standard Binary:
- Standard Binary Search relies on if/else branches to narrow the search space. On a small, unpredictable dataset, this causes branch mispredictions, which flushes CPU pipeline (~15-20 cycle penalty). 
- Branchless binary search replaces these branches with conditional moves, `first += comp(first[half].first, price) * (length - half);` eliminating the misprediction penalty and making it faster than standard binary. 

However both binary approaches still suffer from random memory acces jumps, which breaks hardware prefetching, keeping them strictly slower than the seqeuntial scan of Linear Search.

**Binary Search has a high run-to-run variation**
Standard Binary Search shows a signficiantly higher standard deviation (5.48ns, 7.13%CV) compared to the other implementations, which all maintain a CV under 1%.

This variance is liekly largly driven by non-determinstic branch mispredictions
- since the execution time of a standard binary search depends heavily on the specific sequence of incoming prices
- if a sequence of messages consistently queries price that fall on the "unpredicted" side of the binary search points, the branch predictor fails repeatedly, causing massive and unpredicible pipeline flushes

Linear Search and Branchless Binary Search have deterministic memory access patterns and minimal-to-zero branch mispredictions, resulting in rock-solid, highly consistent execution times regardless of the input data sequence.

### 










# Planned optimizations
Orderbook optimization
- intrusive slab orderbook
- event log spsc 


SPSC Fifo
- cached pushCursor/popCursor
