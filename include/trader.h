#ifndef TRADER_H
#define TRADER_H

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
  // <Orderbook*, vector<Order*>>
  std::unordered_map<Orderbook *, std::vector<Order *>> buyOrders;
  std::unordered_map<Orderbook *, std::vector<Order *>> sellOrders;
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
    buyOrders[orderbook].push_back(order);
  }
  void addSellOrder(Orderbook *orderbook, Order *order) {
    sellOrders[orderbook].push_back(order);
  }
};

#endif
