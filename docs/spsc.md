# SPSC FIFO ring

```
ITCH file → Parser thread → [SPSC ring] → Book builder thread → Orderbook
```

Why SPSC?

- parser can run ahead while orderbook thread consumes the pushed queue items
- Zero allocation on handoff: preallocated ring, fixed-size slots. No malloc on hot path
- it may improve throughput over a single thread doing all these tasks
  - parsing from ITCH file
  - queuing up the order messages (skipping over irrelevent order messages)
  - executing the queued up order message
- it may improve resiliency of the program to spikes in message traffic

Disadvantages:

- have to manage more threads
- uses more memory, since each FIFO needs place to store its messages

# Specs

`Single Producer Single Consumer Lock-Free Wait-Free Fifo`

- Single producer: one producer/writer thread
- Single consumer: one consumer/reader thread
- Lock-free: doesn't use mutex locks. At any point of time, some thread will make progress
- Wait-free: each thread moves forward regardless of other threads
- Circular Fifo : a single, fixed-size buffer as if it were connected end-to-end. Oldest entry is processed first.

# push/pop cursor implementation

64 bit unsigned integers so there are lots of numbers available for use

- start at 0, everytime we push we increment push cursor
- probally won't run out (especially if system reboots periodically, which is standard practice for orderbook implementations)
- when you pop a value, you remove it from the pop cursor location and increment pop cursor
- eventually if no new value are added, pop cursor == push cursor and fifo becomes empty

There are different ways to convert continously increasing cursor value to offset in array

- via % operator constrain cursor to allowable range of indices on the array that is storing the value
- constrain capacity of FIFO to integral power of 2, then using & operator

## Why not use the remainder operator?

On AMD Zen 5 (9800x3d) per "https://www.agner.org/optimize/instruction_tables.pdf"

```
AND, OR, XOR

Operands    μops    Latency    Recip. Throughput
-------------------------------------------------
r, r        1       1          0.20
r, i        1       1          0.19
r, m        1       —          0.25
m, r/i      1       1          0.67

DIV

Operands    μops    Latency    Recip. Throughput
-------------------------------------------------
r8/m8       1       12         4
r16/m16     2       12         4
r32/m32     2       11–14      6
r64/m64     2       11–19      7–13
```

- which is significantly higher latency and Recipricoal throughput
- comparing AND r,r to DIV r64
  - 35-65x throughput gap

## AND(&) operator implemantion

By constraining queue's capacity to integral power of 2, can replace %operator with single-cycle bitwise AND(&) operation

Mathematically, for any non-negative integer cursor and any power of 2 capacity:
cursor % capacity == cursor & (capacity - 1)

```
capacity = 8 => binary representation = 1000
mask = capacity - 1 = 8 - 1 = 7 => binary representation = 0111

say cursor implemented to 10
10%8 = 2

10 & mask (7) => 1010 & 0111 = 0010 = 2
```

# Optimizations

- %operator vs &operator implementation
- memory ordering relaxed vs all sequentially consistent
- false sharing prevention vs no false sharing prevention
  - we achieve false sharing prevention through `alignas(kCacheLineSize)` between push cursor and pop cursor
  - achieve false sharing prevention between push/pop cursors and adjacent objects through `char padding_[skCacheLineSize - sizeof(std::size_t)];`
- caching push/popCursor via cachedPopCursor and cachedPushCursor

# False sharing prevention
Because of MESI cache coherence protocol, when Core 0 (Producer) writes to pushCursor, the cache line holding both cursors is marked "Modified" and invaliadted on Core 1 (Consumer).

- Core 1 is forced to fetch it from L3 or main memory on next read

By using alignas(kCacheLineSize) which we set to 64, on both our push and pop cursor
```c
alignas(kCacheLineSize) std::atomic<std::size_t> pushCursor_{};
alignas(kCacheLineSize) std::atomic<std::size_t> popCursor_{};
```
- it provides us with the offset between 2 objects to avoid false sharing on most modern CPU with cache line of 64 bytes.

# Why cache push / pop cursor?














# Benchmark

| Measurement                                                    | Why                                         |
| -------------------------------------------------------------- | ------------------------------------------- |
| Parse-to-book p50/p99/p99.9, single-threaded vs SPSC-pipelined | Does ring actually help?                    |
| Ring handoff cost (ns/msg)                                     | overhead paid                               |
| Throughput (msg/sec) both modes                                | does pipelining improve throughput          |
| Ring occupancy distribution                                    | is the producer or consumer the bottleneck? |
| Cycles/msg producer, cycles/msg consumer                       | where the time goes                         |
| Book-state hashes, both modes                                  | correctness                                 |

Important things to consider

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
```c
target_compile_options(benchmarks PRIVATE
  $<$<CONFIG:Release>:-O3 -march=native -mtune=native -flto>
)

target_link_options(benchmarks PRIVATE
  $<$<CONFIG:Release>: -flto>
)
```
- `-O3` anables the most aggressive optimization level
- `-march=native` generates the code for the specific CPU we are compiling on, utilizing all avilable instruction set
- `mtune=native` tunes the code generated for your specific CPU model without changing instruction set


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
