#include "spsc_fifo.h"
#include "spsc_fifo_cached.h"
#include "types.h"
#include <benchmark/benchmark.h>
#include <pthread.h>
#include <sched.h>

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

SpscFifo_Cached<QueueItem> fifo_cached{};
QueueItem out_cached;

static void BM_Fifo_Cached_Throughput(benchmark::State &state) {
  for (auto _ : state) {
    if (state.thread_index() == 0) {
      while (!fifo_cached.push(payload))
        ;
    } else if (state.thread_index() == 1) {

      while (!fifo_cached.pop(out_cached))
        ;
      benchmark::DoNotOptimize(out_cached);
    }
  }
  state.SetItemsProcessed(state.iterations() / 2);
}

BENCHMARK(BM_Fifo_Throughput)->Threads(2)->Iterations(100'000'000)->UseRealTime();
BENCHMARK(BM_Fifo_Cached_Throughput)->Threads(2)->Iterations(100'000'000)->UseRealTime();
