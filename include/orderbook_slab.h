#pragma once
#include "types.h"
#include <cstddef>


template <typename SearchPolicy>
class Orderbook_Slab {
public:
  Orderbook_Slab() { eventLog.reserve(1'000'000); }

private:
  std::vector<OrderEvent> eventLog;
};
