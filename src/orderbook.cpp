#include "../include/orderbook.h"
#include "../include/trader.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>

bool Orderbook::buy(Trader *trader, int price, int quantity) {
  if (quantity <= 0) {
    std::cerr << "buy failed, cannot buy less than or equal to 0 quantity\n";
    return false;
  }

  if (trader->getBalance() < quantity * price) {
    std::cerr << "buy failed, insufficent funds\n";
    return false;
  }
  if (!buyValidityCheck(trader, price)) {
    std::cerr << "buy failed, you currently have resting order than can match "
                 "against current buy order.\n";
    return false;
  }

  traderRegistry[trader->getId()] = trader;
  buyOrderMap[price].total_quantity += quantity;

  int trade_id = currTradeId++;
  // emplace returns <iterator to inserted element, bool inserted>
  auto [it, inserted] = buyOrderMap[price].order_queue.emplace(
      trade_id, Order(trade_id, quantity, trader->getId(), price));
  trader->addBuyOrder(this, &it->second);
  trader->changeBalance(-quantity * price);

    handleFill();
  // if (handleFill()) {
  //   std::cout << "Bid by trader:" << trader->getName() << " price:" << price
  //             << " quantity:" << quantity << " triggered a order fill\n";
  // }

  return true;
}

bool Orderbook::sell(Trader *trader, int price, int quantity) {
  if (quantity <= 0) {
    std::cerr << "sell failed, cannot sell less than or equal to 0 quantity\n";
    return false;
  }

  if (trader->getInventoryAmount(this) < quantity) {
    std::cerr << "sell failed, insufficent holding of stock \n";
    return false;
  }

  if (!sellValidityCheck(trader, price)) {
    std::cerr << "sell failed, you currently have resting order than can match "
                 "against current sell order.\n";
    return false;
  }

  traderRegistry[trader->getId()] = trader;
  sellOrderMap[price].total_quantity += quantity;

  trader->changeInventoryAmount(this, -quantity);
  int trade_id = currTradeId++;
  auto [it, inserted] = sellOrderMap[price].order_queue.emplace(
      trade_id, Order(trade_id, quantity, trader->getId(), price));
  trader->addSellOrder(this, &it->second);

  handleFill();
  // if (handleFill()) {
  //   std::cout << "Ask by trader:" << trader->getName() << " price:" << price
  //             << " quantity:" << quantity << " triggered a order fill\n";
  // }

  return true;
}

bool Orderbook::cancelBuy(Trader *trader, int trade_id, int price,
                          int quantity = 0) {
  // if fully cancelBuyOrder remove from Trader::buyOrders
  // on cancel return balance
  if (quantity < 0) {
    std::cerr
        << "buy cancel failed, cannot cancel amount less than or equal to 0\n";
    return false;
  }

  // fail if order doesn't exist in buyOrderMap;
  if (!buyOrderMap.count(price) ||
      !buyOrderMap.at(price).order_queue.count(trade_id)) {
    std::cerr << "buy cancel failed, order may not exist\n";
    return false;
  }

  auto &order = buyOrderMap.at(price).order_queue.at(trade_id);
  int orderQty = order.quantity_;

  if (order.quantity_ < quantity) {
    std::cerr << "buy cancel failed, cancel amount greater than order amount\n";
    return false;
  }

  if (order.trader_id_ != trader->getId()) {
    std::cerr << "buy cancel failed, not your order\n";
    return false;
  }

  // if quantity is left blank, fully cancel order
  if (quantity == 0 || quantity == order.quantity_) {
    trader->removeBuyOrder(this, price, trade_id);
    buyOrderMap[price].total_quantity -= order.quantity_;
    buyOrderMap[price].order_queue.erase(trade_id);

    // if order_queue is empty at price, remove from buyOrderMap
    if (buyOrderMap[price].order_queue.empty()) {
      buyOrderMap.erase(price);
    }

    trader->changeBalance(orderQty * price);
  }

  // partially cancel
  else {
    buyOrderMap[price].total_quantity -= quantity;
    order.quantity_ -= quantity;
    trader->changeBalance(quantity * price);
  }

  return true;
}

bool Orderbook::cancelSell(Trader *trader, int trade_id, int price,
                           int quantity = 0) {

  if (quantity < 0) {
    std::cerr
        << "sell cancel failed, cannot cancel amount less than or equal to 0\n";
    return false;
  }

  if (!sellOrderMap.count(price) ||
      !sellOrderMap.at(price).order_queue.count(trade_id)) {
    std::cerr << "sell cancel failed, order may not exist\n";
    return false;
  }

  auto &order = sellOrderMap.at(price).order_queue.at(trade_id);
  int orderQty = order.quantity_;

  if (order.quantity_ < quantity) {
    std::cerr
        << "sell cancel failed, cancel amount greater than order amount\n";
    return false;
  }

  if (order.trader_id_ != trader->getId()) {
    std::cerr << "sell cancel failed, not your order\n";
    return false;
  }

  // if quantity is left blank, fully cancel order
  if (quantity == 0 || quantity == order.quantity_) {
    trader->removeSellOrder(this, price, trade_id);
    sellOrderMap[price].total_quantity -= order.quantity_;
    sellOrderMap[price].order_queue.erase(trade_id);

    // if order_queue is empty at price, remove from buyOrderMap
    if (sellOrderMap[price].order_queue.empty()) {
      sellOrderMap.erase(price);
    }

    trader->changeInventoryAmount(this, orderQty);
  }

  // partially cancel
  else {
    sellOrderMap[price].total_quantity -= quantity;
    order.quantity_ -= quantity;
    trader->changeInventoryAmount(this, quantity);
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
      std::cout << '\n';
    }
  }
  std::cout << "\n\n\n";
}

