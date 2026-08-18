#include <iostream>
#include "../include/orders.h"
#include "../include/trader.h"
#include "../include/orderbook.h"




int main() {
  Orderbook ob1 {};

  Trader t1 ("jason", 200);

  ob1.buy(&t1, 10, 10);

  ob1.displayBuyOrders();

  Trader t2 ("Kam", 200);
  ob1.buy(&t2, 5, 2);
  
  ob1.displayBuyOrders();
  return 0;
}
