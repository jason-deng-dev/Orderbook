#include <algorithm>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <pthread.h>
#include <sched.h>


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

// Helper function to pin the current thread to a specific CPU
static void pinThread(int cpu) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu, &cpuset);
  if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
    std::cerr << "Error: Failed to pin thread to CPU " << cpu << "\n";
    std::exit(EXIT_FAILURE);
  }
}
