#include "feed_handler.h"
#include <benchmark/benchmark.h>

static void BM_FeedHandler_Throughput(benchmark::State &state) {
  Feed_Handler fh;
  fh.Load(FIXTURES_DIR "/07302019.NASDAQ_ITCH50");
  
  for (auto _ : state) {
    auto queueItem = fh.GetNextMessage();
    benchmark::DoNotOptimize(queueItem);
  }

  state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_FeedHandler_Throughput)->Iterations(50000000);
