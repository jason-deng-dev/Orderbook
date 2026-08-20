#ifndef ORDERS_H
#define ORDERS_H

#include <chrono>
#include <functional>
#include <map>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

class Trader;

struct Order {
  std::chrono::system_clock::time_point ts;
  int trade_id_;
  int quantity_;
  int trader_id_;
  int price_;
  Order(int trade_id, int quantity, int trader_id, int price)
      : ts(std::chrono::system_clock::now()), trade_id_(trade_id),
        quantity_(quantity), trader_id_(trader_id), price_(price) {}
};

struct OrdersAtPrice {
  int total_quantity = 0;
  // <Order::trade_id_, Order*>
  std::map<int, Order> order_queue;
};

struct Trade {
  std::chrono::system_clock::time_point time_of_fill_;
  int buyer_id_;
  int seller_id_;
  int fill_price_;
  int fill_qty_;
  Trade(int buyer_id, int seller_id, int fill_price, int fill_qty)
      : time_of_fill_(std::chrono::system_clock::now()), buyer_id_(buyer_id),
        seller_id_(seller_id), fill_price_(fill_price), fill_qty_(fill_qty) {}
};

#endif
