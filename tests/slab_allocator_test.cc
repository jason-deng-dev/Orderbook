#include "slab_allocator.h"
#include "types.h"
#include <gtest/gtest.h>

TEST(Slab_Allocator, Initalization) {
  SlabAllocator<Order_Intrusive, 10> allocator;
  EXPECT_EQ(allocator.allocatedCount(), 0);
}

TEST(Slab_Allocator, Allocate) {
  SlabAllocator<Order_Intrusive, 5> allocator;
  EXPECT_EQ(allocator.allocatedCount(), 0);
  EXPECT_TRUE(allocator.allocate() != nullptr);
  EXPECT_TRUE(allocator.allocate() != nullptr);
  EXPECT_TRUE(allocator.allocate() != nullptr);
  EXPECT_EQ(allocator.allocatedCount(), 3);
  EXPECT_TRUE(allocator.allocate() != nullptr);
  EXPECT_TRUE(allocator.allocate() != nullptr);
  EXPECT_EQ(allocator.allocatedCount(), 5);
  EXPECT_EQ(allocator.allocate(), nullptr);
  EXPECT_EQ(allocator.allocatedCount(), 5);
  EXPECT_EQ(allocator.allocate(), nullptr);
  EXPECT_EQ(allocator.allocate(), nullptr);
  EXPECT_EQ(allocator.allocatedCount(), 5);
}

TEST(Slab_Allocator, Deallocate) {
  SlabAllocator<Order_Intrusive, 3> allocator;
  EXPECT_EQ(allocator.allocatedCount(), 0);
  auto p1 = allocator.allocate();
  EXPECT_TRUE(p1 != nullptr);
  auto p2 = allocator.allocate();
  EXPECT_TRUE(p2 != nullptr);
  auto p3 = allocator.allocate();
  EXPECT_TRUE(p3 != nullptr);

  EXPECT_EQ(allocator.allocatedCount(), 3);

  EXPECT_TRUE(allocator.deallocate(p1));
  EXPECT_EQ(allocator.allocatedCount(), 2);

  EXPECT_TRUE(allocator.deallocate(p2));
  EXPECT_EQ(allocator.allocatedCount(), 1);

  EXPECT_TRUE(allocator.deallocate(p3));
  EXPECT_EQ(allocator.allocatedCount(), 0);

  Order_Intrusive notInsidePool;
  EXPECT_FALSE(allocator.deallocate(&notInsidePool));
}
