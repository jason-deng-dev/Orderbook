#include "orderbook_map.h"
#include "types.h"
#include <algorithm>
#include <cassert>
#include <optional>

// ----------------------------- AddOrder --------------------------------
[[nodiscard]] MatchResult Orderbook_Map::AddOrder(OrderId orderId, Side side, Price price, Volume volume,
                                                  TraderId traderId) {
  if (idMap.contains(orderId)) return {OrderStatus::Rejected, OrderbookError::DuplicateId, 0, volume};
  if (side == Side::Bid) {
    return AddOrder(bidLevels, side, orderId, price, volume, traderId);
  } else {
    return AddOrder(askLevels, side, orderId, price, volume, traderId);
  }
};

template <typename T>
[[nodiscard]] MatchResult Orderbook_Map::AddOrder(T &levels, Side side, OrderId orderId, Price price, Volume volume,
                                                  TraderId traderId) {
  MatchResult matchResult = HandleFill(side, price, volume);
  if (matchResult.error != OrderbookError::OK || matchResult.status == OrderStatus::Filled) {
    return matchResult;
  }
  // partial fill or resting 
  Order newOrder{side, orderId, traderId, price, matchResult.restingVolume};
  auto [levelIt, inserted] = levels.try_emplace(price);
  auto &priceLevel = levelIt->second;
  priceLevel.total_volume += matchResult.restingVolume;
  auto orderIt = priceLevel.orders.emplace(priceLevel.orders.end(), newOrder);
  idMap.emplace(orderId, orderIt);

  return matchResult;
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
  assert(levelIt != levels.end() && "CRITICAL BUG: Price level not found");

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
  assert(levelIt != levels.end() && "CRITICAL BUG: Price level not found");
  PriceLevel &priceLevel = levelIt->second;

  Volume volumeChange = newVolume - orderIt->volume;

  if (priceLevel.total_volume + volumeChange < 0) {
    return OrderbookError::InvalidVolume; // volume can't drop below 0
  }

  priceLevel.total_volume += volumeChange;
  orderIt->volume = newVolume;
  return OrderbookError::OK;
}

// ----------------------------- HandleFill --------------------------------

[[nodiscard]] MatchResult Orderbook_Map::HandleFill(Side side, Price incomingOrderPrice, Volume incomingOrderVolume) {
  Volume initialVolume = incomingOrderVolume;
  Volume remainingVolume = incomingOrderVolume;
  Volume filledVolume = 0;

  if (side == Side::Bid) {
    auto bestAskPriceOpt = GetBestAsk();
    if (!bestAskPriceOpt.has_value() || *bestAskPriceOpt > incomingOrderPrice) {
      return {OrderStatus::Resting, OrderbookError::OK, 0, initialVolume}; // no crossing liquidity => order rests
    }

    // iterate through askLevels while still have volume to fill
    while (remainingVolume > 0 && !askLevels.empty()) {
      auto bestAskIt = askLevels.begin();
      Price bestAskPrice = bestAskIt->first;

      // did price move and stop crossing
      if (incomingOrderPrice < bestAskPrice) {
        break;
      }

      PriceLevel &askLevel = bestAskIt->second;

      // match against resting orders in price level
      while (remainingVolume > 0 && !askLevel.orders.empty()) {
        auto restingOrderIt = askLevel.orders.begin();
        Order &restingOrder = *restingOrderIt;

        Volume matchQty = std::min(remainingVolume, restingOrder.volume);

        filledVolume += matchQty;
        remainingVolume -= matchQty;

        restingOrder.volume -= matchQty;
        askLevel.total_volume -= matchQty;

        if (restingOrder.volume == 0) {
          // clean up fully filled resting orders
          idMap.erase(restingOrder.id);
          askLevel.orders.erase(restingOrderIt);
        } else {
          // resting order was only partially filled, meaning incoming order was filled
          break;
        }
      }
      if (askLevel.orders.empty()) {
        askLevels.erase(bestAskIt);
      }
    }
  } else {
    auto bestBidPriceOpt = GetBestBid();
    if (!bestBidPriceOpt.has_value() || *bestBidPriceOpt < incomingOrderPrice) {
      return {OrderStatus::Resting, OrderbookError::OK, 0, initialVolume};
    }

    while (remainingVolume > 0 && !bidLevels.empty()) {
      auto bestBidIt = bidLevels.begin();
      Price bestBidPrice = bestBidIt->first;

      if (incomingOrderPrice > bestBidPrice) {
        break;
      }

      PriceLevel &bidLevel = bestBidIt->second;

      while (remainingVolume > 0 && !bidLevel.orders.empty()) {
        auto restingOrderIt = bidLevel.orders.begin();
        Order &restingOrder = *restingOrderIt;

        Volume matchQty = std::min(remainingVolume, restingOrder.volume);

        filledVolume += matchQty;
        remainingVolume -= matchQty;

        restingOrder.volume -= matchQty;
        bidLevel.total_volume -= matchQty;

        if (restingOrder.volume == 0) {
          idMap.erase(restingOrder.id);
          bidLevel.orders.erase(restingOrderIt);
        } else {
          break;
        }
      }
      if (bidLevel.orders.empty()) {
        bidLevels.erase(bestBidIt);
      }
    }
  }

      
  OrderStatus status;
  if (filledVolume == 0) {
    status = OrderStatus::Resting;
  } else if (remainingVolume == 0) {
    status = OrderStatus::Filled;
  } else {
    status = OrderStatus::PartiallyFilled;
  }

  return {status, OrderbookError::OK, filledVolume, remainingVolume};
}
