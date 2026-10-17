#pragma once
#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory>
#include <stdexcept>

inline constexpr size_t kCacheLineSize = 64;
inline constexpr bool isPowerOfTwo(size_t n) { return n != 0 && (n & (n - 1)) == 0; }

template <typename T, typename Alloc = std::allocator<T>>
class SpscFifo : private Alloc {
  using Traits = std::allocator_traits<Alloc>;
  size_t capacity_;
  T *ring_;

  alignas(kCacheLineSize) std::atomic<std::size_t> pushCursorCached_{};
  alignas(kCacheLineSize) std::atomic<std::size_t> pushCursor_{};
  alignas(kCacheLineSize) std::atomic<std::size_t> popCursorCached_{};
  alignas(kCacheLineSize) std::atomic<std::size_t> popCursor_{};

  char padding_[kCacheLineSize - sizeof(size_t)];
  static_assert(std::atomic<size_t>::is_always_lock_free);

public:
  explicit SpscFifo(size_t cap = 65536, Alloc const &alloc = Alloc{})
      : Alloc(alloc), capacity_(cap), ring_(Traits::allocate(*this, capacity_)) {
    if (!isPowerOfTwo(cap)) throw std::invalid_argument("capacity must be a power of 2!");
  }

  ~SpscFifo() {
    auto pushCursor = pushCursor_.load(std::memory_order::relaxed);
    auto popCursor = popCursor_.load(std::memory_order::relaxed);
    while (pushCursor - popCursor > 0) {
      ring_[popCursor & (capacity_ - 1)].~T();
      ++popCursor;
    }
    Traits::deallocate(*this, ring_, capacity_);
  };

  size_t capacity() const noexcept { return capacity_; }

  size_t size() const noexcept {
    auto pushCursor = pushCursor_.load(std::memory_order_relaxed);
    auto popCursor = popCursor_.load(std::memory_order_relaxed);
    return pushCursor - popCursor;
  }
  bool full() const noexcept { return size() == capacity_; }
  bool empty() const noexcept { return size() == 0; }

  bool push(T const &value) {
    auto pushCursor = pushCursor_.load(std::memory_order::relaxed);
    auto popCursor = popCursor_.load(std::memory_order::acquire);
    if (full(pushCursor, popCursor)) {
      return false;
    }
    new (&ring_[pushCursor & (capacity_ - 1)]) T(value);
    pushCursor_.store(pushCursor + 1, std::memory_order_release);
    return true;
  }

  bool pop(T &value) {
    auto pushCursor = pushCursor_.load(std::memory_order::acquire);
    auto popCursor = popCursor_.load(std::memory_order::relaxed);
    if (empty(pushCursor, popCursor)) return false;
    value = ring_[popCursor & (capacity_ - 1)];
    ring_[popCursor & (capacity_ - 1)].~T();
    popCursor_.store(popCursor + 1, std::memory_order_release);
    return true;
  }

private:
  bool empty(std::size_t pushCursor, std::size_t popCursor) const noexcept { return pushCursor - popCursor == 0; }

  bool full(std::size_t pushCursor, std::size_t popCursor) const noexcept {
    return (pushCursor - popCursor) == capacity();
  }
};