const Order *Orderbook::findOrder(const std::map<int, OrdersAtPrice> &orderMap,
                                  int price, int trade_id) const {
  auto level = orderMap.find(price);
  if (level == orderMap.end())
    return nullptr;
  auto it = level->second.order_queue.find(trade_id);
  if (it == level->second.order_queue.end())
    return nullptr;
  return &it->second;
}

Order *Orderbook::getBestBid() {
  auto &ordersAtPrice = buyOrderMap[getBestBidPrice()];
  return &ordersAtPrice.order_queue.begin()->second;
};
Order *Orderbook::getBestAsk() {
  auto &ordersAtPrice = sellOrderMap[getBestAskPrice()];
  return &ordersAtPrice.order_queue.begin()->second;
};

// invariant is this only called by Orderbook::handleFill when there is a
// bid/ask
void Orderbook::removeBestBid() {
  int bestPrice = getBestBidPrice();
  auto &oq = buyOrderMap[bestPrice].order_queue;
  oq.erase(oq.begin());
  if (oq.empty()) {
    buyOrderMap.erase(bestPrice);
  }
};

void Orderbook::removeBestAsk() {
  int bestPrice = getBestAskPrice();
  auto &oq = sellOrderMap[bestPrice].order_queue;
  oq.erase(oq.begin());
  if (oq.empty()) {
    sellOrderMap.erase(bestPrice);
  }
};

bool Orderbook::handleFill() {
  int bestBidPrice = getBestBidPrice();
  int bestAskPrice = getBestAskPrice();

  // no bid or no ask
  if (bestBidPrice == -1 || bestAskPrice == -1) {
    return false;
  }

  if (buyOrderMap.empty() || sellOrderMap.empty() ||
      bestBidPrice < bestAskPrice)
    return false;

  while (!buyOrderMap.empty() && !sellOrderMap.empty()) {
    bestBidPrice = getBestBidPrice();
    bestAskPrice = getBestAskPrice();

    if (bestBidPrice < bestAskPrice)
      break;

    Order *bidOrder = getBestBid();
    Order *askOrder = getBestAsk();
    assert(bidOrder != nullptr && askOrder != nullptr);

    auto buyer = traderRegistry[bidOrder->trader_id_];
    auto seller = traderRegistry[askOrder->trader_id_];

    assert(buyer != nullptr && seller != nullptr);

    int fillQty = std::min(bidOrder->quantity_, askOrder->quantity_);
    int fillPrice = getBestBidPrice();

    if (getBestBidPrice() > getBestAskPrice() && bidOrder->ts > askOrder->ts) {
      fillPrice = getBestAskPrice();
      int refundAmount = fillQty * (getBestBidPrice() - getBestAskPrice());
      buyer->changeBalance(refundAmount);
    }

    tradeHistory.push_back(
        {bidOrder->trader_id_, askOrder->trader_id_, fillPrice, fillQty});

    // add funds to seller
    seller->changeBalance(fillPrice * fillQty);

    // add stock to buyer
    buyer->changeInventoryAmount(this, fillQty);

    bidOrder->quantity_ -= fillQty;
    askOrder->quantity_ -= fillQty;

    if (bidOrder->quantity_ == 0) {
      // need to remove bid from trader
      traderRegistry[bidOrder->trader_id_]->removeBuyOrder(
          this, bidOrder->price_, bidOrder->trade_id_);
      removeBestBid();
    }
    if (askOrder->quantity_ == 0) {
      traderRegistry[askOrder->trader_id_]->removeSellOrder(
          this, askOrder->price_, askOrder->trade_id_);
      removeBestAsk();
    }
    // std::cout << "Order filled at " << tradeHistory.back().getTime() << '\n';
  }
  return true;
}

bool Orderbook::buyValidityCheck(Trader *trader, int price) {
  int bestAsk = trader->getBestAsk(this);
  if (bestAsk == -1 || bestAsk > price)
    return true;
  return false;
};
bool Orderbook::sellValidityCheck(Trader *trader, int price) {
  int bestBid = trader->getBestBid(this);
  if (bestBid == -1 || bestBid < price)
    return true;
  return false;
};
