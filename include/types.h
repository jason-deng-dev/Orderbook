#pragma once
#include <cstdint>
#include <list>

enum class Side : uint8_t { Bid, Ask };
using OrderId = uint64_t;
using Volume = uint32_t;
using Price = int64_t;
using TraderId = uint32_t;

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

enum class EventType : uint8_t { Add, Cancel, Modify, Fill, Reject };

struct alignas(64) OrderEvent {
  // 8-byte types (24 bytes total)
  uint64_t timestamp_ns;
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


