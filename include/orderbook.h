#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "orders.h"
#include <functional>
#include <map>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

class Trader;

class Orderbook {
private:
  std::map<int, OrdersAtPrice> buyOrderMap;
  std::map<int, OrdersAtPrice> sellOrderMap;

  // checks if order can be filled (true if can, false if not)
  // if can be filled, fill order
  // when order filled add to order history
  bool handleFill();

public:
  // if succeed add the Orderbook* to trader, and add information on
  // quantity/price update trader balance update buyOrders
  bool buy(Trader *trader, int quantity, int price);

  // if succeed update information on qunatity/price
  // update trader balance
  // update sellOrders
  bool sell(Trader *trader, int quantity, int price);

  bool cancelBuy(Trader *trader, int quantity, int price);
  bool cancelSell(Trader *trader, int quantity, int price);
  int getBestBid() const { return buyOrderMap.rbegin()->first; }
  int getBestAsk() const { return sellOrderMap.begin()->first; }
};

#endif
