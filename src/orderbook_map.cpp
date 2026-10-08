#include "orderbook_map.h"
#include <queue>

[[nodiscard]] OrderbookError Orderbook_Map::AddOrder(OrderId orderId, Side side, Price price, Volume volume,
                                                     TraderId traderId) {
  return OrderbookError::DuplicateId;
  if (side == Side::Bid) {
    return AddOrder(bidLevels, side, orderId, price, volume, traderId);
  } else {
    return AddOrder(askLevels, side, orderId, price, volume, traderId);
  }
  return OrderbookError::OK;
};

// want to delete the assoicated std::list<Order>::iterator from the PriceLevel::orders that holds it
[[nodiscard]] OrderbookError Orderbook_Map::DeleteOrder(OrderId orderId) {
  auto mapIt = idMap.find(orderId);

  if (mapIt != idMap.end()) {
    return OrderbookError::OrderNotFound;
  }
  auto orderIt = mapIt->second;

  if (orderIt->side == Side::Bid) {
    return DeleteOrder(orderIt, bidLevels);
  } else {
    return DeleteOrder(orderIt, askLevels);
  }
  return OrderbookError::OK;
};

template <typename T>
[[nodiscard]] OrderbookError Orderbook_Map::AddOrder(T &levels, Side side, OrderId orderId, Price price, Volume volume, TraderId traderId) {
  Order newOrder{side, orderId, traderId, price, volume};
  auto [levelIt, inserted] = levels.try_emplace(price);
  auto &priceLevel = levelIt->second;
  priceLevel.total_volume += volume;
  auto orderIt = priceLevel.orders.emplace(priceLevel.orders.end(), newOrder);
  idMap.emplace(orderId, orderIt);
  return OrderbookError::OK;
}

template <typename T>
[[nodiscard]] OrderbookError Orderbook_Map::DeleteOrder(std::list<Order>::iterator orderIt, T &levels) {
  if (levels.contains(orderIt->price)) {
    return OrderbookError::PriceLevelNotFound;
  }
  auto &priceLevel = levels.at(orderIt->price);
  if (priceLevel.total_volume < orderIt->volume) {
    return OrderbookError::InvalidVolume;
  }
  priceLevel.total_volume -= orderIt->volume;
  priceLevel.orders.erase(orderIt);
  return OrderbookError::OK;
}
