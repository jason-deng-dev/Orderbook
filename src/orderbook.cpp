#include "../include/orderbook.h"
#include "../include/trader.h"
#include <iostream>

bool Orderbook::buy(Trader *trader, int quantity, int price) {
  if (trader->getBalance() < quantity * price) {
    std::cerr << "buy failed, insufficent funds\n";
    return false;
  }

  buyOrderMap[price].total_quantity += quantity;
  int trade_id = currTradeId++;
  // emplace returns <iterator to inserted element, bool inserted>
  auto [it, inserted] = buyOrderMap[price].order_queue.emplace(
      trade_id, Order(trade_id, quantity, trader->getId(), price));
  trader->addBuyOrder(this, &it->second);
  return true;
}

// can partially cancel amount
// can fully cancel order if quantity left blank
bool Orderbook::cancelBuy(Order *order, int quantity = 0) {
  int trade_id = order->trade_id_;
  int price = order->price_;

  // fail if order doesn't exist in buyOrderMap;
  if (!buyOrderMap.count(price) ||
      !buyOrderMap[price].order_queue.count(trade_id)) {
    std::cerr << "buy cancelation failed, order may not exist\n";
    return false;
  }

  if (quantity == 0) {
    buyOrderMap[price].total_quantity -= order->quantity_;
    buyOrderMap[price].order_queue.erase(trade_id);
  } else {
    buyOrderMap[price].total_quantity -= quantity;
    order->quantity_ -= quantity;
  }
  return true;
}

bool Orderbook::sell(Trader *trader, int quantity, int price) {
  if (trader->getInventoryAmount(this) < quantity) {
    std::cerr << "sell failed, insufficent holding of stock \n";
    return false;
  }
  trader->changeInventoryAmount(this, -quantity);
  sellOrderMap[price].total_quantity += quantity;
  int trade_id = currTradeId++;
  auto [it, inserted] = sellOrderMap[price].order_queue.emplace(
      trade_id, Order(trade_id, quantity, trader->getId(), price));
  trader->addSellOrder(this, &it->second);
  return true;
}

bool Orderbook::cancelSell(Order *order, int quantity = 0) {}
