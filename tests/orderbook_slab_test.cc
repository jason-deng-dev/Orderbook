#include "slab_allocator.h"
#include "types.h"
#include <cstddef>
#include <gtest/gtest.h>
#include <initializer_list>

namespace {

// Verifies every invariant of the intrusive doubly linked list:
//  - head/tail and order_count agree with the expected node sequence
//  - head->prev == nullptr and tail->next == nullptr
//  - adjacent pairs mirror: A->next == B and B->prev == A
//  - walking next from head visits exactly the expected nodes in order
// Only pass live nodes: deleted nodes are recycled by the slab's free list.
void verifyLevel(PriceLevel_Intrusive &pl,
                 std::initializer_list<Order_Intrusive *> expected) {
  const std::size_t expectedCount = expected.size();
  EXPECT_EQ(pl.order_count, expectedCount);

  if (expectedCount == 0) {
    EXPECT_EQ(pl.head, nullptr);
    EXPECT_EQ(pl.tail, nullptr);
    EXPECT_TRUE(pl.empty());
    return;
  }

  ASSERT_NE(pl.head, nullptr);
  ASSERT_NE(pl.tail, nullptr);
  EXPECT_FALSE(pl.empty());
  EXPECT_EQ(pl.head->prev, nullptr) << "head->prev must be null";
  EXPECT_EQ(pl.tail->next, nullptr) << "tail->next must be null";

  Order_Intrusive *prev = nullptr;
  Order_Intrusive *curr = pl.head;
  std::size_t index = 0;
  for (Order_Intrusive *node : expected) {
    ASSERT_NE(curr, nullptr) << "list ended before expected node index " << index;
    EXPECT_EQ(curr, node) << "unexpected node at index " << index;
    EXPECT_EQ(curr->prev, prev) << "broken prev link at node index " << index;
    prev = curr;
    curr = curr->next;
    ++index;
  }
  EXPECT_EQ(curr, nullptr) << "list longer than expected";
  EXPECT_EQ(prev, pl.tail) << "tail must be the last reachable node";
}

}  // namespace

TEST(PriceLevel_Intrusive, init) {
  PriceLevel_Intrusive pl{};
  EXPECT_EQ(pl.head, nullptr);
  EXPECT_EQ(pl.tail, nullptr);
  EXPECT_EQ(pl.order_count, 0);
  EXPECT_EQ(pl.totalVolume(), 0);
  EXPECT_TRUE(pl.empty());
  verifyLevel(pl, {});
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
  verifyLevel(pl, {pos1});
  EXPECT_EQ(pos1->prev, nullptr);
  EXPECT_EQ(pos1->next, nullptr);

  auto pos2 = allocator.allocate();
  pl.addOrder(pos2, 1, 1, Side::Bid, 1, 1);
  EXPECT_EQ(pos1, pl.head);
  EXPECT_EQ(pos2, pl.tail);
  EXPECT_EQ(pl.totalVolume(), 2);
  EXPECT_EQ(pl.order_count, 2);
  verifyLevel(pl, {pos1, pos2});
  EXPECT_EQ(pos1->next, pos2);
  EXPECT_EQ(pos2->prev, pos1);
  EXPECT_EQ(pos2->next, nullptr);

  auto pos3 = allocator.allocate();
  pl.addOrder(pos3, 1, 1, Side::Bid, 1, 1);
  EXPECT_EQ(pos1, pl.head);
  EXPECT_EQ(pos3, pl.tail);
  EXPECT_EQ(pl.totalVolume(), 3);
  EXPECT_EQ(pl.order_count, 3);
  verifyLevel(pl, {pos1, pos2, pos3});
  EXPECT_EQ(pos2->next, pos3);
  EXPECT_EQ(pos3->prev, pos2);
  EXPECT_EQ(pos3->next, nullptr);
}

