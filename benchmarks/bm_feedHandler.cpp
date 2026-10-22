#include "feed_handler.h"
#include <benchmark/benchmark.h>
#include <cstdint>
#include "bm_helper.h"

static size_t CountFeedCalls() {
  Feed_Handler counter;
  if (!counter.Load(FIXTURES_DIR "/07302019.NASDAQ_ITCH50")) {
    return 0;
  }

  size_t calls = 0;
  while (true) {
    auto msg = counter.GetNextMessage();
    ++calls;
    if (!msg && msg.error() == MessageStatus::End) {
      break;
    }
  }
  return calls;
}

// Counted before BENCHMARK registration so the fixed iteration count matches
// the capture exactly and never runs past End.
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

BENCHMARK(BM_FeedHandler_Throughput)
    ->Iterations(kFeedCalls);
