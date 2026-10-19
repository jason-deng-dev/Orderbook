#pragma once
#include <chrono>
#include <cstdint>
#include <limits>
#include <list>

enum class Side : uint8_t { Bid, Ask };
using OrderId = uint64_t;
using Volume = uint32_t;
using Price = int64_t;
using TraderId = uint32_t;
inline constexpr TraderId kUnknownTrader = std::numeric_limits<TraderId>::max();
using Timestamp = uint64_t;

struct Order {
  OrderId id;
  Price price;
  Side side;
  TraderId trader_id;
  Volume volume;
};

struct PriceLevel {
  std::list<Order> orders;

  Volume totalVolume() const {
    Volume res;
    for (const auto& o : orders) {
      res += o.volume;
    }
    return res;
  }
};
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

  bool empty() { return head == nullptr && tail == nullptr && order_count == 0 && total_volume == 0; }

  // toDelete must be an Order_Intrusive inside this priceLevel
  Order_Intrusive* erase (Order_Intrusive* toDelete) {
    if (toDelete == head && toDelete == tail) {

    }
  }

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

enum class OrderbookError : uint8_t {
  OK,
  DuplicateId,
  OrderNotFound,
  InvalidVolume,
  InvalidPrice,
  InvalidOrder,
  AllocationFailure
};

enum class OrderStatus : uint8_t { Rejected, Resting, PartiallyFilled, Filled };

struct MatchResult {
  Volume filledVolume;
  Volume restingVolume;
  OrderStatus status;
  OrderbookError error;
};

enum class EventType : uint8_t { Add, Cancel, Modify };

struct alignas(64) OrderEvent {
  // 8-byte types (24 bytes total)
  Timestamp ts;
  OrderId orderId;
  Price price;

  // 4-byte types (16 bytes total)
  TraderId traderId;
  Volume volume;
  Volume filledVolume;
  Volume restingVolume;

  // 1-byte types (4 bytes total)
  EventType type;
  Side side;
  OrderbookError orderbookError;
  OrderStatus orderStatus;

  // Raw size 44 bytes, Compiler pads to 48, alignas(64) pads to 64
};

inline int64_t now_ns() { return std::chrono::steady_clock::now().time_since_epoch().count(); }

static OrderEvent MakeAddEvent(Timestamp ts, Side side, OrderId id, Price price, Volume volume, TraderId traderId,
                               MatchResult matchResult) {
  return OrderEvent{.ts = ts,
                    .orderId = id,
                    .price = price,
                    .traderId = traderId,
                    .volume = volume,
                    .filledVolume = matchResult.filledVolume,
                    .restingVolume = matchResult.restingVolume,
                    .type = EventType::Add,
                    .side = side,
                    .orderbookError = matchResult.error,
                    .orderStatus = matchResult.status};
}

static OrderEvent MakeDeleteEvent(Timestamp ts, OrderId id, OrderbookError orderbookError) {
  return OrderEvent{.ts = ts, .orderId = id, .type = EventType::Cancel, .orderbookError = orderbookError};
}

static OrderEvent MakeModifyEvent(Timestamp ts, OrderId id, Volume newVolume, OrderbookError orderbookError) {
  return OrderEvent{
      .ts = ts, .orderId = id, .volume = newVolume, .type = EventType::Modify, .orderbookError = orderbookError};
}

enum class OrderType : uint8_t { Add, Cancel, Delete, Replace };

inline const char *toString(OrderType t) {
  switch (t) {
    {
    case OrderType::Add:
      return "Add";
    case OrderType::Cancel:
      return "Cancel";
    case OrderType::Delete:
      return "Delete";
    case OrderType::Replace:
      return "Replace";
    }
  }
  return "Unknown";
}

struct alignas(32) QueueItem {
  // 8-byte types (24 total)
  OrderId orderId;
  OrderId removeId;
  Price price;

  // 4-byte types (4 total)
  Volume volume;

  // 1-byte types (2 total)
  OrderType orderType;
  Side side;
};