TEST(PriceLevel_Intrusive, deleteOrderHead) {
  SlabAllocator<Order_Intrusive, 3> allocator;
  PriceLevel_Intrusive pl{};
  auto pos1 = allocator.allocate();
  pl.addOrder(pos1, 1, 1, Side::Bid, 1, 1);
  auto pos2 = allocator.allocate();
  pl.addOrder(pos2, 1, 1, Side::Bid, 1, 1);
  auto pos3 = allocator.allocate();
  pl.addOrder(pos3, 1, 1, Side::Bid, 1, 1);
  verifyLevel(pl, {pos1, pos2, pos3});
  EXPECT_EQ(allocator.allocatedCount(), 3);

  // Removing the head must null the new head's prev pointer.
  pl.deleteOrder(pos1, allocator);
  EXPECT_EQ(pl.totalVolume(), 2);
  EXPECT_EQ(pl.order_count, 2);
  EXPECT_EQ(allocator.allocatedCount(), 2);
  verifyLevel(pl, {pos2, pos3});
  EXPECT_EQ(pl.head, pos2);
  EXPECT_EQ(pl.head->prev, nullptr);
  EXPECT_EQ(pos2->next, pos3);
  EXPECT_EQ(pos3->prev, pos2);
  EXPECT_EQ(pl.tail, pos3);

  pl.deleteOrder(pos2, allocator);
  verifyLevel(pl, {pos3});
  EXPECT_EQ(pl.head, pos3);
  EXPECT_EQ(pl.tail, pos3);
  EXPECT_EQ(pos3->prev, nullptr);
  EXPECT_EQ(pos3->next, nullptr);

  pl.deleteOrder(pos3, allocator);
  EXPECT_EQ(pl.totalVolume(), 0);
  EXPECT_EQ(pl.order_count, 0);
  EXPECT_EQ(allocator.allocatedCount(), 0);
  verifyLevel(pl, {});
}

TEST(PriceLevel_Intrusive, deleteOrderMiddle) {
  SlabAllocator<Order_Intrusive, 3> allocator;
  PriceLevel_Intrusive pl{};
  auto pos1 = allocator.allocate();
  pl.addOrder(pos1, 1, 1, Side::Bid, 1, 1);
  auto pos2 = allocator.allocate();
  pl.addOrder(pos2, 1, 1, Side::Bid, 1, 1);
  auto pos3 = allocator.allocate();
  pl.addOrder(pos3, 1, 1, Side::Bid, 1, 1);
  verifyLevel(pl, {pos1, pos2, pos3});
  EXPECT_EQ(allocator.allocatedCount(), 3);

  // Removing the middle must stitch its neighbors together.
  pl.deleteOrder(pos2, allocator);
  EXPECT_EQ(pl.totalVolume(), 2);
  EXPECT_EQ(pl.order_count, 2);
  EXPECT_EQ(allocator.allocatedCount(), 2);
  verifyLevel(pl, {pos1, pos3});
  EXPECT_EQ(pl.head, pos1);
  EXPECT_EQ(pl.tail, pos3);
  EXPECT_EQ(pos1->next, pos3);
  EXPECT_EQ(pos3->prev, pos1);

  pl.deleteOrder(pos1, allocator);
  verifyLevel(pl, {pos3});
  EXPECT_EQ(pl.head, pos3);
  EXPECT_EQ(pl.tail, pos3);
  EXPECT_EQ(pos3->prev, nullptr);
  EXPECT_EQ(pos3->next, nullptr);

  pl.deleteOrder(pos3, allocator);
  EXPECT_EQ(pl.totalVolume(), 0);
  EXPECT_EQ(pl.order_count, 0);
  EXPECT_EQ(allocator.allocatedCount(), 0);
  verifyLevel(pl, {});
}

TEST(PriceLevel_Intrusive, deleteOrderTail) {
  SlabAllocator<Order_Intrusive, 3> allocator;
  PriceLevel_Intrusive pl{};
  auto pos1 = allocator.allocate();
  pl.addOrder(pos1, 1, 1, Side::Bid, 1, 1);
  auto pos2 = allocator.allocate();
  pl.addOrder(pos2, 1, 1, Side::Bid, 1, 1);
  auto pos3 = allocator.allocate();
  pl.addOrder(pos3, 1, 1, Side::Bid, 1, 1);
  verifyLevel(pl, {pos1, pos2, pos3});
  EXPECT_EQ(allocator.allocatedCount(), 3);

  // Removing the tail must null the new tail's next pointer.
  pl.deleteOrder(pos3, allocator);
  EXPECT_EQ(pl.totalVolume(), 2);
  EXPECT_EQ(pl.order_count, 2);
  EXPECT_EQ(allocator.allocatedCount(), 2);
  verifyLevel(pl, {pos1, pos2});
  EXPECT_EQ(pl.head, pos1);
  EXPECT_EQ(pl.tail, pos2);
  EXPECT_EQ(pos1->next, pos2);
  EXPECT_EQ(pos2->prev, pos1);
  EXPECT_EQ(pl.tail->next, nullptr);

  pl.deleteOrder(pos1, allocator);
  verifyLevel(pl, {pos2});
  EXPECT_EQ(pl.head, pos2);
  EXPECT_EQ(pl.tail, pos2);
  EXPECT_EQ(pos2->prev, nullptr);
  EXPECT_EQ(pos2->next, nullptr);

  pl.deleteOrder(pos2, allocator);
  EXPECT_EQ(pl.totalVolume(), 0);
  EXPECT_EQ(pl.order_count, 0);
  EXPECT_EQ(allocator.allocatedCount(), 0);
  verifyLevel(pl, {});
}
