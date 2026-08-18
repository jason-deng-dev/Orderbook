#include "../include/orderbook.h"
#include "../include/trader.h"

bool Orderbook::buy(Trader *trader, int quantity, int price) {
  if (trader->getBalance() < quantity * price)
    return false;
  buyOrderMap[price].total_quantity += quantity;
  int trade_id = currTradeId++;
  // emplace returns <iterator to inserted element, bool inserted>
  auto [it, inserted] = buyOrderMap[price].order_queue.emplace(
      trade_id, Order(trade_id, quantity, trader->getId(), price));
  trader->addBuyOrder(this, &it->second);
  return true;
}

bool Orderbook::cancelBuy(Trader *trader, int trade_id, int quantity) {}

bool Orderbook::sell(Trader *trader, int quantity, int price) {}

bool Orderbook::cancelSell(Trader *trader, int trade_id, int quantity) {}
