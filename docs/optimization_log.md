# benchmark setup
```bash
cmake --preset default -DCMAKE_BUILD_TYPE=Release
cmake --build build
taskset -c 2,4 ./build/benchmarks --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
```

# log 1
git tag: bench-1-baseline

All benchmark and perf runs below are pinned with `taskset -c 2,4`.

## Snapshots

### Full suite
```bash
taskset -c 2,4 ./build/benchmarks --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
2026-10-17T19:46:22+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.31, 0.13, 0.10
-----------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                               Time             CPU   Iterations UserCounters...
-----------------------------------------------------------------------------------------------------------------------------------------
BM_Orderbook_Map_Throughput/iterations:121556_mean                                   79.7 ns         79.7 ns           10 items_per_second=12.5681M/s
BM_Orderbook_Map_Throughput/iterations:121556_median                                 78.4 ns         78.4 ns           10 items_per_second=12.7515M/s
BM_Orderbook_Map_Throughput/iterations:121556_stddev                                 3.49 ns         3.45 ns           10 items_per_second=508.373k/s
BM_Orderbook_Map_Throughput/iterations:121556_cv                                     4.38 %          4.33 %            10 items_per_second=4.04%
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_mean                   76.3 ns         76.3 ns           10 items_per_second=13.1193M/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_median                 75.7 ns         75.7 ns           10 items_per_second=13.2092M/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_stddev                 3.00 ns         3.00 ns           10 items_per_second=497.711k/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_cv                     3.93 %          3.93 %            10 items_per_second=3.79%
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_mean                   76.5 ns         76.5 ns           10 items_per_second=13.0784M/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_median                 76.3 ns         76.3 ns           10 items_per_second=13.1061M/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_stddev                0.486 ns        0.485 ns           10 items_per_second=82.5113k/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_cv                     0.64 %          0.63 %            10 items_per_second=0.63%
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_mean         75.4 ns         75.4 ns           10 items_per_second=13.2573M/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_median       75.6 ns         75.6 ns           10 items_per_second=13.2189M/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_stddev      0.475 ns        0.474 ns           10 items_per_second=83.7492k/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_cv           0.63 %          0.63 %            10 items_per_second=0.63%
BM_FeedHandler_Throughput/iterations:282229685_mean                                  6.09 ns         6.08 ns           10 items_per_second=164.465M/s
BM_FeedHandler_Throughput/iterations:282229685_median                                6.09 ns         6.08 ns           10 items_per_second=164.368M/s
BM_FeedHandler_Throughput/iterations:282229685_stddev                               0.025 ns        0.022 ns           10 items_per_second=604.716k/s
BM_FeedHandler_Throughput/iterations:282229685_cv                                    0.41 %          0.37 %            10 items_per_second=0.37%
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_mean                     2.37 ns         2.36 ns           10 items_per_second=422.823M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_median                   2.34 ns         2.34 ns           10 items_per_second=426.954M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_stddev                  0.119 ns        0.088 ns           10 items_per_second=20.1802M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_cv                       5.04 %          3.72 %            10 items_per_second=4.77%
```

### BM_Fifo_Throughput
```bash
taskset -c 2,4 perf stat -e 'cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock' ./build/benchmarks --benchmark_filter=BM_Fifo_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
2026-10-17T19:46:47+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.46, 0.19, 0.11
-----------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                   Time             CPU   Iterations UserCounters...
-----------------------------------------------------------------------------------------------------------------------------
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_mean         2.32 ns         2.32 ns           10 items_per_second=430.827M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_median       2.32 ns         2.32 ns           10 items_per_second=430.393M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_stddev      0.013 ns        0.013 ns           10 items_per_second=2.5015M/s
BM_Fifo_Throughput/iterations:100000000/real_time/threads:2_cv           0.58 %          0.57 %            10 items_per_second=0.58%

 Performance counter stats for './build/benchmarks --benchmark_filter=BM_Fifo_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true':

       34046344789      cycles                                                                
       79819261359      instructions                                                          
         525412811      cache-misses                                                          
        3705260764      cache-references                                                      
          12060864      branch-misses                                                         
       16603900176      branches                                                              
           6617.69 msec task-clock                                                            

       4.301399939 seconds time elapsed

       6.556048000 seconds user
       0.059963000 seconds sys
```

