# perf usage
```c
void RunPerf(){
  pid_t pid = fork();
  if (pid == 0) {
    const auto parentPid = std::to_string(getppid());
    std::cout<< "Running perf on parent process " << parentPid << std::endl;
    execlp("perf", "perf", "stat", "-", parentPid.c_str(), (char*)nullptr);
    throw std::runtime_error("execlp failed");
  }
}

void InitAndRunBenchmark() {
  InitBenchmark(); // might take long time!
  RunPerf();
  RunBenchmark();
}
```
- approach won't work for micro benchmark, thus we benchmark on NASDAQ_ITCH50 data fed to our benchmark through our Feed_Handler
- we start perf after the Init process since it might take a long time, thus distorting the accuracy of our performance results

```c
execlp("perf", "perf", "stat", "-", parentPid.c_str(), (char*)nullptr);
```
- replaces the current process (child) with a completely new program (perf)
- the child process stops being a copy of benchmark and becomes perf

|Argument| What it is  | Why|
|--|--|--|
"perf" (1st) | program to run | tells OS find perf in PATH and execute it
"perf" (2nd) | argv[0] | by convention, first argument to any C program is its own name
"stat" | argv[1] | tells perf to run in stat mode (count events)
"-p" | argv[2] | attach to PID
parentPid.c_str() | argv[3] | actual PID of benchmark
(char*)nullptr| terminator | tells execlp  (variadic function) this is the end of list

# experimental logging
perform benchmark/perf on different versions of the Orderbook/search policies/data structure implementations to see the performance differences

On each run record a experiment log, to be able to compare authoriatively the effects our refactors/different approaches 

```
date:
git commit:
compiler + flags:
kernel + perf version:
PU model + topology + SMT state:
governor/frequency notes:
workload + seed + duration:
pinning:
perf stat command:
primary metric:
result (median + spread):
interpretation:
boundary/when this should not generalize:
```

# benchmark setup

## Use perf stat as needed
perf stat
- if high cache miss rate
  - probably false sharing/data-layout issue
- if high context-switch count
  - threads are blocking frequently (waiting on mmutex or I/O or thread count far exceeds number of CPU cores)
<!-- - if high cpu-migration count -->
  - likely means OS is moving threads from one core to another (invalidates L1/L2 caches) pin threads using `taskset`

## CPU pinning
```bash
$ lscpu -e

CPU NODE SOCKET CORE L1d:L1i:L2:L3 ONLINE
  0    0      0    0 0:0:0:0          yes
  1    0      0    0 0:0:0:0          yes
  2    0      0    1 1:1:1:0          yes
  3    0      0    1 1:1:1:0          yes
  4    0      0    2 2:2:2:0          yes
  5    0      0    2 2:2:2:0          yes
  6    0      0    3 3:3:3:0          yes
  7    0      0    3 3:3:3:0          yes
  8    0      0    4 4:4:4:0          yes
  9    0      0    4 4:4:4:0          yes
 10    0      0    5 5:5:5:0          yes
 11    0      0    5 5:5:5:0          yes
 12    0      0    6 6:6:6:0          yes
 13    0      0    6 6:6:6:0          yes
 14    0      0    7 7:7:7:0          yes
 15    0      0    7 7:7:7:0          yes
```
```bash
# pin to cores 0 and 2
taskset -c 2,4 ./benchmark
```
- Logical CPUs 0 and 1 are SMT siblingso n the same physical core (core 0)
- To avoid SMT contention in the two-threaded SPSC benchmark, I will pin the consumer and consumer to logical CPUs 0 and 2
  - which resides on distinct physical cores (core 0 and core 1)
  - this ensures the measurement reflects inter-core communication rather than sibling hyperthreaded contention

## benchmark in release build

## Compiler Optimizations / Link-Time Optimization (LTO)

The orderbook benchmark compares two implementations that do not compile the same way by default:
- `Orderbook_Vector` is header-only templates, so all of its code is compiled inside the benchmark translation unit and gets inlined into the benchmark loop.
- `Orderbook_Map` keeps `AddOrder`, `DeleteOrder`, and `ModifyOrder` in `src/orderbook_map.cpp`, compiled into the `orderbook_core` static library.

