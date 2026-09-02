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

  EXPECT_EQ(t1.removeBuyOrder(&ob, 100, o1.trade_id_), false);
  EXPECT_EQ(t1.removeBuyOrder(&ob, 40, o3.trade_id_), false);

  EXPECT_EQ(t1.removeBuyOrder(&ob, 100, o3.trade_id_), true);
  EXPECT_EQ(t1.getBestBid(&ob), 70);

  EXPECT_EQ(t1.removeBuyOrder(&ob, 70, o2.trade_id_), true);
  EXPECT_EQ(t1.getBestBid(&ob), 50);

  EXPECT_EQ(t1.removeBuyOrder(&ob, 50, o1.trade_id_), true);
  EXPECT_EQ(t1.getBestBid(&ob), -1);

  EXPECT_EQ(t1.removeBuyOrder(&ob, 100, o3.trade_id_), false);
}

TEST(TraderTest, sellOrderOperations) {
  Trader t1("t1");
  Orderbook ob{"Stock"};
  Order o1(0, 10, t1.getId(), 100);
  Order o2(1, 10, t1.getId(), 70);
  Order o3(2, 10, t1.getId(), 50);

  EXPECT_EQ(t1.getBestAsk(&ob), -1);

  t1.addSellOrder(&ob, &o1);
  EXPECT_EQ(t1.getBestAsk(&ob), 100);

  t1.addSellOrder(&ob, &o2);
  EXPECT_EQ(t1.getBestAsk(&ob), 70);

  t1.addSellOrder(&ob, &o3);
  EXPECT_EQ(t1.getBestAsk(&ob), 50);

  EXPECT_EQ(t1.removeSellOrder(&ob, 100, o2.trade_id_), false);
  EXPECT_EQ(t1.removeSellOrder(&ob, 40, o3.trade_id_), false);

  EXPECT_EQ(t1.removeSellOrder(&ob, 50, o3.trade_id_), true);
  EXPECT_EQ(t1.getBestAsk(&ob), 70);

  EXPECT_EQ(t1.removeSellOrder(&ob, 70, o2.trade_id_), true);
  EXPECT_EQ(t1.getBestAsk(&ob), 100);

  EXPECT_EQ(t1.removeSellOrder(&ob, 100, o1.trade_id_), true);
  EXPECT_EQ(t1.getBestAsk(&ob), -1);

  EXPECT_EQ(t1.removeBuyOrder(&ob, 100, o3.trade_id_), false);
}

TEST(OrderbookTest, sellOperations) {
  Orderbook ob{"Stock1"};
  Trader t1("t1", 200);
  Trader t2("t2", 200);

  // give trader 1, 20 stock
  t1.changeInventoryAmount(&ob, 20);
  EXPECT_EQ(t1.getInventoryAmount(&ob), 20);

  // selling when don't have stock
  EXPECT_EQ(ob.sell(&t2, 20, 20), false);

  // selling partial amount
  EXPECT_EQ(ob.getBestAskPrice(), -1);
  EXPECT_EQ(ob.sell(&t1, 20, 1), true);
  EXPECT_EQ(ob.getBestAskPrice(), 20);
  EXPECT_EQ(t1.getInventoryAmount(&ob), 19);

  // unsuccessful cancel sell order
  EXPECT_EQ(ob.cancelSell(&t2, 0, 20, 1), false);
  EXPECT_EQ(ob.cancelSell(&t1, 1, 20, 1), false); // wrong trade id
  EXPECT_EQ(ob.cancelSell(&t1, 0, 19, 1), false); // wrong price
  EXPECT_EQ(ob.cancelSell(&t1, 0, 20, 2), false); // too high quantity
  EXPECT_EQ(ob.cancelSell(&t1, 0, 20, -1), false); // negative quantity


  // successful cancel sell order
  EXPECT_EQ(ob.cancelSell(&t1, 0, 20, 1), true);
  EXPECT_EQ(t1.getInventoryAmount(&ob), 20);

  // selling full amount
  EXPECT_EQ(ob.getBestAskPrice(), -1);
  EXPECT_EQ(ob.sell(&t1, 20, 20), true);
  EXPECT_EQ(ob.getBestAskPrice(), 20);
  EXPECT_EQ(t1.getInventoryAmount(&ob), 0);
}

TEST(OrderbookTest, buyOperations) {
  Orderbook ob{"Stock1"};
  Trader buyer("t2", 200);

  // unsucessful buyOrder
  EXPECT_EQ(ob.buy(&buyer, 20, 11), false); // no enough funds
  EXPECT_EQ(ob.buy(&buyer, 201, 1), false); // no enough funds
  EXPECT_EQ(ob.buy(&buyer, 20, -1), false); // negative quantity

  // sucessful buyOrder
   EXPECT_EQ(ob.getBestBidPrice(), -1);
  EXPECT_EQ(ob.buy(&buyer, 20, 5), true);
  EXPECT_EQ(buyer.getBalance(), 100);
  EXPECT_EQ(ob.getBestBidPrice(), 20);

  // unsuccessful cancel buy order
  EXPECT_EQ(ob.cancelBuy(&buyer, 1, 20, 5), false); // wrong trade id
  EXPECT_EQ(ob.cancelBuy(&buyer, 0, 19, 1), false); // wrong price
  EXPECT_EQ(ob.cancelBuy(&buyer, 0, 20, 6), false); // too high quantity
  EXPECT_EQ(ob.cancelBuy(&buyer, 0, 20, -1), false); // negative quantity

  // sucessful cancel buy order
  EXPECT_EQ(ob.cancelBuy(&buyer, 0, 20, 1), true);
  EXPECT_EQ(buyer.getBalance(), 120);
  EXPECT_EQ(ob.cancelBuy(&buyer, 0, 20, 4), true);
  EXPECT_EQ(buyer.getBalance(), 200);
  EXPECT_EQ(ob.getBestBidPrice(), -1);
}

