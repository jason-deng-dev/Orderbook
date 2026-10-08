#pragma once

#include "types.h"
#include <cassert>
#include <functional>
#include <map>
#include <optional>


class Orderbook_Map {
public:
  [[nodiscard]] MatchResult AddOrder(OrderId orderId, Side side, Price price, Volume volume, TraderId traderId);
  [[nodiscard]] OrderbookError ModifyOrder(OrderId orderId, Volume newVolume);
  [[nodiscard]] OrderbookError DeleteOrder(OrderId orderId);

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
    if (bidLevels.empty()) {
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

private:
  template <typename T>
  [[nodiscard]] MatchResult AddOrder(T &levels, Side side, OrderId orderId, Price price, Volume volume,
                                        TraderId traderId);

  template <typename T>
  [[nodiscard]] OrderbookError DeleteOrder(std::list<Order>::iterator orderIt, T &levels);

  template <typename T>
  [[nodiscard]] OrderbookError ModifyOrder(std::list<Order>::iterator orderIt, T &levels, Volume newVolume);

  std::map<Price, PriceLevel, std::greater<Price>> bidLevels;

  std::map<Price, PriceLevel, std::less<Price>> askLevels;

  std::unordered_map<OrderId, std::list<Order>::iterator> idMap;
};
