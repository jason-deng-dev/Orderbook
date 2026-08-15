#ifndef ORDERS_H
#define ORDERS_H

#include <functional>
#include <map>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

struct Order {
  static int currId;
  int trader_id_;
  int quantity_;

  Order(int quantity) : quantity_(quantity) { trader_id_ = currId++; }
};

struct OrdersAtPrice {
  int total_quantity = 0;
  int total_orders = 0;
  // <Order::trader_id_, Order*>
  std::map<int, Order*>order_queue;
};

class BuyOrders {
private:
  // <price, orders>
  std::map<int, OrdersAtPrice *> buyOrders;

public:
  const int getBestBid() const { return buyOrders.rbegin()->first; }
};
class SellOrders {
private:
  std::map<int, OrdersAtPrice *> sellOrders;

public:
  const int getBestAsk() const { return sellOrders.begin()->first; }
};

#endif
