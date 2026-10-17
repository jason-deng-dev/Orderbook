#include "types.h"
#include <benchmark/benchmark.h>
#include <pthread.h>
#include <sched.h>
#include <spsc_fifo.h>





QueueItem payload;
SpscFifo<QueueItem> fifo{};
QueueItem out;

static void BM_Fifo_Throughput(benchmark::State &state) {
  for (auto _ : state) {
    if (state.thread_index() == 0) {
      while (!fifo.push(payload))
        ;
    }
    else if (state.thread_index() == 1) {
      
      while (!fifo.pop(out))
        ;
      benchmark::DoNotOptimize(out);
    }
  }
  state.SetItemsProcessed(state.iterations()/2);
}

BENCHMARK(BM_Fifo_Throughput)->Threads(2)->Iterations(100'000'000)->UseRealTime();
