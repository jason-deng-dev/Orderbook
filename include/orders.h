#ifndef ORDERS_H
#define ORDERS_H

#include <functional>
#include <queue>
#include <unordered_map>
#include <map>
#include <utility>
#include <vector>

struct Order {
  static int currId = 0;
  


  int trader_id;
  int quantity;
};

struct OrdersAtPrice {
  int total_quantity = 0;
  int total_orders = 0;
  std::queue<Order> order_queue;
};

class BuyOrders {
private:
  // <price, orders>
  std::map<int, OrdersAtPrice*> buyOrders;

public:
  const int getBestBid() const { return buyOrders.rbegin()->first; }
};
class SellOrders {
private:
  std::map<int, OrdersAtPrice*> sellOrders;

public:
  const int getBestAsk() const { return sellOrders.begin()->first; }
};

#endif