### BM_FeedHandler_Throughput
```bash
taskset -c 2,4 perf stat -e 'cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock' ./build/benchmarks --benchmark_filter=BM_FeedHandler_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
2026-10-17T19:47:40+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.56, 0.27, 0.15
----------------------------------------------------------------------------------------------------------------
Benchmark                                                      Time             CPU   Iterations UserCounters...
----------------------------------------------------------------------------------------------------------------
BM_FeedHandler_Throughput/iterations:282229685_mean         6.18 ns         6.18 ns           10 items_per_second=161.919M/s
BM_FeedHandler_Throughput/iterations:282229685_median       6.19 ns         6.19 ns           10 items_per_second=161.447M/s
BM_FeedHandler_Throughput/iterations:282229685_stddev      0.057 ns        0.057 ns           10 items_per_second=1.52089M/s
BM_FeedHandler_Throughput/iterations:282229685_cv           0.92 %          0.92 %            10 items_per_second=0.94%

 Performance counter stats for './build/benchmarks --benchmark_filter=BM_FeedHandler_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true':

       99481687796      cycles                                                                
      174659821621      instructions                                                          
           9240093      cache-misses                                                          
        3923276689      cache-references                                                      
        1598116757      branch-misses                                                         
       36649735581      branches                                                              
          19504.06 msec task-clock                                                            

      19.506071798 seconds time elapsed

      19.305244000 seconds user
       0.195972000 seconds sys
```

### BM_Orderbook_Map_Throughput
```bash
taskset -c 2,4 perf stat -e 'cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock' ./build/benchmarks --benchmark_filter=BM_Orderbook_Map_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
2026-10-17T19:47:11+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.70, 0.27, 0.14
---------------------------------------------------------------------------------------------------------------
Benchmark                                                     Time             CPU   Iterations UserCounters...
---------------------------------------------------------------------------------------------------------------
BM_Orderbook_Map_Throughput/iterations:121556_mean         78.2 ns         77.9 ns           10 items_per_second=12.8306M/s
BM_Orderbook_Map_Throughput/iterations:121556_median       77.7 ns         77.7 ns           10 items_per_second=12.8702M/s
BM_Orderbook_Map_Throughput/iterations:121556_stddev       1.36 ns        0.901 ns           10 items_per_second=146.092k/s
BM_Orderbook_Map_Throughput/iterations:121556_cv           1.73 %          1.16 %            10 items_per_second=1.14%

 Performance counter stats for './build/benchmarks --benchmark_filter=BM_Orderbook_Map_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true':

       10444047250      cycles                                                                
       35892049389      instructions                                                          
           3705100      cache-misses                                                          
        1118024216      cache-references                                                      
          10956237      branch-misses                                                         
        8660036618      branches                                                              
           2025.48 msec task-clock                                                            

       2.026656017 seconds time elapsed

       1.930074000 seconds user
       0.095904000 seconds sys
```

### BM_Orderbook_Vector_LinearSearch_Throughput
```bash
taskset -c 2,4 perf stat -e 'cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock' ./build/benchmarks --benchmark_filter=BM_Orderbook_Vector_LinearSearch_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
2026-10-17T19:48:03+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.61, 0.30, 0.16
-------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                     Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------------------------------------------
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_mean         71.7 ns         71.7 ns           10 items_per_second=13.9567M/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_median       71.4 ns         71.4 ns           10 items_per_second=14.0037M/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_stddev      0.947 ns        0.945 ns           10 items_per_second=178.925k/s
BM_Orderbook_Vector_LinearSearch_Throughput/iterations:121556_cv           1.32 %          1.32 %            10 items_per_second=1.28%

 Performance counter stats for './build/benchmarks --benchmark_filter=BM_Orderbook_Vector_LinearSearch_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true':

       10382486183      cycles                                                                
       35937745145      instructions                                                          
           3137261      cache-misses                                                          
        1113312015      cache-references                                                      
           9632030      branch-misses                                                         
        8653885405      branches                                                              
           2006.54 msec task-clock                                                            

       2.007212321 seconds time elapsed

       1.940796000 seconds user
       0.064026000 seconds sys
```

