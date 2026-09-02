#include "orderbook.h"
#include "orders.h"
#include "trader.h"
#include <gtest/gtest.h>

// Trader tests
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

// Orderbook tests
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

// resting ask, crossing buy at same price -> full fill at ask price
TEST(OrderbookTest, fillSamePrice) {
  Orderbook ob{"Stock1"};
  Trader seller("seller", 0);
  Trader buyer("buyer", 200);
  seller.changeInventoryAmount(&ob, 5);

  // resting ask, no bid -> no fill
  EXPECT_EQ(ob.sell(&seller, 10, 5), true);
  EXPECT_EQ(ob.getBestAskPrice(), 10);
  EXPECT_EQ(buyer.getBalance(), 200);

  // crossing buy at same price -> full fill, both sides consumed
  EXPECT_EQ(ob.buy(&buyer, 10, 5), true);
  EXPECT_EQ(buyer.getInventoryAmount(&ob), 5);
  EXPECT_EQ(buyer.getBalance(), 150); // paid 5 * 10
  EXPECT_EQ(seller.getInventoryAmount(&ob), 0);
  EXPECT_EQ(seller.getBalance(), 50); // received 5 * 10
  EXPECT_EQ(ob.getBestBidPrice(), -1);
  EXPECT_EQ(ob.getBestAskPrice(), -1);
  // filled orders removed from book
  EXPECT_EQ(ob.getBuyOrder(10, 1), nullptr);
  EXPECT_EQ(ob.getSellOrder(10, 0), nullptr);
}

// resting bid, aggressor sell below bid -> fill at passive bid price
TEST(OrderbookTest, fillAggressorSellGetsBidPrice) {
  Orderbook ob{"Stock1"};
  Trader buyer("buyer", 200);
  Trader seller("seller", 0);
  seller.changeInventoryAmount(&ob, 5);

  // resting bid 5 @ 10
  EXPECT_EQ(ob.buy(&buyer, 10, 5), true);
  EXPECT_EQ(buyer.getBalance(), 150);
  EXPECT_EQ(ob.getBestBidPrice(), 10);

  // seller crosses at 9, fills at resting bid price 10
  EXPECT_EQ(ob.sell(&seller, 9, 5), true);
  EXPECT_EQ(seller.getBalance(), 50); // got 10, not 9
  EXPECT_EQ(seller.getInventoryAmount(&ob), 0);
  EXPECT_EQ(buyer.getInventoryAmount(&ob), 5);
  EXPECT_EQ(buyer.getBalance(), 150); // no refund, paid 10 upfront
  EXPECT_EQ(ob.getBestBidPrice(), -1);
  EXPECT_EQ(ob.getBestAskPrice(), -1);
}

// resting ask, aggressor buy above ask -> fill at ask price with refund
TEST(OrderbookTest, fillAggressorBuyPaysAskPrice) {
  Orderbook ob{"Stock1"};
  Trader seller("seller", 0);
  Trader buyer("buyer", 200);
  seller.changeInventoryAmount(&ob, 5);

  // resting ask 5 @ 9
  EXPECT_EQ(ob.sell(&seller, 9, 5), true);
  EXPECT_EQ(ob.getBestAskPrice(), 9);

  // buyer crosses at 10, fills at 9, gets refunded the difference
  EXPECT_EQ(ob.buy(&buyer, 10, 5), true);
  EXPECT_EQ(buyer.getBalance(), 155); // paid 50, refunded 5
  EXPECT_EQ(buyer.getInventoryAmount(&ob), 5);
  EXPECT_EQ(seller.getBalance(), 45); // received 5 * 9
  EXPECT_EQ(seller.getInventoryAmount(&ob), 0);
  EXPECT_EQ(ob.getBestBidPrice(), -1);
  EXPECT_EQ(ob.getBestAskPrice(), -1);
}

// resting bid partially filled, remaining qty stays on the book
TEST(OrderbookTest, fillPartialFillRestingBid) {
  Orderbook ob{"Stock1"};
  Trader buyer("buyer", 200);
  Trader seller("seller", 0);
  seller.changeInventoryAmount(&ob, 3);

  // resting bid 5 @ 10
  EXPECT_EQ(ob.buy(&buyer, 10, 5), true);

  // seller crosses with 3 -> partial fill of bid
  EXPECT_EQ(ob.sell(&seller, 9, 3), true);
  EXPECT_EQ(buyer.getInventoryAmount(&ob), 3);
  EXPECT_EQ(seller.getBalance(), 30); // filled at resting bid price 10
  EXPECT_EQ(seller.getInventoryAmount(&ob), 0);
  // remaining 2 @ 10 still resting
  ASSERT_NE(ob.getBuyOrder(10, 0), nullptr);
  EXPECT_EQ(ob.getBuyOrder(10, 0)->quantity_, 2);
  EXPECT_EQ(ob.getBestBidPrice(), 10);
  EXPECT_EQ(ob.getBestAskPrice(), -1);
}

