#pragma once

#include "types.h"
#include <cassert>
#include <functional>
#include <map>
#include <optional>
#include <vector>

class Orderbook_Map {
public:
  [[nodiscard]] MatchResult AddOrder(OrderId orderId, Side side, Price price, Volume volume, TraderId traderId = -1);
  [[nodiscard]] OrderbookError ModifyOrder(OrderId orderId, Volume newVolume);
  [[nodiscard]] OrderbookError DeleteOrder(OrderId orderId);
  [[nodiscard]] OrderbookError ProcessMessage(const QueueItem &queueItem) {
    switch (queueItem.orderType) {
    case OrderType::Add: {
      return AddOrder(queueItem.orderId, queueItem.side, queueItem.price, queueItem.volume, kUnknownTrader).error;
    }
    case OrderType::Cancel: {
      auto id = queueItem.orderId;
      auto orderOpt = GetOrder(id);
      if (orderOpt == std::nullopt) return OrderbookError::OrderNotFound;
      return ModifyOrder(id, orderOpt->volume - queueItem.volume);
    }
    case OrderType::Delete: {
      return DeleteOrder(queueItem.orderId);
    }
    case OrderType::Replace: {
      auto deleteResult = DeleteOrder(queueItem.removeId);
      if (deleteResult != OrderbookError::OK) return deleteResult;
      return AddOrder(queueItem.orderId, queueItem.side, queueItem.price, queueItem.volume, kUnknownTrader).error;
    }
    }
    return OrderbookError::InvalidOrder;
  }

private:
  template <typename T>
  [[nodiscard]] MatchResult AddOrder(Timestamp ts, T &levels, Side side, OrderId orderId, Price price, Volume volume,
                                     TraderId traderId);
  template <typename T>
  [[nodiscard]] OrderbookError DeleteOrder(Timestamp ts, std::list<Order>::iterator orderIt, T &levels);
  template <typename T>
  [[nodiscard]] OrderbookError ModifyOrder(Timestamp ts, std::list<Order>::iterator orderIt, T &levels,
                                           Volume newVolume);

  [[nodiscard]] MatchResult HandleFill(Side side, Price incomingOrderPrice, Volume incomingOrderVolume,
                                       TraderId incomingTraderId);

  std::map<Price, PriceLevel, std::greater<Price>> bidLevels;

  std::map<Price, PriceLevel, std::less<Price>> askLevels;

  std::unordered_map<OrderId, std::list<Order>::iterator> idMap;

  std::vector<OrderEvent> eventLog;

public:
  // --- getters ---
  // idMap
  [[nodiscard]] bool HasOrder(OrderId orderId) const { return idMap.contains(orderId); }
  [[nodiscard]] std::optional<Order> GetOrder(OrderId orderId) const {
    auto mapIt = idMap.find(orderId);
    if (mapIt == idMap.end()) {
      return std::nullopt;
    }
    return *(mapIt->second);
  }

  // price levels
  [[nodiscard]] std::optional<Price> GetBestBid() const {
    if (bidLevels.empty()) {
      return std::nullopt;
    }
    return bidLevels.begin()->first;
  }

  [[nodiscard]] std::optional<Price> GetBestAsk() const {
    if (askLevels.empty()) {
      return std::nullopt;
    }
    return askLevels.begin()->first;
  };

  [[nodiscard]] Volume GetTotalVolumeAtPrice(Price price, Side side) const {

    if (side == Side::Bid) {
      auto it = bidLevels.find(price);
      return (it != bidLevels.end()) ? it->second.total_volume : 0;
    } else {
      auto it = askLevels.find(price);
      return (it != askLevels.end()) ? it->second.total_volume : 0;
    }
  };

  [[nodiscard]] size_t GetOrderCountAtPrice(Price price, Side side) const {
    if (side == Side::Bid) {
      auto it = bidLevels.find(price);
      return (it != bidLevels.end()) ? it->second.orders.size() : 0;
    } else {
      auto it = askLevels.find(price);
      return (it != askLevels.end()) ? it->second.orders.size() : 0;
    }
  };

  // overall book state
  [[nodiscard]] size_t GetTotalOrderCount() const {
    size_t orderCount = 0;
    for (auto &[price, priceLevel] : bidLevels) {
      orderCount += priceLevel.orders.size();
    }
    for (auto &[price, priceLevel] : askLevels) {
      orderCount += priceLevel.orders.size();
    }
    return orderCount;
  };

  // event log
  [[nodiscard]] const std::vector<OrderEvent> &GetEventLog() const { return eventLog; }
};
