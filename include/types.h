#pragma once
#include <chrono>
#include <cstdint>
#include <list>

enum class Side : uint8_t { Bid, Ask };
using OrderId = uint64_t;
using Volume = uint32_t;
using Price = int64_t;
using TraderId = uint32_t;
using Timestamp = uint64_t;

struct Order {
  OrderId id;
  Price price;
  Side side;
  TraderId trader_id;
  Volume volume;
};

struct PriceLevel {
  Volume total_volume = 0;
  std::list<Order> orders;
};

enum class OrderbookError : uint8_t { OK, DuplicateId, OrderNotFound, InvalidVolume, InvalidPrice };

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
  Timestamp timestamp_ns;
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
  return OrderEvent{.timestamp_ns = ts,
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
  return OrderEvent{.timestamp_ns = ts, .orderId = id, .type = EventType::Cancel, .orderbookError = orderbookError};
}


static OrderEvent MakeModifyEvent(Timestamp ts, OrderId id, Volume newVolume, OrderbookError orderbookError) {
  return OrderEvent{.timestamp_ns = ts,
                    .orderId = id,
                    .volume = newVolume,
                    .type = EventType::Modify,
                    .orderbookError = orderbookError};
}
