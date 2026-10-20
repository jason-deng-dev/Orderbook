#pragma once
#include <cstddef>
#include <stdexcept>

inline constexpr size_t kCacheLineSize = 64;
inline constexpr bool isPowerOfTwo(size_t n) { return n != 0 && (n & (n - 1)) == 0; }
inline size_t checkedCapacity(size_t cap) {
  if (!isPowerOfTwo(cap)) {
    throw std::invalid_argument("capacity must be power of 2");
  }
  return cap;
}
