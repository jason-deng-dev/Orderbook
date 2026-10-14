#include <benchmark/benchmark.h>
#include <iostream>
#include <spsc_fifo.h>
#include <thread>

constexpr int N = 65536;

static void BM_fifo_push(benchmark::State &state) {

  // - push number of values onto the FIFO on one thread
  // - read that same number of values off the FIFO on another thread
  // - verify value pushed in == value popped (if differ => exception)
  // - get ops/s

  SpscFifo<int, N> fifo;

  for (auto _ : state) {
    for (int i = 0; i < N; ++i) {
      if (!fifo.push(i)) {
        std::cerr << "push failed at " << i;
        return;
      }
    }

    state.PauseTiming();
    int a = 0;
    for (int i = 0; i < N; ++i) {
      if (!fifo.pop(a)) {
        std::cerr << "pop failed at " << i;
        return;
      }
    }
    state.ResumeTiming();

    benchmark::DoNotOptimize(fifo);
  }
}

BENCHMARK(BM_fifo_push);

static void BM_fifo_pop(benchmark::State &state) {

  // - push number of values onto the FIFO on one thread
  // - read that same number of values off the FIFO on another thread
  // - verify value pushed in == value popped (if differ => exception)
  // - get ops/s

  SpscFifo<int, N> fifo;
  int N = N;

  for (auto _ : state) {
    state.PauseTiming();
    for (int i = 0; i < N; ++i) {
      fifo.push(i);
    }
    state.ResumeTiming();

    int a = 0;
    for (int i = 0; i < N; ++i) {
      if (!fifo.pop(a)) {
        std::cerr << "pop failed at " << i;
        return;
      }
    }

    benchmark::DoNotOptimize(fifo);
  }
}

BENCHMARK(BM_fifo_pop);
