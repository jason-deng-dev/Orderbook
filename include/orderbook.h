#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "orders.h"
#include <functional>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

class Trader;
class Order;
class Trade;

class Orderbook {
private:
  std::string name_;
  int currTradeId{0};
  // <price, OrdersAtPrice>
  std::map<int, OrdersAtPrice> buyOrderMap{};
  std::map<int, OrdersAtPrice> sellOrderMap{};
  std::unordered_map<int, Trader *> traderRegistry{};
  std::vector<Trade> tradeHistory{};

  // checks if order can be filled (true if can, false if not)
  // if can be filled, fill order
  // when order filled add to order history
  bool handleFill();

  const Order *findOrder(const std::map<int, OrdersAtPrice> &orderMap,
                         int price, int trade_id) const;

public:
  Orderbook(const std::string &name) : name_{name} {}
  std::string_view getName() const { return name_; }

  // if succeed add the Orderbook* to trader, and add information on
  // quantity/price update trader balance update buyOrders
  bool buy(Trader *trader, int price, int quantity);

  // if succeed update information on qunatity/price
  // update trader balance
  // update sellOrders
  bool sell(Trader *trader, int price, int quantity);

  bool cancelBuy(Trader *trader, int trade_id, int price, int quantity);
  bool cancelSell(Trader *trader, int trade_id, int price, int quantity);

  int getBestBidPrice() const { return buyOrderMap.rbegin()->first; }
  int getBestAskPrice() const { return sellOrderMap.begin()->first; }

  void displayBuyOrders() const;
  void displaySellOrders() const;
  void displayOrders() const;

  // returns nullptr if doesn't exist
  const Order *getBuyOrder(int price, int trade_id) const {
    return findOrder(buyOrderMap, price, trade_id);
  }
  const Order *getSellOrder(int price, int trade_id) const {
    return findOrder(sellOrderMap, price, trade_id);
  }

  Order *getBestBid();
  Order *getBestAsk();

  // invariant is this only called by Orderbook::handleFill when there is a
  // bid/ask
  void removeBestBid();
  void removeBestAsk();
};

#endif
