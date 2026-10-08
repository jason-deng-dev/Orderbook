#pragma once

#include "types.h"
#include <cassert>
#include <functional>
#include <map>

enum class OrderbookError { OK, DuplicateId, OrderNotFound, InvalidVolume, InvalidPrice, PriceLevelNotFound };

class Orderbook_Map {
public:
  [[nodiscard]] OrderbookError AddOrder(OrderId orderId, Side side, Price price, Volume volume, TraderId traderId);
  [[nodiscard]] OrderbookError ModifyOrder(OrderId orderId, Volume newVolume);
  [[nodiscard]] OrderbookError DeleteOrder(OrderId orderId);

private:
  template <typename T>
  [[nodiscard]] OrderbookError AddOrder(T &levels, Side side, OrderId orderId, Price price, Volume volume,
                                        TraderId traderId);

  template <typename T>
  [[nodiscard]] OrderbookError DeleteOrder(std::list<Order>::iterator orderIt, T &levels);

  template <typename T>
  [[nodiscard]] OrderbookError ModifyOrder(std::list<Order>::iterator orderIt, T &levels, Volume newVolume);

  std::map<Price, PriceLevel, std::greater<Price>> bidLevels;

  std::map<Price, PriceLevel, std::less<Price>> askLevels;

  std::unordered_map<OrderId, std::list<Order>::iterator> idMap;
};
