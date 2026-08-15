#include <iostream>
#include "../include/orders.h"
#include "../include/trader.h"
#include "../include/orderbook.h"




int main() {

  Order o1(5);
  Order o2(2);
  Order o3( 52);

  std::cout << o1.trader_id_ << '-' << o2.trader_id_ << '-' <<o3.trader_id_ ;
  return 0;
}
