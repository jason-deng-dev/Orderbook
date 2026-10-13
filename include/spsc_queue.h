

#include <cstddef>
#include <cstdint>
class SpscQueue {
  static constexpr size_t CAPACITY = 65536;

  uint64_t t_arrival_ns[CAPACITY]; // stamped in feed handler, read after book update in both paths
};
