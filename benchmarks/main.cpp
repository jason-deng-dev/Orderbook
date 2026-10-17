#include <benchmark/benchmark.h>
#include <iostream>
#include <string>
#include <unistd.h>



BENCHMARK_MAIN();

static double p50(const std::vector<double> &v) {
  std::vector<double> data = v;
  std::sort(data.begin(), data.end());
  if (data.empty()) return 0.0;
  return data[static_cast<size_t>(0.5 * (data.size() - 1))];
}

static double p99(const std::vector<double> &v) {
  std::vector<double> data = v;
  std::sort(data.begin(), data.end());
  if (data.empty()) return 0.0;
  return data[static_cast<size_t>(0.99 * (data.size() - 1))];
}

static double p99_9(const std::vector<double> &v) {
  std::vector<double> data = v;
  std::sort(data.begin(), data.end());
  if (data.empty()) return 0.0;
  return data[static_cast<size_t>(0.999 * (data.size() - 1))];
}

