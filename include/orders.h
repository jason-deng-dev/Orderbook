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
  
  int trade_id_;
  int quantity_;
  int trader_id_;
  int price_;
  Order(int trade_id, int quantity, int trader_id, int price)
      : trade_id_(trade_id), quantity_(quantity), trader_id_(trader_id),
        price_(price) {}
};

struct OrdersAtPrice {
  int total_quantity = 0;
  // <Order::trade_id_, Order*>
  std::map<int, Order> order_queue;
};

#endif
