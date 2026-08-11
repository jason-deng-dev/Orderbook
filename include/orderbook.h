#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <functional>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

class Trader;

struct Order {
  int trader_id;
  int quantity;
};

struct OrdersAtPrice {
  int total_quantity;
  std::queue<Order> order_queue;
};

class BuyOrders {
private:
  // <price, orders>
  std::priority_queue<std::pair<int, OrdersAtPrice>> max_buy_prices_;

public:
  const int getBuyPrice() const { return max_buy_prices_.top().first; }
};
class SellOrders {
private:
  std::priority_queue<std::pair<int, OrdersAtPrice>,
                      std::vector<std::pair<int, OrdersAtPrice>>,
                      std::greater<std::pair<int, OrdersAtPrice>>>
      min_sell_prices_;

public:
  const int getSellPrice() const { return min_sell_prices_.top().first; }
};

class Orderbook {
private:
  BuyOrders buyOrders_;
  SellOrders sellOrders_;
  // checks if order can be filled (true if can, false if not)
  // if can be filled, fill order
  // when order filled add to order history
  bool handleFill();

public:
  // if succeed add the Orderbook* to trader, and add information on quantity/price
  // update trader balance
  // update buyOrders
  bool buy(Trader* trader, int quantity, int price);

  // if succeed update information on qunatity/price
  // update trader balance
  // update sellOrders
  bool sell(Trader* trader, int quantity, int price);

  bool cancelBuy(Trader* trader, int quantity, int price);
  bool cancelSell(Trader* trader, int quantity, int price);
};

#endif
