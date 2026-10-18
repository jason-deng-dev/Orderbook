#pragma once
#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory>
#include <stdexcept>

inline constexpr size_t kCacheLineSize_Cached = 64;
inline constexpr bool isPowerOfTwo_Cached(size_t n) { return n != 0 && (n & (n - 1)) == 0; }


template <typename T, typename Alloc = std::allocator<T>>
class SpscFifo_Cached : private Alloc {
  using Traits = std::allocator_traits<Alloc>;
  size_t capacity_;
  T *ring_;

  // --- Load and stored by Producer; loaded by Consumer ---
  alignas(kCacheLineSize_Cached) std::atomic<std::size_t> pushCursor_{};

  // --- Producer's exclusive cache line ---
  alignas(kCacheLineSize_Cached) std::size_t cachedPopCursor_{};

  // --- Load and stored by Consumer; loaded by Producer ---
  alignas(kCacheLineSize_Cached) std::atomic<std::size_t> popCursor_{};

  // --- Consumer's exclusive cache line ---
  alignas(kCacheLineSize_Cached) std::size_t cachedPushCursor_{};

  char padding_[kCacheLineSize_Cached - sizeof(size_t)];
  static_assert(std::atomic<size_t>::is_always_lock_free);

public:
  explicit SpscFifo_Cached(size_t cap = 65536, Alloc const &alloc = Alloc{})
      : Alloc(alloc), capacity_(cap), ring_(Traits::allocate(*this, capacity_)) {
    if (!isPowerOfTwo_Cached(cap)) throw std::invalid_argument("capacity must be a power of 2!");
  }

  ~SpscFifo_Cached() {
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
    if (popCursor <= pushCursor) {
      return pushCursor - popCursor;
    } else {
      return capacity_ - (popCursor - (pushCursor + 1));
    }
  }
  auto full() const noexcept {
    auto pushCursor = pushCursor_.load(std::memory_order_relaxed);
    auto popCursor = popCursor_.load(std::memory_order_relaxed);
    return full(pushCursor, popCursor);
  }

  auto empty() const noexcept {
    auto pushCursor = pushCursor_.load(std::memory_order_relaxed);
    auto popCursor = popCursor_.load(std::memory_order_relaxed);
    return empty(pushCursor, popCursor);
  }

  bool push(T const &value) {
    auto pushCursor = pushCursor_.load(std::memory_order::relaxed);
    if (full(pushCursor, cachedPopCursor_)) {
      cachedPopCursor_ = popCursor_.load(std::memory_order::acquire);
      if (full(pushCursor, cachedPopCursor_)) return false;
    }
    new (&ring_[pushCursor & (capacity_ - 1)]) T(value);
    if (pushCursor == capacity_) {
      pushCursor_.store(0, std::memory_order_release);
    } else {
      pushCursor_.store(pushCursor + 1, std::memory_order_release);
    }
    return true;
  }

  bool pop(T &value) {
    auto popCursor = popCursor_.load(std::memory_order::relaxed);
    if (empty(cachedPushCursor_, popCursor)) {
      cachedPushCursor_ = pushCursor_.load(std::memory_order::acquire);
      if (empty(cachedPushCursor_, popCursor)) return false;
    }
    value = ring_[popCursor & (capacity_ - 1)];
    ring_[popCursor & (capacity_ - 1)].~T();

    if (popCursor == capacity_) {
      popCursor_.store(0, std::memory_order_release);
    } else {
      popCursor_.store(popCursor + 1, std::memory_order_release);
    }
    return true;
  }

private:
  bool empty(std::size_t pushCursor, std::size_t popCursor) const noexcept { return pushCursor == popCursor; }

  bool full(std::size_t pushCursor, std::size_t popCursor) const noexcept {
    if (pushCursor < popCursor) {
      return pushCursor == popCursor - 1;
    } else if (popCursor < pushCursor) {
      return popCursor == pushCursor - capacity_;
    } else {
      return false;
    }
  }
};
