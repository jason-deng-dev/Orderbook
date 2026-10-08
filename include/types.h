#pragma once
#include <cstdint>
#include <list>

enum class Side { Bid, Ask };
using OrderId = uint64_t;
using Volume = int64_t;
using Price = int64_t;
using TraderId = uint64_t;

struct Order {
  Side side;
  OrderId id;
  TraderId trader_id;
  Price price;
  Volume volume;
};

struct PriceLevel {
  Volume total_volume = 0;
  std::list<Order> orders;
};



enum class OrderbookError { OK, DuplicateId, OrderNotFound, InvalidVolume, InvalidPrice };

enum class OrderStatus {
  Rejected, 
  Resting, 
  PartiallyFilled, 
  Filled
};

struct MatchResult {
  OrderStatus status;
  OrderbookError error;
  Volume filledVolume;
  Volume restingVolume;
};
