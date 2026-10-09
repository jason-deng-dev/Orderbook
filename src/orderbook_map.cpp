#include "orderbook_map.h"
#include "types.h"
#include <algorithm>
#include <cassert>
#include <optional>

// ----------------------------- AddOrder --------------------------------
[[nodiscard]] MatchResult Orderbook_Map::AddOrder(OrderId orderId, Side side, Price price, Volume volume,
                                                  TraderId traderId) {
  Timestamp timestamp_ns = now_ns();

  if (idMap.contains(orderId)) {
    MatchResult matchResult{.filledVolume = 0,
                            .restingVolume = volume,
                            .status = OrderStatus::Rejected,
                            .error = OrderbookError::DuplicateId};
    eventLog.emplace_back(MakeAddEvent(timestamp_ns, side, orderId, price, volume, traderId, matchResult));
    return matchResult;
  }
  if (side == Side::Bid) {
    return AddOrder(timestamp_ns, bidLevels, side, orderId, price, volume, traderId);
  } else {
    return AddOrder(timestamp_ns, askLevels, side, orderId, price, volume, traderId);
  }
};

template <typename T>
[[nodiscard]] MatchResult Orderbook_Map::AddOrder(Timestamp timestamp_ns, T &levels, Side side, OrderId orderId,
                                                  Price price, Volume volume, TraderId traderId) {
  const MatchResult matchResult = HandleFill(side, price, volume, traderId);
  if (matchResult.error != OrderbookError::OK || matchResult.status == OrderStatus::Filled) {
    eventLog.emplace_back(MakeAddEvent(timestamp_ns, side, orderId, price, volume, traderId, matchResult));
    return matchResult;
  }
  // partial fill or resting
  Order newOrder{
      .id = orderId, .price = price, .side = side, .trader_id = traderId, .volume = matchResult.restingVolume};
  auto [levelIt, inserted] = levels.try_emplace(price);
  auto &priceLevel = levelIt->second;
  priceLevel.total_volume += matchResult.restingVolume;
  auto orderIt = priceLevel.orders.emplace(priceLevel.orders.end(), newOrder);
  idMap.emplace(orderId, orderIt);

  eventLog.emplace_back(MakeAddEvent(timestamp_ns, side, orderId, price, volume, traderId, matchResult));
  return matchResult;
}

// ----------------------------- DeleteOrder --------------------------------
[[nodiscard]] OrderbookError Orderbook_Map::DeleteOrder(OrderId orderId) {
  Timestamp timestamp_ns = now_ns();
  auto mapIt = idMap.find(orderId);
  if (mapIt == idMap.end()) {
    eventLog.emplace_back(MakeDeleteEvent(timestamp_ns, orderId, OrderbookError::OrderNotFound));
    return OrderbookError::OrderNotFound;
  }
  auto orderIt = mapIt->second;

  if (orderIt->side == Side::Bid) {
    return DeleteOrder(timestamp_ns, orderIt, bidLevels);
  } else {
    return DeleteOrder(timestamp_ns, orderIt, askLevels);
  }
};

template <typename T>
[[nodiscard]] OrderbookError Orderbook_Map::DeleteOrder(Timestamp timestamp_ns, std::list<Order>::iterator orderIt,
                                                        T &levels) {
  auto levelIt = levels.find(orderIt->price);
  assert(levelIt != levels.end() && "CRITICAL BUG: Price level not found");

  PriceLevel &priceLevel = levelIt->second;

  assert(priceLevel.total_volume >= orderIt->volume && "CRITICAL BUG: level volume below order volume");

  priceLevel.total_volume -= orderIt->volume;
  idMap.erase(orderIt->id);
  priceLevel.orders.erase(orderIt);

  if (priceLevel.orders.empty()) {
    levels.erase(levelIt);
  }

  eventLog.emplace_back(MakeDeleteEvent(timestamp_ns, orderIt->id, OrderbookError::OK));
  return OrderbookError::OK;
}

// ----------------------------- ModifyOrder --------------------------------

