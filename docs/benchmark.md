# Benchmark setup

## Build and run

```bash
cmake --preset default -DCMAKE_BUILD_TYPE=Release
cmake --build build
taskset -c 2,4 ./build/benchmarks --benchmark_repetitions=25 --benchmark_report_aggregates_only=true
```

Every recorded run uses 25 repetitions and reports aggregates: mean, median, standard deviation, and coefficient of variation. When CV is high I compare medians instead of means.

## CPU pinning

```bash
lscpu -e
```

```text
CPU NODE SOCKET CORE L1d:L1i:L2:L3 ONLINE
  0    0      0    0 0:0:0:0          yes
  1    0      0    0 0:0:0:0          yes
  2    0      0    1 1:1:1:0          yes
  3    0      0    1 1:1:1:0          yes
  4    0      0    2 2:2:2:0          yes
  5    0      0    2 2:2:2:0          yes
  ...
```

Logical CPUs 0/1 are SMT siblings on physical core 0, 2/3 on core 1, 4/5 on core 2, and so on. `taskset -c 2,4` puts the two benchmark threads on distinct physical cores. This matters for the SPSC benchmark: producer and consumer then exercise inter-core communication instead of competing for the same core's execution units.

## perf stat

```bash
taskset -c 2,4 perf stat -e 'cycles,instructions,cache-misses,cache-references,branch-misses,branches,task-clock' \
  ./build/benchmarks \
  --benchmark_filter=BM_Fifo_Throughput \
  --benchmark_repetitions=10 \
  --benchmark_report_aggregates_only=true
```

perf stat counts the whole process, not just the benchmark loop. Both fixture passes run during static initialization, before `main`:

- `LoadBenchmarkData` in `bm_orderbook.cpp` loads the NASDAQ capture and collects the orderbook messages.
- `CountFeedCalls` in `bm_feedHandler.cpp` loads the capture again and counts `GetNextMessage` calls until `End`.

That static work costs roughly 35G instructions per invocation. An orderbook benchmark run totals about 36G instructions, so almost all of its counter totals describe fixture parsing rather than orderbook work. The feed handler and FIFO benchmarks contain the same static work, but their timed loops are large enough that it is a smaller fraction (about 20% and 44%). I do not derive per-message cache or branch numbers from these counters. Doing that accurately would need an empty-filter baseline run to subtract, or counters scoped to the benchmark loop.

The 8.66 GB fixture also means a cold first run reads from disk. The logged runs show low system time, which is consistent with a warm page cache; keep that in mind when comparing against a fresh boot.

## Codegen parity

The orderbook benchmark compares an implementation that lives in a `.cpp` file with one that is header-only:

- `Orderbook_Map` keeps `AddOrder`, `DeleteOrder`, and `ModifyOrder` in `src/orderbook_map.cpp`, compiled into `orderbook_core`.
- `Orderbook_Vector` is a header-only template, so its code compiles directly into the benchmark translation unit and inlines into the benchmark loop.

If only the benchmark target used `-march=native`, the vector would get native instructions and cross-function inlining while the map would not. Any difference would then mix data-structure cost with build settings. To remove that confound:

```cmake
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported LANGUAGES CXX)
if(ipo_supported)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

target_compile_options(orderbook_core PRIVATE
  $<$<CONFIG:Release>:-march=native -mtune=native>
)
```

LTO is enabled for every Release target, so calls from the benchmark into `orderbook_core` can inline across translation units. `orderbook_core` and `benchmarks` both use `-march=native -mtune=native`. The benchmark target adds `-O3` as well, matching the Release flags.

`-march=native` makes the binary machine-specific. Benchmark numbers are only valid on the machine that built them, so the CPU model belongs in every experiment log.

## Preventing dead-code elimination

Every benchmark feeds its result into `benchmark::DoNotOptimize(...)`. This tells the compiler the value might be observed externally, so it cannot delete the work that produced it. `ClobberMemory` is not used; none of these benchmarks mutate memory in a way the compiler could elide.

# Benchmarks

## SPSC FIFO

