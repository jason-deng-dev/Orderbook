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

  EXPECT_EQ(t1.getInventoryAmount(&ob), 0);

  t1.changeInventoryAmount(&ob, 3);
  EXPECT_EQ(t1.getInventoryAmount(&ob), 3);

  t1.changeInventoryAmount(&ob, -3);
  EXPECT_EQ(t1.getInventoryAmount(&ob), 0);
}

TEST(TraderTest, balanceOperations) {
  Trader t1("t1", 250);
  EXPECT_EQ(t1.getBalance(), 250);
  t1.changeBalance(500);
  EXPECT_EQ(t1.getBalance(), 750);
  t1.changeBalance(-800);
  EXPECT_EQ(t1.getBalance(), -50);
}

TEST(TraderTest, buyOrderOperations) {
  Trader t1("t1");
   Orderbook ob{"Stock"};
  Order o1(0, 10, t1.getId(), 50);
  Order o2(1, 10, t1.getId(), 70);
  Order o3(2, 10, t1.getId(), 100);

  EXPECT_EQ(t1.getBestBid(&ob), -1);

  t1.addBuyOrder(&ob, &o1);
  EXPECT_EQ(t1.getBestBid(&ob), 50);

  t1.addBuyOrder(&ob, &o2);
  EXPECT_EQ(t1.getBestBid(&ob), 70);

  t1.addBuyOrder(&ob, &o3);
  EXPECT_EQ(t1.getBestBid(&ob), 100);

  t1.removeBuyOrder(&ob, 100, o3.trade_id_);
  EXPECT_EQ(t1.getBestBid(&ob), 70);

  t1.removeBuyOrder(&ob, 70, o2.trade_id_);
  EXPECT_EQ(t1.getBestBid(&ob), 50);

  t1.removeBuyOrder(&ob, 50, o1.trade_id_);
  EXPECT_EQ(t1.getBestBid(&ob), -1);
}