// aggressor bid partially filled, remaining qty stays on the book
TEST(OrderbookTest, fillPartialFillAggressorBid) {
  Orderbook ob{"Stock1"};
  Trader seller("seller", 0);
  Trader buyer("buyer", 200);
  seller.changeInventoryAmount(&ob, 3);

  // resting ask 3 @ 9
  EXPECT_EQ(ob.sell(&seller, 9, 3), true);

  // buyer crosses with 5 -> fills 3 at ask price 9, refunds 3
  EXPECT_EQ(ob.buy(&buyer, 10, 5), true);
  EXPECT_EQ(buyer.getInventoryAmount(&ob), 3);
  EXPECT_EQ(buyer.getBalance(), 153); // paid 50, refunded 3 * (10 - 9)
  EXPECT_EQ(seller.getBalance(), 27); // received 3 * 9
  EXPECT_EQ(seller.getInventoryAmount(&ob), 0);
  // remaining 2 @ 10 still resting
  ASSERT_NE(ob.getBuyOrder(10, 1), nullptr);
  EXPECT_EQ(ob.getBuyOrder(10, 1)->quantity_, 2);
  EXPECT_EQ(ob.getBestBidPrice(), 10);
}

// no cross: bid below ask, both orders rest
TEST(OrderbookTest, fillNoCross) {
  Orderbook ob{"Stock1"};
  Trader buyer("buyer", 200);
  Trader seller("seller", 0);
  seller.changeInventoryAmount(&ob, 5);

  EXPECT_EQ(ob.buy(&buyer, 9, 5), true);   // bid 9
  EXPECT_EQ(ob.sell(&seller, 10, 5), true); // ask 10
  EXPECT_EQ(buyer.getBalance(), 155);       // no fill, no cash movement
  EXPECT_EQ(seller.getBalance(), 0);
  EXPECT_EQ(buyer.getInventoryAmount(&ob), 0);
  EXPECT_EQ(seller.getInventoryAmount(&ob), 0);
  EXPECT_EQ(ob.getBestBidPrice(), 9);
  EXPECT_EQ(ob.getBestAskPrice(), 10);
  // both orders still on the book
  ASSERT_NE(ob.getBuyOrder(9, 0), nullptr);
  EXPECT_EQ(ob.getBuyOrder(9, 0)->quantity_, 5);
  ASSERT_NE(ob.getSellOrder(10, 1), nullptr);
  EXPECT_EQ(ob.getSellOrder(10, 1)->quantity_, 5);
}

// aggressive bid crosses two ask levels, refund per level
TEST(OrderbookTest, fillMultiLevel) {
  Orderbook ob{"Stock1"};
  Trader buyer("buyer", 1000);
  Trader seller1("seller1", 0);
  Trader seller2("seller2", 0);
  seller1.changeInventoryAmount(&ob, 5);
  seller2.changeInventoryAmount(&ob, 3);

  // resting asks: 5 @ 90, 3 @ 95
  EXPECT_EQ(ob.sell(&seller1, 90, 5), true);
  EXPECT_EQ(ob.sell(&seller2, 95, 3), true);
  EXPECT_EQ(ob.getBestAskPrice(), 90);

  // buyer crosses with 8 @ 100 -> 5 @ 90 (refund 50), 3 @ 95 (refund 15)
  EXPECT_EQ(ob.buy(&buyer, 100, 8), true);
  EXPECT_EQ(buyer.getBalance(), 265); // paid 800, refunded 65
  EXPECT_EQ(buyer.getInventoryAmount(&ob), 8);
  EXPECT_EQ(seller1.getBalance(), 450);
  EXPECT_EQ(seller2.getBalance(), 285);
  EXPECT_EQ(seller1.getInventoryAmount(&ob), 0);
  EXPECT_EQ(seller2.getInventoryAmount(&ob), 0);
  EXPECT_EQ(ob.getBestBidPrice(), -1);
  EXPECT_EQ(ob.getBestAskPrice(), -1);
}

// own resting order cannot be crossed (self-trade prevention)
TEST(OrderbookTest, fillSelfTradePrevented) {
  Orderbook ob{"Stock1"};
  Trader t1("t1", 200);
  t1.changeInventoryAmount(&ob, 10);

  // resting ask 10 @ 10
  EXPECT_EQ(ob.sell(&t1, 10, 10), true);
  EXPECT_EQ(ob.getBestAskPrice(), 10);

  // own buy at or above own ask -> blocked
  EXPECT_EQ(ob.buy(&t1, 11, 1), false);
  EXPECT_EQ(ob.buy(&t1, 10, 1), false);
  EXPECT_EQ(t1.getBalance(), 200); // no cash movement

  // own buy below own ask -> allowed, both rest
  EXPECT_EQ(ob.buy(&t1, 9, 1), true);
  EXPECT_EQ(t1.getBalance(), 191);
  EXPECT_EQ(ob.getBestBidPrice(), 9);
  EXPECT_EQ(ob.getBestAskPrice(), 10);
}

// FIX: filled orders leave stale entries in Trader's book
TEST(OrderbookTest, fillCleansTraderBook) {
  Orderbook ob{"Stock1"};
  Trader buyer("buyer", 200);
  Trader seller("seller", 0);
  seller.changeInventoryAmount(&ob, 5);

  // full fill 5 @ 10
  EXPECT_EQ(ob.buy(&buyer, 10, 5), true);
  EXPECT_EQ(ob.sell(&seller, 10, 5), true);
  EXPECT_EQ(ob.getBestBidPrice(), -1);
  EXPECT_EQ(ob.getBestAskPrice(), -1);

  // trader side should also be clean after fill
  EXPECT_EQ(buyer.getBestBid(&ob), -1); 
  EXPECT_EQ(seller.getBestAsk(&ob), -1); 
}
