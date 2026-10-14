

#include <atomic>
#include <cstddef>
#include <memory>

template <typename T, typename Alloc = std::allocator<T>>
class SpscFifo : private Alloc {
  using Traits = std::allocator_traits<Alloc>;

  static constexpr size_t capacity_ = 65536;
  T *ring_ = Traits::allocate(*this, capacity_);

  std::atomic<std::size_t> pushCursor_{};
  std::atomic<std::size_t> popCursor_{};

public:
  explicit SpscFifo(Alloc const &alloc = Alloc{}) : Alloc { alloc } {};
  ~SpscFifo();

  auto capacity() const { return capacity_; }
  auto size() const { return pushCursor_ - popCursor_; }
  auto empty() const { return size() == 0; }
  auto full() const { return size() == capacity(); }

  auto push(T const &value) {
    if (full()) return false;
    new (&ring_[pushCursor_ % capacity_]) T(value);
    ++pushCursor_;
    return true;
  }

  auto pop(T &value) {
    if (empty()) return false;
    value = ring_[popCursor_ % capacity_];
    ring_[popCursor_ % capacity_].~T();
    ++popCursor_;
    return true;
  }
};
