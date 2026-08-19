#ifndef TRADER_H
#define TRADER_H

#include "orders.h"
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class Orderbook;
class Order;

class Trader {

private:
  static int next_id_;
  int id_;
  std::string name_;
  int balance_;
  // unordered_map<Orderbook*, map<price, vector<trade_id>>>
  std::unordered_map<Orderbook *, std::map<int, std::vector<int>>> buyOrders;
  std::unordered_map<Orderbook *, std::map<int, std::vector<int>>> sellOrders;
  std::unordered_map<Orderbook *, int> inventory;

public:
  Trader(const std::string &name, int balance = 0.0)
      : id_(next_id_++), name_(name), balance_(balance) {}

  int getId() const { return id_; }
  int getBalance() const { return balance_; }
  std::string_view getName() const { return name_; }

  int getInventoryAmount(Orderbook *orderbook) { return inventory[orderbook]; }

  void changeInventoryAmount(Orderbook *orderbook, int amount) {
    inventory[orderbook] += amount;
  }

  void addBuyOrder(Orderbook *orderbook, Order *order) {
    buyOrders[orderbook][order->price_].push_back(order->trade_id_);
  }
  void addSellOrder(Orderbook *orderbook, Order *order) {
    sellOrders[orderbook][order->price_].push_back(order->trade_id_);
  }

  void addStock(Orderbook *orderbook, int amount){
    inventory[orderbook] += amount;
  }
};

#endif