```cpp
QueueItem payload;
SpscFifo<QueueItem> fifo{};
QueueItem out;

static void BM_Fifo_Throughput(benchmark::State &state) {
  for (auto _ : state) {
    if (state.thread_index() == 0) {
      while (!fifo.push(payload))
        ;
    } else if (state.thread_index() == 1) {
      while (!fifo.pop(out))
        ;
      benchmark::DoNotOptimize(out);
    }
  }
  state.SetItemsProcessed(state.iterations() / 2);
}

BENCHMARK(BM_Fifo_Throughput)->Threads(2)->Iterations(100'000'000)->UseRealTime();
```

Thread 0 is the producer and thread 1 is the consumer. Each spins until its push or pop succeeds, so the loop measures the queue's steady-state handoff rather than a single operation. `UseRealTime` makes the two threads share one wall-clock measurement.

`SetItemsProcessed(state.iterations() / 2)` accounts for Google Benchmark summing the work reported by each thread. Each thread reports `iterations / 2` items; across the two threads that is one item per iteration, matching the push/pop pair. The benchmark does no correctness checking. Correctness is covered by the FIFO tests, so this measures throughput assuming a correct queue.

## Feed handler

```cpp
static const int64_t kFeedCalls = static_cast<int64_t>(CountFeedCalls());

static void BM_FeedHandler_Throughput(benchmark::State &state) {
  Feed_Handler fh;
  fh.Load(FIXTURES_DIR "/07302019.NASDAQ_ITCH50");

  for (auto _ : state) {
    auto queueItem = fh.GetNextMessage();
    benchmark::DoNotOptimize(queueItem);
  }

  state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_FeedHandler_Throughput)->Iterations(kFeedCalls);
```

`CountFeedCalls` runs during static initialization, loads the capture, and calls `GetNextMessage` until `End`. The result, 282,229,685, becomes the fixed iteration count, so the benchmark never reads past the end of the buffer.

Each repetition constructs a `Feed_Handler` and calls `Load` inside the benchmark function. Google Benchmark times the whole function body, so the `mmap` and `madvise` cost is included in the reported per-iteration time. It is amortized over 282M iterations, but it is not zero.

The handler uses the default `filter_by_stock = false`, so this benchmark parses every message in the capture. `GetNextMessage` returns a `QueueItem` for add, cancel, delete, and replace messages, `MessageStatus::Skipped` for administrative messages and unsupported types, and `MessageStatus::End` at the end of the buffer. The result is parse and dispatch throughput weighted by the real NASDAQ message mix.

## Orderbook

```cpp
static std::vector<QueueItem> benchmark_messages;
static Feed_Handler fh(true);

static const size_t kMessageCount = LoadBenchmarkData(1'000'000);

template <typename Book>
static void RunOnePass(benchmark::State &state, Book &book) {
  size_t index = 0;
  for (auto _ : state) {
    auto res = book.ProcessMessage(benchmark_messages[index]);
    benchmark::DoNotOptimize(res);
    ++index;
  }
  state.SetItemsProcessed(state.iterations());
}
```

`LoadBenchmarkData` runs during static initialization. It uses a stock-filtered feed handler, which latches onto the stock locate of the first add message and emits only messages for that stock. The requested cap is 1,000,000 messages, but the capture ends first: 121,556 orderbook messages are collected. That is `kMessageCount`, and it becomes the fixed iteration count.

Each benchmark function constructs its own book and replays the stored messages once, in capture order. With 25 repetitions, each repetition starts a fresh book and runs a full pass from index 0. The array is not wrapped around or cycled within a repetition. This isolates `ProcessMessage` from parsing, FIFO handoff, and I/O, and keeps the message distribution realistic.

The four variants share `RunOnePass` and differ only in the book type: `Orderbook_Map`, and `Orderbook_Vector` with linear, binary, or branchless binary search. The map allocates through `std::map`; the vector variants keep price levels in a sorted vector.

Fixture loading happens before `main`, so it is outside the Google Benchmark timing. It is inside the process-wide perf counters, which is why those counters are not used for orderbook per-message claims.

## End-to-end

Not implemented yet. The intended setup is one producer thread running `Feed_Handler` and pushing into the SPSC FIFO, and one consumer thread popping and calling `ProcessMessage`, with the same pinning and repetition policy as the isolated benchmarks. That run will show the cost the isolated benchmarks leave out: queue handoff and cross-thread cache traffic.
