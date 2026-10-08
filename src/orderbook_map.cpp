#include "orderbook_map.h"

// ----------------------------- AddOrder --------------------------------
[[nodiscard]] OrderbookError Orderbook_Map::AddOrder(OrderId orderId, Side side, Price price, Volume volume,
                                                     TraderId traderId) {
  if (idMap.contains(orderId)) return OrderbookError::DuplicateId;
  if (side == Side::Bid) {
    return AddOrder(bidLevels, side, orderId, price, volume, traderId);
  } else {
    return AddOrder(askLevels, side, orderId, price, volume, traderId);
  }
  return OrderbookError::OK;
};

template <typename T>
[[nodiscard]] OrderbookError Orderbook_Map::AddOrder(T &levels, Side side, OrderId orderId, Price price, Volume volume,
                                                     TraderId traderId) {
  Order newOrder{side, orderId, traderId, price, volume};
  auto [levelIt, inserted] = levels.try_emplace(price);
  auto &priceLevel = levelIt->second;
  priceLevel.total_volume += volume;
  auto orderIt = priceLevel.orders.emplace(priceLevel.orders.end(), newOrder);
  idMap.emplace(orderId, orderIt);
  return OrderbookError::OK;
}

// ----------------------------- DeleteOrder --------------------------------
[[nodiscard]] OrderbookError Orderbook_Map::DeleteOrder(OrderId orderId) {
  auto mapIt = idMap.find(orderId);

  if (mapIt == idMap.end()) {
    return OrderbookError::OrderNotFound;
  }
  auto orderIt = mapIt->second;

  if (orderIt->side == Side::Bid) {
    return DeleteOrder(orderIt, bidLevels);
  } else {
    return DeleteOrder(orderIt, askLevels);
  }
};

template <typename T>
[[nodiscard]] OrderbookError Orderbook_Map::DeleteOrder(std::list<Order>::iterator orderIt, T &levels) {
  auto levelIt = levels.find(orderIt->price);
  if (levelIt == levels.end()) {
    return OrderbookError::PriceLevelNotFound;
  }
  PriceLevel &priceLevel = levelIt->second;

  if (priceLevel.total_volume < orderIt->volume) {
    return OrderbookError::InvalidVolume; // volume can't drop below 0
  }

  priceLevel.total_volume -= orderIt->volume;
  idMap.erase(orderIt->id);
  priceLevel.orders.erase(orderIt);

  if (priceLevel.orders.empty()) {
    levels.erase(levelIt);
  }
  return OrderbookError::OK;
}

// ----------------------------- ModifyOrder --------------------------------

[[nodiscard]] OrderbookError Orderbook_Map::ModifyOrder(OrderId orderId, Volume newVolume) {
  if (newVolume == 0) {
    return DeleteOrder(orderId);
  }

  auto mapIt = idMap.find(orderId);

  if (mapIt == idMap.end()) {
    return OrderbookError::OrderNotFound;
  }
  auto orderIt = mapIt->second;
  if (orderIt->volume <= newVolume) {
    return OrderbookError::InvalidVolume; // can't add orders or cancel 0 orders
  }

  if (orderIt->side == Side::Bid) {
    return ModifyOrder(orderIt, bidLevels, newVolume);
  } else {
    return ModifyOrder(orderIt, askLevels, newVolume);
  }
}

template <typename T>
[[nodiscard]] OrderbookError Orderbook_Map::ModifyOrder(std::list<Order>::iterator orderIt, T &levels,
                                                        Volume newVolume) {
  auto levelIt = levels.find(orderIt->price);
  if (levelIt == levels.end()) {
    return OrderbookError::PriceLevelNotFound;
  }
  PriceLevel &priceLevel = levelIt->second;

  Volume volumeChange = newVolume - orderIt->volume;

  if (priceLevel.total_volume + volumeChange < 0) {
    return OrderbookError::InvalidVolume; // volume can't drop below 0
  }

  priceLevel.total_volume += volumeChange;
  orderIt->volume = newVolume;
  return OrderbookError::OK;
}
