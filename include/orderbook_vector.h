#pragma once

#include "search_policies.h"
#include "types.h"
#include <cassert>
#include <cstddef>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

template <typename SearchPolicy>
class Orderbook_Vector {
public:
  [[nodiscard]] MatchResult AddOrder(OrderId orderId, Side side, Price price, Volume volume, TraderId traderId);
  [[nodiscard]] OrderbookError ModifyOrder(OrderId orderId, Volume newVolume);
  [[nodiscard]] OrderbookError DeleteOrder(OrderId orderId);

private:
  template <typename T, typename Compare>
  [[nodiscard]] MatchResult AddOrder(Timestamp ts, T &levels, Side side, OrderId orderId, Price price, Volume volume,
                                     TraderId traderId, Compare comp);

  template <typename T, typename Compare>
  [[nodiscard]] OrderbookError DeleteOrder(Timestamp ts, std::list<Order>::iterator orderIt, T &levels, Compare comp);
  template <typename T, typename Compare>
  [[nodiscard]] OrderbookError ModifyOrder(Timestamp ts, std::list<Order>::iterator orderIt, T &levels,
                                           Volume newVolume, Compare comp);

  template <typename Compare>
  [[nodiscard]] MatchResult HandleFill(Side side, Price incomingOrderPrice, Volume incomingOrderVolume,
                                       TraderId incomingTraderId, Compare comp);

  template <typename Levels, typename Compare>
  [[nodiscard]] size_t FindLevelIndex(const Levels &levels, Price price, Compare comp);

  std::vector<std::pair<Price, PriceLevel>> bidLevels; // best bid at end
  std::vector<std::pair<Price, PriceLevel>> askLevels; // best ask at end
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
    if (bidLevels.empty()) return std::nullopt;
    return bidLevels.back().first;
  }

  [[nodiscard]] std::optional<Price> GetBestAsk() const {
    if (askLevels.empty()) return std::nullopt;
    return askLevels.back().first;
  }

  [[nodiscard]] Volume GetTotalVolumeAtPrice(Price price, Side side) const {
    if (side == Side::Bid) {
      auto [res, levelIt] = SearchPolicy::search(bidLevels.begin(), bidLevels.end(), price, std::less<Price>());
      return res == SearchResult::found ? levelIt->second.total_volume : 0;
    } else {
      auto [res, levelIt] = SearchPolicy::search(askLevels.begin(), askLevels.end(), price, std::greater<Price>());
      return res == SearchResult::found ? levelIt->second.total_volume : 0;
    }
  };

  [[nodiscard]] size_t GetOrderCountAtPrice(Price price, Side side) const {
    if (side == Side::Bid) {
      auto [res, levelIt] = SearchPolicy::search(bidLevels.begin(), bidLevels.end(), price, std::less<Price>());
      return res == SearchResult::found ? levelIt->second.orders.size() : 0;
    } else {
      auto [res, levelIt] = SearchPolicy::search(askLevels.begin(), askLevels.end(), price, std::greater<Price>());
      return res == SearchResult::found ? levelIt->second.orders.size() : 0;
    }
  }

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

// ----------------------------- HandleFill --------------------------------

