#include "slab_allocator.h"
#include "types.h"
#include <gtest/gtest.h>

TEST(PriceLevel_Intrusive, init) {
  PriceLevel_Intrusive pl{};
  EXPECT_EQ(pl.head, nullptr);
  EXPECT_EQ(pl.tail, nullptr);
  EXPECT_EQ(pl.order_count, 0);
  EXPECT_EQ(pl.totalVolume(), 0);
  EXPECT_TRUE(pl.empty());
}

TEST(PriceLevel_Intrusive, addOrder) {
  SlabAllocator<Order_Intrusive, 3> allocator;
  PriceLevel_Intrusive pl{};
  auto pos1 = allocator.allocate();
  pl.addOrder(pos1, 1, 1, Side::Bid, 1, 1);
  EXPECT_EQ(pos1, pl.head);
  EXPECT_EQ(pos1, pl.tail);
  EXPECT_EQ(pl.totalVolume(), 1);
  EXPECT_EQ(pl.order_count, 1);
  EXPECT_FALSE(pl.empty());

  auto pos2 = allocator.allocate();
  pl.addOrder(pos2, 1, 1, Side::Bid, 1, 1);
  EXPECT_EQ(pos1, pl.head);
  EXPECT_EQ(pos2, pl.tail);
  EXPECT_EQ(pl.totalVolume(), 2);
  EXPECT_EQ(pl.order_count, 2);

  auto pos3 = allocator.allocate();
  pl.addOrder(pos3, 1, 1, Side::Bid, 1, 1);
  EXPECT_EQ(pos1, pl.head);
  EXPECT_EQ(pos3, pl.tail);
  EXPECT_EQ(pl.totalVolume(), 3);
  EXPECT_EQ(pl.order_count, 3);
}

TEST(PriceLevel_Intrusive, deleteOrder) {
  SlabAllocator<Order_Intrusive, 3> allocator;
  PriceLevel_Intrusive pl{};
  auto pos1 = allocator.allocate();
  pl.addOrder(pos1, 1, 1, Side::Bid, 1, 1);
  auto pos2 = allocator.allocate();
  pl.addOrder(pos2, 1, 1, Side::Bid, 1, 1);
  auto pos3 = allocator.allocate();
  pl.addOrder(pos3, 1, 1, Side::Bid, 1, 1);

  EXPECT_EQ(pos1, pl.head);
  EXPECT_EQ(pos3, pl.tail);
  EXPECT_EQ(pl.totalVolume(), 3);
  EXPECT_EQ(pl.order_count, 3);
  EXPECT_EQ(allocator.allocatedCount(), 3);

  pl.deleteOrder(pos1, allocator);
  EXPECT_EQ(pl.totalVolume(), 2);
  EXPECT_EQ(pl.order_count, 2);
  EXPECT_EQ(pl.head, pos2);
  EXPECT_EQ(pl.tail, pos3);
  EXPECT_EQ(pl.order_count, 2);
  EXPECT_EQ(allocator.allocatedCount(), 2);
}