[[nodiscard]] OrderbookError Orderbook_Map::ModifyOrder(OrderId orderId, Volume newVolume) {
  Timestamp timestamp_ns = now_ns();
  if (newVolume == 0) {
    return DeleteOrder(orderId);
  }

  auto mapIt = idMap.find(orderId);

  if (mapIt == idMap.end()) {
    eventLog.emplace_back(MakeModifyEvent(timestamp_ns, orderId, newVolume, OrderbookError::OrderNotFound));
    return OrderbookError::OrderNotFound;
  }
  auto orderIt = mapIt->second;
  if (orderIt->volume <= newVolume) {
    eventLog.emplace_back(MakeModifyEvent(timestamp_ns, orderId, newVolume, OrderbookError::InvalidVolume));
    return OrderbookError::InvalidVolume; // can't add orders or cancel 0 orders
  }

  if (orderIt->side == Side::Bid) {
    return ModifyOrder(timestamp_ns, orderIt, bidLevels, newVolume);
  } else {
    return ModifyOrder(timestamp_ns, orderIt, askLevels, newVolume);
  }
}

template <typename T>
[[nodiscard]] OrderbookError Orderbook_Map::ModifyOrder(Timestamp timestamp_ns, std::list<Order>::iterator orderIt,
                                                        T &levels, Volume newVolume) {
  auto levelIt = levels.find(orderIt->price);
  assert(levelIt != levels.end() && "CRITICAL BUG: Price level not found");
  PriceLevel &priceLevel = levelIt->second;

  Volume volumeChange = newVolume - orderIt->volume;

  priceLevel.total_volume += volumeChange;
  orderIt->volume = newVolume;

  eventLog.emplace_back(MakeModifyEvent(timestamp_ns, orderIt->id, newVolume, OrderbookError::OK));
  return OrderbookError::OK;
}

// ----------------------------- HandleFill --------------------------------

[[nodiscard]] MatchResult Orderbook_Map::HandleFill(Side side, Price incomingOrderPrice, Volume incomingOrderVolume,
                                                    TraderId incomingTraderId) {
  Volume remainingVolume = incomingOrderVolume;
  Volume filledVolume = 0;

  auto shouldCross = [&](Price bestPrice) {
    return side == Side::Bid ? (bestPrice <= incomingOrderPrice) : (bestPrice >= incomingOrderPrice);
  };

  auto getBestPrice = [&]() -> std::optional<Price> { return side == Side::Bid ? GetBestAsk() : GetBestBid(); };

  auto bestPriceOpt = getBestPrice();
  if (!bestPriceOpt.has_value() || !shouldCross(*bestPriceOpt)) {
    // no crossing liquidity => order rests
    return {.filledVolume = 0,
            .restingVolume = incomingOrderVolume,
            .status = OrderStatus::Resting,
            .error = OrderbookError::OK};
  }

  auto executeMatching = [&](auto &levels) {
    auto levelIt = levels.begin();
    // while have volume to fill and still have fillable orders
    while (remainingVolume > 0 && levelIt != levels.end() && shouldCross(levelIt->first)) {
      auto &priceLevel = levelIt->second;
      auto restingOrderIt = priceLevel.orders.begin();

      while (remainingVolume > 0 && restingOrderIt != priceLevel.orders.end()) {
        Order &restingOrder = *restingOrderIt;
        // self-trade prevention
        if (restingOrder.trader_id == incomingTraderId) {
          ++restingOrderIt;
          continue;
        }
        // normal matching
        Volume matchQty = std::min(remainingVolume, restingOrder.volume);
        filledVolume += matchQty;
        remainingVolume -= matchQty;

        restingOrder.volume -= matchQty;
        priceLevel.total_volume -= matchQty;

        if (restingOrder.volume == 0) {
          idMap.erase(restingOrder.id);
          restingOrderIt = priceLevel.orders.erase(restingOrderIt);
        } else {
          break; // resting order was paritally filled, meaning incoming order was filled
        }
      }
      if (priceLevel.orders.empty()) {
        levelIt = levels.erase(levelIt);
      } else {
        // exit if order is filled
        if (remainingVolume == 0) break;
        levelIt++; // had a self trade occur, just move on to next level
      }
    }
  };

  if (side == Side::Bid) {
    executeMatching(askLevels);
  } else {
    executeMatching(bidLevels);
  }

  OrderStatus status;
  if (filledVolume == 0) {
    status = OrderStatus::Resting;
  } else if (remainingVolume == 0) {
    status = OrderStatus::Filled;
  } else {
    status = OrderStatus::PartiallyFilled;
  }

  return {
      .filledVolume = filledVolume, .restingVolume = remainingVolume, .status = status, .error = OrderbookError::OK};
}
