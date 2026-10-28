#include "bm_helper.h"
#include "spsc_fifo.h"
#include "spsc_fifo_cached.h"
#include "types.h"
#include <atomic>
#include <benchmark/benchmark.h>
#include <cstdint>
#include <thread>

constexpr int kProducerCpu = 2;
constexpr int kConsumerCpu = 4;
template <template <typename> class Fifo>
void BM_Fifo(benchmark::State &state) {
  constexpr size_t fifoSize = 131072;
  Fifo<QueueItem> fifo(fifoSize);

  std::atomic<bool> keepRunning{true};
  std::int64_t consumed = 0;  

  auto consumer = std::jthread([&] {
    pinThread(kConsumerCpu);
    QueueItem val;
    for (;;) {
      while (fifo.pop(val)) {
        benchmark::DoNotOptimize(val);
        ++consumed;
      }
      if (not keepRunning.load(std::memory_order_relaxed)) {
        break;
      }
    }
  });

  pinThread(kProducerCpu);

  std::int64_t produced = 0;
  QueueItem value{};
  for (auto _ : state) {
    while (not fifo.push(value)) {
      ;
    }
    ++produced;
  }

  state.PauseTiming();
  keepRunning.store(false, std::memory_order_relaxed);
  consumer.join();

  if (produced != consumed) {
    state.SkipWithError("data loss: produced != consumed");
    return;
  }

  state.counters["ops/sec"] = benchmark::Counter(double(produced), benchmark::Counter::kIsRate);
}

BENCHMARK_TEMPLATE(BM_Fifo, SpscFifo)->UseRealTime();
BENCHMARK_TEMPLATE(BM_Fifo, SpscFifo_Cached)->UseRealTime();
