#include "orderbook.h"
#include "orders.h"
#include "trader.h"
#include <gtest/gtest.h>

TEST(TraderTest, Initalization) {
  Trader t1("t1");
  EXPECT_EQ(t1.getId(), 0);
  Trader t2("t2", 50);
  Trader t3("t3", 150);
  EXPECT_EQ(t2.getId(), 1);
  EXPECT_EQ(t3.getId(), 2);
  EXPECT_EQ(t1.getBalance(), 0);
  EXPECT_EQ(t2.getBalance(), 50);
  EXPECT_EQ(t3.getBalance(), 150);
  EXPECT_EQ(t1.getName(), "t1");
  EXPECT_EQ(t2.getName(), "t2");
  EXPECT_EQ(t3.getName(), "t3");
}

TEST(TraderTest, InventoryOperations) {
  Trader t1("t1", 250);
  Orderbook ob{"Stock"};

  EXPECT_EQ(t1.getInventoryAmount(&ob), 0) << "failed 1";

  t1.changeInventoryAmount(&ob, 3);
  EXPECT_EQ(t1.getInventoryAmount(&ob), 3) << "failed 2";

  t1.changeInventoryAmount(&ob, -3);
  EXPECT_EQ(t1.getInventoryAmount(&ob), 0) << "failed 3";
}
