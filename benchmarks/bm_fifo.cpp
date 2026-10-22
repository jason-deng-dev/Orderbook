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
  state.SetItemsProcessed(produced);
}

BENCHMARK_TEMPLATE(BM_Fifo, SpscFifo)->UseRealTime();
BENCHMARK_TEMPLATE(BM_Fifo, SpscFifo_Cached)->UseRealTime();

// template <typename Fifo>
// static void RunOnePass(benchmark::State &state, Fifo &fifo) {
//   std::atomic<bool> keep_running{true};
//   std::atomic<int64_t> consumed_count{0};

//   std::jthread consumer([&]() {
//     pinThread(kConsumerCpu);

//     QueueItem out{};

//     while (keep_running || !fifo.empty()) {
//       while (fifo.pop(out)) {
//         benchmark::DoNotOptimize(out);
//         consumed_count.fetch_add(1, std::memory_order_relaxed);
//       }
//     }
//   });

//   // producer runs on main benchmark thread
//   pinThread(kProducerCpu);
//   int64_t produced_count = 0;

//   for (auto _ : state) {
//     QueueItem qItem{1, 1, 1, 1, OrderType::Add, Side::Bid};
//     while (!fifo.push(qItem)) {
//       benchmark::ClobberMemory();
//     }
//     produced_count++;
//   }

//   keep_running = false;
//   consumer.join();
//   if (produced_count != consumed_count.load()) {
//     state.SkipWithError("Data loss: produced count != consumed count");
//     return;
//   }
//   state.SetItemsProcessed(produced_count);
// }

// static void BM_Fifo_Throughput(benchmark::State &state) {
//   SpscFifo<QueueItem> fifo(kQueueSize);
//   RunOnePass(state, fifo);
// }

// static void BM_Fifo_Cached_Throughput(benchmark::State &state) {
//   SpscFifo_Cached<QueueItem> fifo(kQueueSize);
//   RunOnePass(state, fifo);
// }

// BENCHMARK(BM_Fifo_Throughput)->Iterations(100'000'000)->UseRealTime();
// BENCHMARK(BM_Fifo_Cached_Throughput)->Iterations(100'000'000)->UseRealTime();

// SpscFifo<QueueItem> fifo{};
// QueueItem payload{};

// static void BM_Fifo_Throughput_Prev(benchmark::State &state) {
//   pinThread(state.thread_index() == 0 ? kProducerCpu : kConsumerCpu);
//   QueueItem out{};
//   size_t successful_ops = 0;

//   for (auto _ : state) {
//     if (state.thread_index() == 0) {
//       while (!fifo.push(payload))
//         ;
//       ++successful_ops;
//     } else if (state.thread_index() == 1) {

//       while (!fifo.pop(out))
//         ;
//       benchmark::DoNotOptimize(out);
//       ++successful_ops;
//     }
//   }
//   state.SetItemsProcessed(successful_ops);
// }

// SpscFifo_Cached<QueueItem> fifo_cached{};

// static void BM_Fifo_Cached_Throughput_Prev(benchmark::State &state) {
//   pinThread(state.thread_index() == 0 ? kProducerCpu : kConsumerCpu);

//   QueueItem out{};
//   size_t push_ops = 0;
//   size_t pop_ops = 0;

//   for (auto _ : state) {
//     if (state.thread_index() == 0) {
//       while (!fifo_cached.push(payload)) {
//       }
//       ++push_ops;
//     } else {
//       while (!fifo_cached.pop(out)) {
//       }
//       benchmark::DoNotOptimize(out);
//       ++pop_ops;
//     }
//   }

//   if (push_ops != pop_ops) {
//     state.SkipWithError("Data loss: produced count != consumed count");
//     return;
//   }

//   state.SetItemsProcessed(push_ops);
// }

// BENCHMARK(BM_Fifo_Throughput_Prev)->Threads(2)->Iterations(100'000'000)->UseRealTime();
// BENCHMARK(BM_Fifo_Cached_Throughput_Prev)->Threads(2)->Iterations(100'000'000)->UseRealTime();
