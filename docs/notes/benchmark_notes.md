# Concurrency Performance Testing and Benchmarking
"https://awesome-embedded-learning-studio.github.io/Tutorial_AwesomeModernCPP/en/vol5-concurrency/ch08-debug-testing-perf/02-concurrency-benchmarks"

# Multithread benchmark example:
```c
#include <benchmark/benchmark.h>
#include <atomic>

// A multithreaded benchmark of an atomic counter
static void bm_atomic_counter(benchmark::State& state)
{
    std::atomic<int> counter{0};
    const int num_threads = state.threads();

    for (auto _ : state) {
        // Each thread performs one atomic increment
        counter.fetch_add(1, std::memory_order_relaxed);
        benchmark::ClobberMemory();
    }

    // Use wall time; otherwise CPU time accumulates across threads
    state.SetItemsProcessed(state.iterations());
}

// Test with 1/2/4/8 threads
BENCHMARK(bm_atomic_counter)
    ->ThreadRange(1, 8)
    ->UseRealTime();

BENCHMARK_MAIN();
```
- Can use Threads(n) or ThreadRange to sweep through different thread counts automatically
- UseRealTime() is important since without it, framework reports CPU time by default, and under multithreading the CPU time is the sum across all hredas
- ClobberMemroy() is a compiler-level memeory barrier that tells compiler "do not cache any memory state", preventing the optimizer from optiming our atomic operations away

# Design pitfalls with Concurrent Benchmarking

## Warm-up: Cold Start and Steady State
CPU's cache Hierarchy(L1,L2,L3) affects performance by orders of magnitude
- first time you access data, it loads from main memory costing 100-300 CPU cycles
- on second access its in L1 cache and takes only 3-4 cycles
- if benchmark does no warm-up, data loading in first iteration will inflate avg time

using GBench's KeepRunning() loop performs a certian amount of warm-up itself
- the framework first runs a small number of iterations to "settle" results
- but if allocated a large block of memory outside loop, that memory may not be in cache during first iteration
- if goal is "steady-state" performance, can run few manual passes before the loop
```c
static void bm_with_warmup(benchmark::State& state)
{
    std::vector<int> data(10000);

    // Warm-up: pull the data into the cache
    for (int i = 0; i < 100; ++i) {
        volatile int dummy = data[0];
        (void)dummy;
    }

    for (auto _ : state) {
        int sum = 0;
        for (int v : data) {
            sum += v;
        }
        benchmark::DoNotOptimize(sum);
    }
}
```
## Compiler Optimization
Compiler's job is to make code faster, but your goal is to measure code's raw speed

GBench provides 
- benchmark::DoNoteOptimize(result)
  - Tell the compiler the value of expr "might" be used externally; do not optimize it away
- benchmark::ClobberMemory()
  - Tell the compiler that all memory state may be modified externally
```c
for (auto _ : state) {
    int result = expensive_computation();
    benchmark::DoNotOptimize(result);
    benchmark::ClobberMemory();
}
```
- DoNoteOptimize guarantees that result is not optimized away
- ClobberMemory guarantees that each iteration's memory reads are not optimized into "we already read this last time, just reuse it"

## False Sharing
Two-threads each modify differnt variables, but those variables happen to sit on same cache line (usually 64 bytes), so every write has to invalidate the other core's copy of htel ine
- with padding, each counter owns a cache line, threads stop interfering with one another and performance is nearly independent of thread count

## Thread Creation: Don't spawn threads inside loop
Do note create and destroy threads inside benchmark loop
- it is an expensive operation typically cost 50-200 microseconds

better to create threads outside loop (with thread pool, for instance) and only submit tasks and wait for results inside the loop
- GBench's Threads(n) already creates the threads for you outside the loop
- all you need to do inside loop body is actual work

# perf stat
```bash
# Run the program directly
perf stat ./your_program

# Focus on specific events only
perf stat -e cache-misses,cache-references,context-switches,cpu-migrations ./your_program

 Performance counter stats for './your_program':

          2345.67 msec  task-clock              #  3.821 CPUs utilized
                15      context-switches        #  6.395 /sec
                 2      cpu-migrations          #  0.852 /sec
             10457      page-faults             #  4.459 K/sec
     8,234,567,890      cycles                  #  3.510 GHz
     5,678,901,234      instructions            #  0.69  insn per cycle
       456,789,012      cache-references        # 194.857 M/sec
        12,345,678      cache-misses            #  2.70% of all cache refs

       0.614234567 seconds time elapsed

       0.520000000 seconds user
       1.890000000 seconds sys
```
Important metrics:
cache-misses
- how many times CPU failed to find data in cache and had to fetch it from main memory
- 2-3% is normal for program with sequential access pattern, but for concurrent program if notice cache-miss rate soaring as thread count grow, there is probably false sharing/data-layout problem

context-switches
- reflects how often OS swaps threads in and out
- high context-switch count usually means threads are blocking frequently (waiting on mutex or I/O or being overscheduled because the thread count far exceeds number of CPU cores)

cpu-migration
- if high- likely means OS is moving threads from one core to another
- CPU migration invaldiates L1/L2 cache (since L1/L2 are private to each core) which hurts performance signicantly
- In concurrent programs where threads are migrated frequently, consider pinning threads to specific core with `pthread_set_affinitiy_np` or `taskset`
```bash
# Run on cores 0-3 only
taskset -c 0-3 ./your_program
```
