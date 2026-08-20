#include "../include/orderbook.h"
#include "../include/trader.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>

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

bool Orderbook::cancelSell(Order *order, int quantity = 0) {
  int trade_id = order->trade_id_;
  int price = order->price_;

  // fail if order doesn't exist in buyOrderMap;
  if (!sellOrderMap.count(price) ||
      !sellOrderMap[price].order_queue.count(trade_id)) {
    std::cerr << "sell cancelation failed, order may not exist\n";
    return false;
  }

  if (quantity == 0) {
    sellOrderMap[price].total_quantity -= order->quantity_;
    sellOrderMap[price].order_queue.erase(trade_id);
  } else {
    sellOrderMap[price].total_quantity -= quantity;
    order->quantity_ -= quantity;
  }
  return true;
}

void Orderbook::displayBuyOrders() const {
  std::cout << "Buy Orders: \n";
  for (auto it = buyOrderMap.rbegin(); it != buyOrderMap.rend(); ++it) {
    int price = it->first;
    auto &orders = it->second;
    std::cout << "Buyers:" << orders.order_queue.size()
              << " Volume:" << orders.total_quantity << " Buy:" << price
              << '\n';
  }
  std::cout << '\n';
}

void Orderbook::displaySellOrders() const {
  std::cout << "Sell Orders: \n";
  for (const auto &[price, orders] : sellOrderMap)
    std::cout << "Sellers:" << orders.order_queue.size()
              << " Volume:" << orders.total_quantity << " Sell:" << price
              << '\n';
  std::cout << '\n';
}

// Buy Orders                   Sell Orders
// Buyers   Volume    Buy       Sell       Volume       Sellers
void Orderbook::displayOrders() const {
  std::cout << std::string(110, '-') << '\n';
  std::cout << std::string(18, ' ') << "Buy Orders" << std::string(52, ' ')
            << "Sell Orders\n";
  std::cout << std::string(110, '-') << '\n';
  // 20 | 20 | 14 | 14 | 20 | 8
  std::cout << "Buyers" << std::string(14, ' ') << "Volume"
            << std::string(14, ' ') << "Buy";
  std::cout << std::string(11, ' ') << "|" << std::string(10, ' ');

  std::cout << "Sell" << std::string(14, ' ') << "Volume"
            << std::string(14, ' ') << "Sellers";

  std::cout << '\n' << std::string(110, '-') << '\n';

  int sellSize = static_cast<int>(sellOrderMap.size());
  int buySize = static_cast<int>(buyOrderMap.size());
  // displays up to 10 of best buy/sell offers
  int maxSize = std::max(sellSize, buySize);
  maxSize = std::min(maxSize, 10);

  auto buy_it = buyOrderMap.rbegin();
  auto sell_it = sellOrderMap.begin();

  for (int i = 0; i < maxSize; ++i) {
    if (buy_it != buyOrderMap.rend()) {
      auto &orders = buy_it->second;
      int buyers = static_cast<int>(orders.order_queue.size());
      std::size_t buyers_gap = 20 - std::to_string(buyers).length();

      int volume = static_cast<int>(orders.total_quantity);
      std::size_t volume_gap = 20 - std::to_string(volume).length();

      int price = static_cast<int>(buy_it->first);
      std::size_t price_gap = 14 - std::to_string(price).length();

      std::cout << orders.order_queue.size() << std::string(buyers_gap, ' ')
                << orders.total_quantity << std::string(volume_gap, ' ')
                << price << std::string(price_gap, ' ');

      ++buy_it;

    } else {
      std::cout << std::string(54, ' ');
    }

    std::cout << "|";

    if (sell_it != sellOrderMap.end()) {
      auto &orders = sell_it->second;
      int sellers = static_cast<int>(orders.order_queue.size());

      int volume = static_cast<int>(orders.total_quantity);
      std::size_t volume_gap = 17 - std::to_string(sellers).length();

      int price = static_cast<int>(sell_it->first);

      std::size_t seller_gap = 20 - std::to_string(volume).length();

      std::cout << std::string(10, ' ') << price << std::string(volume_gap, ' ')
                << orders.total_quantity << std::string(seller_gap, ' ')
                << orders.order_queue.size();
      std::cout << '\n';
      ++sell_it;
    } else {
      std::cout << std::string(54, ' ');
    }
    std::cout <<'\n';
  }
}
