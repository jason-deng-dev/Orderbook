#pragma once
#include "slab_allocator.h"
#include "types.h"
#include <cstddef>
#include <unordered_map>
#include "search_policies.h"

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
  // template <typename T, typename>

private:
  std::vector<std::pair<Price, PriceLevel_Intrusive>> bidLevels;
  std::vector<std::pair<Price, PriceLevel_Intrusive>> askLevels;
  std::vector<OrderEvent> eventLog;
  SlabAllocator<Order_Intrusive, SlabSize> alloc;
  std::unordered_map<OrderId, Order_Intrusive*> idMap;

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