If only the `benchmarks` target gets `-march=native`, the vector gets the native instruction set and cross-function inlining while the map does not. The difference would show up as a data-structure result even though it is really a build result. To remove that confound:
- compile `orderbook_core` with the same `-march=native -mtune=native`
- enable LTO so a call from `ProcessMessage` (header) into `orderbook_map.cpp` can inline across translation units, the same way the vector's header code already does

```cmake
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported LANGUAGES CXX)
if(ipo_supported)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

# Match benchmark codegen so map and vector are not compared across different ISA flags.
target_compile_options(orderbook_core PRIVATE
  $<$<CONFIG:Release>:-march=native -mtune=native>
)
```

- `check_ipo_supported` avoids hardcoding `-flto`: the correct compile and link flags depend on the toolchain (GCC/Clang use `-flto`, MSVC uses `/GL` and `/LTCG`), and CMake applies the matching flags to both compilation and linking.
- `CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE` initializes LTO for every Release target created after it, so `orderbook_core` and `benchmarks` are optimized and linked consistently.
- `-O3` already comes from `CMAKE_CXX_FLAGS_RELEASE` (`-O3 -DNDEBUG`); the benchmark target only needs to add ISA flags.
- `-march=native` lets the compiler use this CPU's instruction set. `-mtune=native` tunes instruction scheduling for this CPU without changing which instructions are allowed.
- `-march=native` makes the binary machine-specific. Benchmark numbers are only valid on the machine that built the binary, and the CPU model should be recorded in the experiment log.
- LTO plus matched flags means map vs vector differences now come from data layout and algorithm, not from one implementation being inlined and the other called across a library boundary.


## avoid compiler optimization by using GBench
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

## avoid warm up
- by using GBench's KeepRunning()
- or if allocated large block of memory outside loop, run few manual passes before loop

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

# SPSC Fifo benchmark
I will be performing the throughput test on my SPSC Fifo via GoogleBenchmark, using the settings/methodology specified in `benchmark setup`
```c
QueueItem payload;
SpscFifo<QueueItem> fifo{};
QueueItem out;

static void BM_Throughput(benchmark::State &state) {
  for (auto _ : state) {
    if (state.thread_index() == 0) {
      while (!fifo.push(payload))
        ;
    }
    else if (state.thread_index() == 1) {
      
      while (!fifo.pop(out))
        ;
      benchmark::DoNotOptimize(out);
    }
  }
  state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_Throughput)->Threads(2)->Iterations(50000000);
```
I run the BM_Throughput using 2 threads, where inside the benchmark loop, depending on the states thread index the thread is assigned the role of producer (thread index == 0) or consumer (thread index == 1)

Importantly I don't perform correctness check inside the benchmark for throughput, since the correctness is validated through our testing, and thus our benchmark should capture the pure performance of our SPSC Fifo assuming correctness.

# Feed Handler benchmark
To measure the raw parsing and filtering throughput of the Feed Handler, we benchmark the GetNextMessage() method against an mmap'd NASDAQ ITCH 5.0 data buffer. This method returns a std::optional<QueueItem> for state-modifying events and std::nullopt for administrative messages. By running this over a live capture file, the benchmark accurately captures the CPU cost of parsing, branching, and filtering, weighted by the real-world distribution of message types.


# Orderbook benchmark
To measure the absolute maximum throughput of the Orderbook, we isolate it from the upstream components (Feed Handler, SPSC Fifo) to eliminate I/O and IPC bottlenecks.

We pre-allocate an array of 1 million parsed QueueItem messages in memory and invoke the Orderbook's `process_message()` function directly within the benchmark loop.

This array is cycled continously to sustain the load, accurately capturing the raw matching engine performacne under a realistic distribution of order types.

# System end-to-end benchmarking
Consumer Thread: Orderbook will pop from SPSC Fifo and process the order
Producer Thread: Feed_Handler will push messages into the SPSC Fifo

