#pragma once
#include "search_policies.h"
#include "slab_allocator.h"
#include "types.h"
#include <cstddef>
#include <unordered_map>
#include <vector>

template <typename SearchPolicy, int SlabSize>
class Orderbook_Slab {

public:
  [[nodiscard]] MatchResult AddOrder(OrderId orderId, Side side, Price price, Volume volume, TraderId traderId);
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
  template <typename T, typename Compare>
  [[nodiscard]] MatchResult AddOrder(T &levels, Side side, OrderId orderId, Price price, Volume volume,
                                     TraderId traderId, Compare comp);

  template <typename T, typename Compare>
  [[nodiscard]] OrderbookError DeleteOrder(Order_Intrusive *orderPtr, T &levels, Compare comp);
  template <typename T, typename Compare>
  [[nodiscard]] OrderbookError ModifyOrder(Order_Intrusive *orderPtr, T &levels, Volume newVolume, Compare comp);

  template <typename Compare>
  [[nodiscard]] MatchResult HandleFill(Side side, Price incomingOrderPrice, Volume incomingOrderVolume,
                                       TraderId incomingTraderId, Compare comp);

private:
  std::vector<std::pair<Price, PriceLevel_Intrusive>> bidLevels;
  std::vector<std::pair<Price, PriceLevel_Intrusive>> askLevels;
  SlabAllocator<Order_Intrusive, SlabSize> allocator;
  std::unordered_map<OrderId, Order_Intrusive *> idMap;

public:
  // --- getters ---
  // idMap
  [[nodiscard]] bool HasOrder(OrderId orderId) const { return idMap.contains(orderId); }

  [[nodiscard]] std::optional<Order_Intrusive> GetOrder(OrderId orderId) const {
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
      return res == SearchResult::found ? levelIt->second.totalVolume() : 0;
    } else {
      auto [res, levelIt] = SearchPolicy::search(askLevels.begin(), askLevels.end(), price, std::greater<Price>());
      return res == SearchResult::found ? levelIt->second.totalVolume() : 0;
    }
  };

  [[nodiscard]] size_t GetOrderCountAtPrice(Price price, Side side) const {
    if (side == Side::Bid) {
      auto [res, levelIt] = SearchPolicy::search(bidLevels.begin(), bidLevels.end(), price, std::less<Price>());
      return res == SearchResult::found ? levelIt->second.order_count : 0;
    } else {
      auto [res, levelIt] = SearchPolicy::search(askLevels.begin(), askLevels.end(), price, std::greater<Price>());
      return res == SearchResult::found ? levelIt->second.order_count : 0;
    }
  }

  // overall book state
  [[nodiscard]] size_t GetTotalOrderCount() const {
    size_t orderCount = 0;
    for (auto &[price, priceLevel] : bidLevels) {
      orderCount += priceLevel.order_count;
    }
    for (auto &[price, priceLevel] : askLevels) {
      orderCount += priceLevel.order_count;
    }
    return orderCount;
  };
};

// ----------------------------- AddOrder --------------------------------
template <typename SearchPolicy, int SlabSize>

[[nodiscard]] MatchResult Orderbook_Slab<SearchPolicy, SlabSize>::AddOrder(OrderId orderId, Side side, Price price,
                                                                           Volume volume, TraderId traderId) {
  if (idMap.contains(orderId)) {
    MatchResult matchResult{.filledVolume = 0,
                            .restingVolume = volume,
                            .status = OrderStatus::Rejected,
                            .error = OrderbookError::DuplicateId};
    return matchResult;
  }

  if (side == Side::Bid) {
    return AddOrder(bidLevels, side, orderId, price, volume, traderId, std::less<Price>());
  } else {
    return AddOrder(askLevels, side, orderId, price, volume, traderId, std::greater<Price>());
  }
}

template <typename SearchPolicy, int SlabSize>
template <typename T, typename Compare>
[[nodiscard]] MatchResult Orderbook_Slab<SearchPolicy, SlabSize>::AddOrder(T &levels, Side side, OrderId orderId,
                                                                           Price price, Volume volume,
                                                                           TraderId traderId, Compare comp) {

  auto newOrderLocation = allocator.allocate();
  // allocation failure
  if (newOrderLocation == nullptr) {
    MatchResult matchResult{.filledVolume = 0,
                            .restingVolume = volume,
                            .status = OrderStatus::Rejected,
                            .error = OrderbookError::AllocationFailure};
    return matchResult;
  }

  const MatchResult matchResult = HandleFill(side, price, volume, traderId, comp);
  if (matchResult.error != OrderbookError::OK || matchResult.status == OrderStatus::Filled) {
    // incoming order never rests => return its slab slot
    allocator.deallocate(newOrderLocation);
    return matchResult;
  }

  auto [res, levelIt] = SearchPolicy::search(levels.begin(), levels.end(), price, comp);
  if (res == SearchResult::notFound) {
    levelIt = levels.insert(levelIt, {price, PriceLevel_Intrusive{}});
  }

  auto &priceLevel = levelIt->second;

  priceLevel.addOrder(newOrderLocation, orderId, price, side, traderId, matchResult.restingVolume);
  idMap.emplace(orderId, newOrderLocation);

  return matchResult;
}

