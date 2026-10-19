#pragma once
#include <cstddef>

// to be used via std::allocator_traits, thus only need to implement allocate/deallocate
template <typename T, int SlabSize>
class SlabAllocator {
  struct FreeNode {
    FreeNode *next;
  };

public:
  static_assert(sizeof(T) >= sizeof(FreeNode));
  static_assert(alignof(T) >=  alignof(FreeNode));
  
  SlabAllocator() { init(); };

  T *allocate() {
    if (free_head_ == nullptr) {
      return nullptr;
    }
    T *result = reinterpret_cast<T *>(free_head_);
    free_head_ = free_head_->next;
    ++allocated_count_;
    return result;
  }

  bool deallocate(T &p) {
    if (p == nullptr) return false;

    if (p < reinterpret_cast<T &>(pool_) || p >= reinterpret_cast<T *>(pool_ + sizeof(pool_))) {
      return false; // pointer not from pool
    }

    FreeNode *node = reinterpret_cast<FreeNode *>(p);
    node->next = free_head_;
    free_head_ = node;
    --allocated_count_;
    return true;
  }

  auto allocatedCount() const { return allocated_count_; }
  auto poolSize() const { return pool_; }

private:
  alignas(T) char pool_[SlabSize * sizeof(T)];

  void init() {
    char *current = pool_;

    // intialize slots in our slab
    for (size_t i = 0; i < SlabSize - 1; ++i) {
      reinterpret_cast<FreeNode *>(current)->next = reinterpret_cast<FreeNode *>(current + sizeof(T));
      current += sizeof(T);
    }

    // last slot points to nullptr
    reinterpret_cast<FreeNode *>(current)->next = nullptr;
    free_head_ = reinterpret_cast<FreeNode *>(pool_);
  }

  FreeNode *free_head_ = nullptr;
  size_t allocated_count_ = 0;
};
