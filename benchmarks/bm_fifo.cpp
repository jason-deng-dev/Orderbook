#include "spsc_fifo.h"
#include "spsc_fifo_cached.h"
#include "types.h"
#include <benchmark/benchmark.h>
#include <cstddef>
#include <cstdint>
#include <pthread.h>
#include <sched.h>

SpscFifo<QueueItem> fifo{};
  QueueItem payload{};

static void BM_Fifo_Throughput(benchmark::State &state) {
  QueueItem out{};

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

SpscFifo_Cached<QueueItem> fifo_cached{};

static void BM_Fifo_Cached_Throughput(benchmark::State &state) {
  QueueItem out{};
  for (auto _ : state) {
    if (state.thread_index() == 0) {
      while (!fifo_cached.push(payload))
        ;
    } else if (state.thread_index() == 1) {

      while (!fifo_cached.pop(out))
        ;
      benchmark::DoNotOptimize(out);
    }
  }
  state.SetItemsProcessed(state.iterations() / 2);
}

constexpr auto slackFifoCapacity = 4'194'304;
SpscFifo_Cached<QueueItem> fifo_cached_slack{slackFifoCapacity};
SpscFifo<QueueItem> fifo_slack{slackFifoCapacity};

constexpr auto slack_target = slackFifoCapacity / 2;

template <typename FIFO>
static size_t SetupSlackedQueue(FIFO &fifo, size_t slackTarget) {
  while (fifo.size() < slackTarget) {
    fifo.push(payload);
  }
  return fifo.size();
}

static const size_t cachedSlackSize = SetupSlackedQueue(fifo_cached_slack, slackFifoCapacity / 2);
static const size_t baseSlackSize = SetupSlackedQueue(fifo_slack, slackFifoCapacity / 2);

static void BM_Fifo_Cached_Throughput_Slacked(benchmark::State &state) {
  QueueItem out{};
  for (auto _ : state) {
    if (state.thread_index() == 0) {
      while (!fifo_cached_slack.push(payload))
        ;
    } else if (state.thread_index() == 1) {

      while (!fifo_cached_slack.pop(out))
        ;
      benchmark::DoNotOptimize(out);
    }
  }
  state.SetItemsProcessed(state.iterations() / 2);
}

static void BM_Fifo_Throughput_Slacked(benchmark::State &state) {
  QueueItem out{};
  for (auto _ : state) {
    if (state.thread_index() == 0) {
      while (!fifo_slack.push(payload))
        ;
    } else if (state.thread_index() == 1) {

      while (!fifo_slack.pop(out))
        ;
      benchmark::DoNotOptimize(out);
    }
  }
  state.SetItemsProcessed(state.iterations() / 2);
}


BENCHMARK(BM_Fifo_Throughput)->Threads(2)->Iterations(100'000'000)->UseRealTime();
BENCHMARK(BM_Fifo_Cached_Throughput)->Threads(2)->Iterations(100'000'000)->UseRealTime();
BENCHMARK(BM_Fifo_Throughput_Slacked)->Threads(2)->Iterations(static_cast<int64_t>(baseSlackSize))->UseRealTime();
BENCHMARK(BM_Fifo_Cached_Throughput_Slacked)->Threads(2)->Iterations(static_cast<int64_t>(cachedSlackSize))->UseRealTime();
