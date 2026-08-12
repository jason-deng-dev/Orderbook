#ifndef ORDERS_H
#define ORDERS_H

#include <functional>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

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

#endif