### BM_Orderbook_Vector_BinarySearch_Throughput
```bash
taskset -c 2,4 perf stat -e 'cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock' ./build/benchmarks --benchmark_filter=BM_Orderbook_Vector_BinarySearch_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
2026-10-17T19:47:15+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.73, 0.28, 0.15
-------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                     Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------------------------------------------
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_mean         75.2 ns         75.2 ns           10 items_per_second=13.3023M/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_median       74.9 ns         74.9 ns           10 items_per_second=13.3437M/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_stddev      0.757 ns        0.757 ns           10 items_per_second=132.491k/s
BM_Orderbook_Vector_BinarySearch_Throughput/iterations:121556_cv           1.01 %          1.01 %            10 items_per_second=1.00%

 Performance counter stats for './build/benchmarks --benchmark_filter=BM_Orderbook_Vector_BinarySearch_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true':

       10368027903      cycles                                                                
       35938395639      instructions                                                          
           3255360      cache-misses                                                          
        1115315979      cache-references                                                      
          10158680      branch-misses                                                         
        8646881876      branches                                                              
           2002.64 msec task-clock                                                            

       2.003460193 seconds time elapsed

       1.930124000 seconds user
       0.071930000 seconds sys
```

### BM_Orderbook_Vector_BranchlessBinarySearch_Throughput
```bash
taskset -c 2,4 perf stat -e 'cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock' ./build/benchmarks --benchmark_filter=BM_Orderbook_Vector_BranchlessBinarySearch_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
2026-10-17T19:47:17+08:00
Running ./build/benchmarks
Run on (16 X 4691.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.73, 0.28, 0.15
-----------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                               Time             CPU   Iterations UserCounters...
-----------------------------------------------------------------------------------------------------------------------------------------
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_mean         74.3 ns         74.3 ns           10 items_per_second=13.4546M/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_median       74.3 ns         74.3 ns           10 items_per_second=13.4618M/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_stddev      0.731 ns        0.732 ns           10 items_per_second=131.493k/s
BM_Orderbook_Vector_BranchlessBinarySearch_Throughput/iterations:121556_cv           0.98 %          0.98 %            10 items_per_second=0.98%

 Performance counter stats for './build/benchmarks --benchmark_filter=BM_Orderbook_Vector_BranchlessBinarySearch_Throughput --benchmark_repetitions=10 --benchmark_report_aggregates_only=true':

       10395133823      cycles                                                                
       35949742684      instructions                                                          
           3613614      cache-misses                                                          
        1116159047      cache-references                                                      
           9679307      branch-misses                                                         
        8641216091      branches                                                              
           2007.86 msec task-clock                                                            

       2.008766212 seconds time elapsed

       1.949810000 seconds user
       0.055937000 seconds sys
```

## Summary table

|Benchmark|Throughput|mean|median|stddev|Coefficient of Variation|cache miss rate|branch miss rate|
|---------|----------|----|------|------|------------------------|---------------|----------------|
| BM_Fifo_Throughput | 430.83 M/s | 2.32 ns | 2.32 ns | 0.013 ns | 0.58% | 14.180% | 0.0726% |
| BM_FeedHandler_Throughput | 161.92 M/s | 6.18 ns | 6.19 ns | 0.057 ns | 0.92% | 0.236% | 4.3605% |
| BM_Orderbook_Map_Throughput | 12.83 M/s | 78.2 ns | 77.7 ns | 1.36 ns | 1.73% | 0.331% | 0.1265% |
| BM_Orderbook_Vector_LinearSearch_Throughput | 13.96 M/s | 71.7 ns | 71.4 ns | 0.947 ns | 1.32% | 0.282% | 0.1113% |
| BM_Orderbook_Vector_BinarySearch_Throughput | 13.30 M/s | 75.2 ns | 74.9 ns | 0.757 ns | 1.01% | 0.292% | 0.1175% |
| BM_Orderbook_Vector_BranchlessBinarySearch_Throughput | 13.45 M/s | 74.3 ns | 74.3 ns | 0.731 ns | 0.98% | 0.324% | 0.1120% |

Miss rates are `cache-misses / cache-references` and `branch-misses / branches` from the perf stat runs. Counters cover the whole benchmark process, not per-message work. Feed handler and linear search entries use reruns after the first pinned perf passes showed high variance.

Pinning changed the cache-miss picture substantially, especially for `Orderbook_Map`: 2.237% unpinned versus 0.331% pinned.

# Planned optimizations
Orderbook optimization
- event log spsc 
- intrusive slab orderbook

SPSC Fifo
- cached pushCursor/popCursor
