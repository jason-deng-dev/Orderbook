#include "bm_helper.h"
#include "feed_handler.h"
#include "orderbook_map.h"
#include "orderbook_slab.h"
#include "orderbook_vector.h"
#include "search_policies.h"
#include "types.h"
#include <benchmark/benchmark.h>
#include <utility>

static std::vector<QueueItem> benchmark_messages;
static Feed_Handler fh(true);

static size_t LoadBenchmarkData(size_t maxMessages) {
  benchmark_messages.reserve(maxMessages);
  if (!fh.Load(FIXTURES_DIR "/07302019.NASDAQ_ITCH50")) {
    return 0;
  }
  while (benchmark_messages.size() < maxMessages) {
    auto msg = fh.GetNextMessage();
    if (msg) {
      benchmark_messages.push_back(std::move(msg.value()));
    } else if (msg.error() == MessageStatus::End) {
      break;
    }
  }
  return benchmark_messages.size();
}

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

static void BM_Orderbook_Map_Throughput(benchmark::State &state) {
  Orderbook_Map ob;
  RunOnePass(state, ob);
}

static void BM_Orderbook_Vector_LinearSearch_Throughput(benchmark::State &state) {
  Orderbook_Vector<LinearSearch> ob;
  RunOnePass(state, ob);
}

static void BM_Orderbook_Vector_BinarySearch_Throughput(benchmark::State &state) {
  Orderbook_Vector<BinarySearch> ob;
  RunOnePass(state, ob);
}

static void BM_Orderbook_Vector_BranchlessBinarySearch_Throughput(benchmark::State &state) {
  Orderbook_Vector<BranchlessBinarySearch> ob;
  RunOnePass(state, ob);
}

constexpr int SlabSize = 100;
static void BM_Orderbook_Slab_LinearSearch_Throughput(benchmark::State &state) {
  Orderbook_Slab<LinearSearch, SlabSize> ob;
  RunOnePass(state, ob);
}

static void BM_Orderbook_Slab_BinarySearch_Throughput(benchmark::State &state) {
  Orderbook_Slab<BinarySearch, SlabSize> ob;
  RunOnePass(state, ob);
}

static void BM_Orderbook_Slab_BranchlessBinarySearch_Throughput(benchmark::State &state) {
  Orderbook_Slab<BranchlessBinarySearch, SlabSize> ob;
  RunOnePass(state, ob);
}


BENCHMARK(BM_Orderbook_Map_Throughput)->Iterations(static_cast<int64_t>(kMessageCount));
BENCHMARK(BM_Orderbook_Vector_LinearSearch_Throughput)->Iterations(static_cast<int64_t>(kMessageCount));
BENCHMARK(BM_Orderbook_Vector_BinarySearch_Throughput)->Iterations(static_cast<int64_t>(kMessageCount));
BENCHMARK(BM_Orderbook_Vector_BranchlessBinarySearch_Throughput)->Iterations(static_cast<int64_t>(kMessageCount));
BENCHMARK(BM_Orderbook_Slab_LinearSearch_Throughput)->Iterations(static_cast<int64_t>(kMessageCount));
BENCHMARK(BM_Orderbook_Slab_BinarySearch_Throughput)->Iterations(static_cast<int64_t>(kMessageCount));
BENCHMARK(BM_Orderbook_Slab_BranchlessBinarySearch_Throughput)->Iterations(static_cast<int64_t>(kMessageCount));