// ----------------------------- HandleFill --------------------------------

template <typename SearchPolicy, int SlabSize>
template <typename Compare>
[[nodiscard]] MatchResult Orderbook_Slab<SearchPolicy, SlabSize>::HandleFill(Side side, Price incomingOrderPrice,
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

      auto restingOrderPtr = priceLevel.head;

      while (remainingVolume > 0 && restingOrderPtr != nullptr) {
        Order_Intrusive &restingOrder = *restingOrderPtr;
        // self-trade prevention
        if (restingOrder.trader_id != kUnknownTrader && restingOrder.trader_id == incomingTraderId) {
          restingOrderPtr = restingOrderPtr->next;
          continue;
        }
        // normal matching
        Volume matchQty = std::min(remainingVolume, restingOrder.volume);
        filledVolume += matchQty;
        remainingVolume -= matchQty;

        restingOrder.volume -= matchQty;

        if (restingOrder.volume == 0) {
          // capture the next link before deleteOrder recycles this node
          Order_Intrusive *nextOrderPtr = restingOrderPtr->next;
          idMap.erase(restingOrder.id);
          priceLevel.deleteOrder(restingOrderPtr, allocator);
          restingOrderPtr = nextOrderPtr;
        } else {
          break; // resting order was paritally filled, meaning incoming order was filled
        }
      }
      if (priceLevel.empty()) {
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

// ----------------------------- DeleteOrder --------------------------------
template <typename SearchPolicy, int SlabSize>
[[nodiscard]] OrderbookError Orderbook_Slab<SearchPolicy, SlabSize>::DeleteOrder(OrderId orderId) {
  auto mapIt = idMap.find(orderId);
  if (mapIt == idMap.end()) {
    return OrderbookError::OrderNotFound;
  }
  auto orderPtr = mapIt->second;

  if (orderPtr->side == Side::Bid) {
    return DeleteOrder(orderPtr, bidLevels, std::less<Price>());
  } else {
    return DeleteOrder(orderPtr, askLevels, std::greater<Price>());
  }
};

template <typename SearchPolicy, int SlabSize>
template <typename T, typename Compare>
[[nodiscard]] OrderbookError
Orderbook_Slab<SearchPolicy, SlabSize>::DeleteOrder(Order_Intrusive *orderPtr, T &levels, Compare comp) {
  auto [res, levelIt] = SearchPolicy::search(levels.begin(), levels.end(), orderPtr->price, comp);

  assert(res != SearchResult::notFound && "CRITICAL BUG: Price level not found");

  auto &priceLevel = levelIt->second;

  idMap.erase(orderPtr->id);
  priceLevel.deleteOrder(orderPtr, allocator);

  if (priceLevel.empty()) {
    levels.erase(levelIt);
  }
  return OrderbookError::OK;
}

// ----------------------------- ModifyOrder --------------------------------
template <typename SearchPolicy, int SlabSize>
[[nodiscard]] OrderbookError Orderbook_Slab<SearchPolicy, SlabSize>::ModifyOrder(OrderId orderId, Volume newVolume) {
  if (newVolume == 0) {
    return DeleteOrder(orderId);
  }

  auto mapIt = idMap.find(orderId);

  if (mapIt == idMap.end()) {
    return OrderbookError::OrderNotFound;
  }
  auto orderPtr = mapIt->second;
  if (orderPtr->volume <= newVolume) {
    return OrderbookError::InvalidVolume; // can't add orders or cancel 0 orders
  }

  if (orderPtr->side == Side::Bid) {
    return ModifyOrder(orderPtr, bidLevels, newVolume, std::less<Price>());
  } else {
    return ModifyOrder(orderPtr, askLevels, newVolume, std::greater<Price>());
  }
}

template <typename SearchPolicy, int SlabSize>
template <typename T, typename Compare>
[[nodiscard]] OrderbookError Orderbook_Slab<SearchPolicy, SlabSize>::ModifyOrder(Order_Intrusive *orderPtr, T &levels, Volume newVolume,
                                                                         Compare comp) {
  auto [res, levelIt] = SearchPolicy::search(levels.begin(), levels.end(), orderPtr->price, comp);

  assert(res != SearchResult::notFound && "CRITICAL BUG: Price level not found");
  PriceLevel_Intrusive &priceLevel = levelIt->second;

  Volume volumeChange = newVolume - orderPtr->volume;

  orderPtr->volume = newVolume;

  return OrderbookError::OK;
}
