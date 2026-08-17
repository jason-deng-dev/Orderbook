#ifndef ORDERS_H
#define ORDERS_H

#include <functional>
#include <map>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

class Trader;

struct Order {
  static int currId;
  int price_;
  int trade_id_;
  int quantity_;
  int trader_id_;
  Order(int quantity, int trader_id, int price)
      : price_(price), quantity_(quantity), trader_id_(trader_id) {
    trade_id_ = currId++;
  }
};

struct OrdersAtPrice {
  int total_quantity = 0;
  int total_orders = 0;
  // <Order::trade_id_, Order*>
  std::map<int, Order *> order_queue;
};

class BuyOrders {
private:
  // <price, orders>
  std::map<int, OrdersAtPrice *> buyOrderMap;

public:
  bool add(Trader *trader, int quantity, int price);
  bool cancel(Trader *trader, int quanity, int price);

  const int getBestBid() const { return buyOrderMap.rbegin()->first; }
};
class SellOrders {
private:
  std::map<int, OrdersAtPrice *> sellOrderMap;

public:
  const int getBestAsk() const { return sellOrderMap.begin()->first; }
};

#endif
