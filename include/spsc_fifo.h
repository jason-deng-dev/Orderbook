#pragma once
#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory>
#include <stdexcept>

inline constexpr bool isPowerOfTwo(size_t n) { return n != 0 && (n & (n - 1)) == 0; }

template <typename T, typename Alloc = std::allocator<T>>
class SpscFifo : private Alloc {
  using Traits = std::allocator_traits<Alloc>;
  size_t capacity_;
  T *ring_;

  std::atomic<std::size_t> pushCursor_{};
  std::atomic<std::size_t> popCursor_{};

public:
  explicit SpscFifo(size_t cap, Alloc const &alloc = Alloc{})
      : Alloc(alloc), capacity_(cap), ring_(Traits::allocate(*this, capacity_)) {
    if (!isPowerOfTwo(cap)) throw std::invalid_argument("capacity must be a power of 2!");
  }
  ~SpscFifo() {
    while (not empty()) {
      ring_[popCursor_ & (capacity_ - 1)].~T();
      ++popCursor_;
    }
    Traits::deallocate(*this, ring_, capacity_);
  };

  auto capacity() const { return capacity_; }
  auto size() const { return pushCursor_ - popCursor_; }
  auto empty() const { return size() == 0; }
  auto full() const { return size() == capacity(); }
  auto pushIndex() const { return pushCursor_ & (capacity_ - 1); }
  auto popIndex() const { return popCursor_ & (capacity_ - 1); }

  auto push(T const &value) {
    if (full()) return false;
    new (&ring_[pushIndex()]) T(value);
    ++pushCursor_;
    return true;
  }

  auto pop(T &value) {
    if (empty()) return false;
    value = ring_[popIndex()];
    ring_[popIndex()].~T();
    ++popCursor_;
    return true;
  }
};
