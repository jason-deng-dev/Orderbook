#pragma once
#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <stdexcept>

inline constexpr bool isPowerOfTwo(size_t n) { return n != 0 && (n & (n - 1)) == 0; }

template <typename T, typename Alloc = std::allocator<T>>
class SpscFifo : private Alloc {
  using Traits = std::allocator_traits<Alloc>;
  size_t capacity_;
  T *ring_;

  alignas(std::hardware_destructive_interference_size) std::atomic<std::size_t> pushCursor_{};
  alignas(std::hardware_destructive_interference_size) std::atomic<std::size_t> popCursor_{};
  // padding to avoid false sharing with adjacent objects
  char padding_[std::hardware_destructive_interference_size - sizeof(size_t)];
  static_assert(std::atomic<size_t>::is_always_lock_free);

public:
  explicit SpscFifo(size_t cap = 65536, Alloc const &alloc = Alloc{})
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

  auto empty(std::size_t pushCursor, std::size_t popCursor) const { return pushCursor - popCursor == 0; }
  auto full(std::size_t pushCursor, std::size_t popCursor) const { return (pushCursor - popCursor) == capacity(); }

  auto pushIndex() const { return pushCursor_ & (capacity_ - 1); }
  auto popIndex() const { return popCursor_ & (capacity_ - 1); }

  auto push(T const &value) {
    auto pushCursor = pushCursor_.load(std::memory_order::relaxed);
    auto popCursor = popCursor_.load(std::memory_order::acquire);
    if (full(pushCursor, popCursor)) {
      return false;
    }
    new (&ring_[pushIndex()]) T(value);
    pushCursor_.store(pushCursor+1, std::memory_order_release);
    return true;
  }

  auto pop(T &value) {
    auto pushCursor = pushCursor_.load(std::memory_order::acquire);
    auto popCursor = popCursor_.load(std::memory_order::relaxed);
    if (empty(pushCursor, popCursor)) return false;
    value = ring_[popIndex()];
    ring_[popIndex()].~T();
    popCursor_.store(popCursor + 1, std::memory_order_release);
    return true;
  }
};
