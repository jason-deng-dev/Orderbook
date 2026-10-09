#pragma once

#include "types.h"

class Orderbook_Vector {
public:
  [[nodiscard]] MatchResult AddOrder(OrderId orderId, Side side, Price price, Volume volume, TraderId traderId);
  [[nodiscard]] OrderbookError ModifyOrder(OrderId orderId, Volume newVolume);
  [[nodiscard]] OrderbookError DeleteOrder(OrderId orderId);

private:
  std::vector<OrderEvent> eventLog;
};
