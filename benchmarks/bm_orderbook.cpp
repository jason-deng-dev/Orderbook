#include "feed_handler.h"
#include "orderbook_map.h"
#include "types.h"
#include <benchmark/benchmark.h>
#include <unistd.h>
#include <utility>
#include "orderbook_vector.h"

static std::vector<QueueItem> benchmark_messages;
static Feed_Handler fh;

static void SetupBenchmarkData(int n) {
  if (!benchmark_messages.empty()) return;
  benchmark_messages.reserve(n);
  fh.Load(FIXTURES_DIR "/07302019.NASDAQ_ITCH50");
  while (benchmark_messages.size() < n) {
    auto msg = fh.GetNextMessage();
    if (msg) {
      benchmark_messages.push_back(std::move(msg.value()));
    }
    else if (msg.error() == MessageStatus::End) {
      return;
    }
  }
}

static void BM_Orderbook_Map_Throughput(benchmark::State &state) {
  SetupBenchmarkData(state.range(0));
  Orderbook_Map ob;
  int index = 0;
  for (auto _ : state) {
    auto res = ob.ProcessMessage(benchmark_messages[index]);

    benchmark::DoNotOptimize(res);
    // state.PauseTiming();
    index = (index + 1) % benchmark_messages.size();
    // state.ResumeTiming();
  }
  state.SetItemsProcessed(state.iterations());
}


static void BM_Orderbook_Vector_LinearSearch_Throughput(benchmark::State &state) {
  SetupBenchmarkData(state.range(0));
  Orderbook_Vector<LinearSearch> ob;
  int index = 0;
  for (auto _ : state) {
    auto res = ob.ProcessMessage(benchmark_messages[index]);
    benchmark::DoNotOptimize(res);
    // state.PauseTiming();
    index = (index + 1) % benchmark_messages.size();
    // state.ResumeTiming();
  }
  state.SetItemsProcessed(state.iterations());
}

static void BM_Orderbook_Vector_BinarySearch_Throughput(benchmark::State &state) {
  SetupBenchmarkData(state.range(0));
  Orderbook_Vector<BinarySearch> ob;
  int index = 0;
  for (auto _ : state) {
    auto res = ob.ProcessMessage(benchmark_messages[index]);

    benchmark::DoNotOptimize(res);
    // state.PauseTiming();
    index = (index + 1) % benchmark_messages.size();
    // state.ResumeTiming();
  }
  state.SetItemsProcessed(state.iterations());
}

static void BM_Orderbook_Vector_BranchlessBinarySearch_Throughput(benchmark::State &state) {
  SetupBenchmarkData(state.range(0));
  Orderbook_Vector<BranchlessBinarySearch> ob;
  int index = 0;
  for (auto _ : state) {
    auto res = ob.ProcessMessage(benchmark_messages[index]);

    benchmark::DoNotOptimize(res);
    // state.PauseTiming();
    index = (index + 1) % benchmark_messages.size();
    // state.ResumeTiming();
  }
  state.SetItemsProcessed(state.iterations());
}


inline constexpr int N = 1000000000;
BENCHMARK(BM_Orderbook_Map_Throughput)->Arg(N);
BENCHMARK(BM_Orderbook_Vector_LinearSearch_Throughput)->Arg(N);
BENCHMARK(BM_Orderbook_Vector_BinarySearch_Throughput)->Arg(N);
BENCHMARK(BM_Orderbook_Vector_BranchlessBinarySearch_Throughput)->Arg(N);
