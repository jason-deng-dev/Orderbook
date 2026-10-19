#pragma once
#include "search_policies.h"
#include "slab_allocator.h"
#include "types.h"
#include <cstddef>
#include <unordered_map>

struct Order_Intrusive {
  OrderId id;
  Price price;
  Side side;
  TraderId trader_id;
  Volume volume;
  Order_Intrusive *prev;
  Order_Intrusive *next;
};

struct PriceLevel_Intrusive {
  Volume total_volume = 0;
  Order_Intrusive *head{nullptr};
  Order_Intrusive *tail{nullptr};
  size_t order_count = 0;

  void addOrder(Order_Intrusive *newOrderLocation, OrderId orderId, Price price, Side side, TraderId traderId,
                Volume volume) {
    ++order_count;
    total_volume += volume;
    if (head == nullptr && tail == nullptr) {
      new (newOrderLocation) Order_Intrusive{orderId, price, side, traderId, volume, nullptr, nullptr};
      head = newOrderLocation;
      tail = newOrderLocation;
      return;
    } else {
      new (newOrderLocation) Order_Intrusive{orderId, price, side, traderId, volume, tail, nullptr};
      tail->next = newOrderLocation;
      tail = newOrderLocation;
      return;
    }
  }
};

template <typename SearchPolicy, int SlabSize>
class Orderbook_Slab {

public:
  Orderbook_Slab() { eventLog.reserve(1'000'000); }
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

private:
  std::vector<std::pair<Price, PriceLevel_Intrusive>> bidLevels;
  std::vector<std::pair<Price, PriceLevel_Intrusive>> askLevels;
  std::vector<OrderEvent> eventLog;
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
      orderCount += priceLevel.order_count;
    }
    for (auto &[price, priceLevel] : askLevels) {
      orderCount += priceLevel.order_count;
    }
    return orderCount;
  };

  // event log
  [[nodiscard]] const std::vector<OrderEvent> &GetEventLog() const { return eventLog; }
};

// ----------------------------- AddOrder --------------------------------
template <typename SearchPolicy, int SlabSize>

[[nodiscard]] MatchResult Orderbook_Slab<SearchPolicy, SlabSize>::AddOrder(OrderId orderId, Side side, Price price,
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

template <typename SearchPolicy, int SlabSize>
template <typename T, typename Compare>
[[nodiscard]] MatchResult Orderbook_Slab<SearchPolicy, SlabSize>::AddOrder(Timestamp ts, T &levels, Side side,
                                                                           OrderId orderId, Price price, Volume volume,
                                                                           TraderId traderId, Compare comp) {

  auto newOrderLocation = allocator.allocate();
  // allocation failure
  if (newOrderLocation == nullptr) {
    MatchResult matchResult{.filledVolume = 0,
                            .restingVolume = volume,
                            .status = OrderStatus::Rejected,
                            .error = OrderbookError::AllocationFailure};
    eventLog.emplace_back(MakeAddEvent(ts, side, orderId, price, volume, traderId, matchResult));
    return matchResult;
  }

  const MatchResult matchResult = HandleFill(side, price, volume, traderId, comp);
  if (matchResult.error != OrderbookError::OK || matchResult.status == OrderStatus::Filled) {
    eventLog.emplace_back(MakeAddEvent(ts, side, orderId, price, volume, traderId, matchResult));
    return matchResult;
  }

  auto [res, levelIt] = SearchPolicy::search(levels.begin(), levels.end(), price, comp);
  if (res == SearchResult::notFound) {
    levelIt = levels.insert(levelIt, {price, PriceLevel_Intrusive{}});
  }


  auto &priceLevel = levelIt->second;

  priceLevel.addOrder(newOrderLocation, orderId, price, side, traderId, volume);
  idMap.emplace(orderId, newOrderLocation);

  eventLog.emplace_back(MakeAddEvent(ts, side, orderId, price, volume, traderId, matchResult));
  return matchResult;
}
