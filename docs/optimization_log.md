# log 1
```bash
cmake --preset default -DCMAKE_BUILD_TYPE=Release
cmake --build build
taskset -c 2,4 ./build/benchmarks --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
```
git tag: bench-1-baseline
```bash
2026-10-17T17:29:58+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.72, 0.25, 0.17
-----------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                               Time             CPU   Iterations UserCounters...
-----------------------------------------------------------------------------------------------------------------------------------------
BM_Orderbook_Map_Throughput/iterations:121556_mean                                   79.3 ns         79.3 ns           10 items_per_second=12.6295M/s
BM_Orderbook_Map_Throughput/iterations:121556_median                                 78.2 ns         78.2 ns           10 items_per_second=12.7893M/s
BM_Orderbook_Map_Throughput/iterations:121556_stddev                                 2.87 ns         2.87 ns           10 items_per_second=424.06k/s
BM_Orderbook_Map_Throughput/iterations:121556_cv                                     3.63 %          3.62 %            10 items_per_second=3.36%
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_mean                   72.2 ns         72.2 ns           10 items_per_second=13.8519M/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_median                 72.1 ns         72.1 ns           10 items_per_second=13.8701M/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_stddev                0.204 ns        0.192 ns           10 items_per_second=36.7175k/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_cv                     0.28 %          0.27 %            10 items_per_second=0.27%
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_mean                   75.2 ns         75.2 ns           10 items_per_second=13.3016M/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_median                 75.1 ns         75.1 ns           10 items_per_second=13.3099M/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_stddev                0.352 ns        0.352 ns           10 items_per_second=62.2105k/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_cv                     0.47 %          0.47 %            10 items_per_second=0.47%
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_mean         73.8 ns         73.8 ns           10 items_per_second=13.554M/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_median       73.8 ns         73.8 ns           10 items_per_second=13.556M/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_stddev      0.275 ns        0.275 ns           10 items_per_second=50.6271k/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_cv           0.37 %          0.37 %            10 items_per_second=0.37%
BM_FeedHandler_Throughput/iterations:282229685_mean                                  6.07 ns         6.07 ns           10 items_per_second=164.849M/s
BM_FeedHandler_Throughput/iterations:282229685_median                                6.06 ns         6.06 ns           10 items_per_second=165.115M/s
BM_FeedHandler_Throughput/iterations:282229685_stddev                               0.030 ns        0.030 ns           10 items_per_second=808.386k/s
BM_FeedHandler_Throughput/iterations:282229685_cv                                    0.49 %          0.49 %            10 items_per_second=0.49%
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_mean                     2.38 ns         2.34 ns           10 items_per_second=422.272M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_median                   2.30 ns         2.30 ns           10 items_per_second=435.49M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_stddev                  0.162 ns        0.093 ns           10 items_per_second=26.5571M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_cv                       6.81 %          3.98 %            10 items_per_second=6.29%
```

# Planned optimizations
Orderbook optimization
- event log spsc 
- intrusive slab orderbook

SPSC Fifo
- cached pushCursor/popCursor