template <typename SearchPolicy>
template <typename Compare>
[[nodiscard]] MatchResult Orderbook_Vector<SearchPolicy>::HandleFill(Side side, Price incomingOrderPrice,
                                                                     Volume incomingOrderVolume,
                                                                     TraderId incomingTraderId, Compare comp) {
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
    // levels run from worst price to best, so matching starts at the back
    size_t nextLevelIdx = levels.size();
    // while have volume to fill and still have fillable orders
    while (remainingVolume > 0 && nextLevelIdx > 0 && shouldCross(levels[nextLevelIdx - 1].first)) {
      auto &priceLevel = levels[--nextLevelIdx].second;
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
        levels.erase(levels.begin() + nextLevelIdx);
      } else {
        // exit if order is filled
        if (remainingVolume == 0) break;
        // had a self trade occur, just move on to next level
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

// ----------------------------- AddOrder --------------------------------
template <typename SearchPolicy>

[[nodiscard]] MatchResult Orderbook_Vector<SearchPolicy>::AddOrder(OrderId orderId, Side side, Price price,
                                                                   Volume volume, TraderId traderId) {
  Timestamp ts = now_ns();
  if (idMap.contains(orderId)) {
    MatchResult matchResult{.filledVolume = 0,
                            .restingVolume = volume,
                            .status = OrderStatus::Rejected,
                            .error = OrderbookError::DuplicateId};
    eventLog.emplace_back(MakeAddEvent(ts, side, orderId, price, volume, traderId, matchResult));
    return matchResult;
  }

  if (side == Side::Bid) {
    return AddOrder(ts, bidLevels, side, orderId, price, volume, traderId, std::less<Price>());
  } else {
    return AddOrder(ts, askLevels, side, orderId, price, volume, traderId, std::greater<Price>());
  }
}

template <typename SearchPolicy>
template <typename T, typename Compare>
[[nodiscard]] MatchResult Orderbook_Vector<SearchPolicy>::AddOrder(Timestamp ts, T &levels, Side side, OrderId orderId,
                                                                   Price price, Volume volume, TraderId traderId,
                                                                   Compare comp) {
  const MatchResult matchResult = HandleFill(side, price, volume, traderId, comp);
  if (matchResult.error != OrderbookError::OK || matchResult.status == OrderStatus::Filled) {
    eventLog.emplace_back(MakeAddEvent(ts, side, orderId, price, volume, traderId, matchResult));
    return matchResult;
  }
  // partial fill or resting
  Order newOrder{
      .id = orderId, .price = price, .side = side, .trader_id = traderId, .volume = matchResult.restingVolume};

  auto [res, levelIt] = SearchPolicy::search(levels.begin(), levels.end(), price, comp);
  if (res == SearchResult::notFound) {
    levelIt = levels.insert(levelIt, {price, PriceLevel{}});
  }

  auto &priceLevel = levelIt->second;
  priceLevel.total_volume += matchResult.restingVolume;
  auto orderIt = priceLevel.orders.emplace(priceLevel.orders.end(), newOrder);
  idMap.emplace(orderId, orderIt);

  eventLog.emplace_back(MakeAddEvent(ts, side, orderId, price, volume, traderId, matchResult));
  return matchResult;
}

// ----------------------------- DeleteOrder --------------------------------
template <typename SearchPolicy>
[[nodiscard]] OrderbookError Orderbook_Vector<SearchPolicy>::DeleteOrder(OrderId orderId) {
  Timestamp ts = now_ns();
  auto mapIt = idMap.find(orderId);
  if (mapIt == idMap.end()) {
    eventLog.emplace_back(MakeDeleteEvent(ts, orderId, OrderbookError::OrderNotFound));
    return OrderbookError::OrderNotFound;
  }
  auto orderIt = mapIt->second;

  if (orderIt->side == Side::Bid) {
    return DeleteOrder(ts, orderIt, bidLevels, std::less<Price>());
  } else {
    return DeleteOrder(ts, orderIt, askLevels, std::greater<Price>());
  }
};

template <typename SearchPolicy>
template <typename T, typename Compare>
[[nodiscard]] OrderbookError
Orderbook_Vector<SearchPolicy>::DeleteOrder(Timestamp ts, std::list<Order>::iterator orderIt, T &levels, Compare comp) {
  auto [res, levelIt] = SearchPolicy::search(levels.begin(), levels.end(), orderIt->price, comp);

  assert(res != SearchResult::notFound && "CRITICAL BUG: Price level not found");

  PriceLevel &priceLevel = levelIt->second;

  assert(priceLevel.total_volume >= orderIt->volume && "CRITICAL BUG: level volume below order volume");

  priceLevel.total_volume -= orderIt->volume;
  idMap.erase(orderIt->id);
  priceLevel.orders.erase(orderIt);

  if (priceLevel.orders.empty()) {
    levels.erase(levelIt);
  }

  eventLog.emplace_back(MakeDeleteEvent(ts, orderIt->id, OrderbookError::OK));
  return OrderbookError::OK;
}

// ----------------------------- ModifyOrder --------------------------------
template <typename SearchPolicy>
[[nodiscard]] OrderbookError Orderbook_Vector<SearchPolicy>::ModifyOrder(OrderId orderId, Volume newVolume) {
  Timestamp ts = now_ns();
  if (newVolume == 0) {
    return DeleteOrder(orderId);
  }

  auto mapIt = idMap.find(orderId);

  if (mapIt == idMap.end()) {
    eventLog.emplace_back(MakeModifyEvent(ts, orderId, newVolume, OrderbookError::OrderNotFound));
    return OrderbookError::OrderNotFound;
  }
  auto orderIt = mapIt->second;
  if (orderIt->volume <= newVolume) {
    eventLog.emplace_back(MakeModifyEvent(ts, orderId, newVolume, OrderbookError::InvalidVolume));
    return OrderbookError::InvalidVolume; // can't add orders or cancel 0 orders
  }

  if (orderIt->side == Side::Bid) {
    return ModifyOrder(ts, orderIt, bidLevels, newVolume, std::less<Price>());
  } else {
    return ModifyOrder(ts, orderIt, askLevels, newVolume, std::greater<Price>());
  }
}

template <typename SearchPolicy>
template <typename T, typename Compare>
[[nodiscard]] OrderbookError Orderbook_Vector<SearchPolicy>::ModifyOrder(Timestamp ts,
                                                                         std::list<Order>::iterator orderIt, T &levels,
                                                                         Volume newVolume, Compare comp) {
  auto [res, levelIt] = SearchPolicy::search(levels.begin(), levels.end(), orderIt->price, comp);

  assert(res != SearchResult::notFound && "CRITICAL BUG: Price level not found");
  PriceLevel &priceLevel = levelIt->second;

  Volume volumeChange = newVolume - orderIt->volume;

  priceLevel.total_volume += volumeChange;
  orderIt->volume = newVolume;

  eventLog.emplace_back(MakeModifyEvent(ts, orderIt->id, newVolume, OrderbookError::OK));
  return OrderbookError::OK;
}
