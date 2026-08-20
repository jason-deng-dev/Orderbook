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
  ob1.buy(&t2, 5, 10);
  
  ob1.displayBuyOrders();

  t1.addStock(&ob1, 20);
  ob1.displaySellOrders();
  ob1.sell(&t1, 20, 50);
  ob1.displaySellOrders();
  
  ob1.displayOrders();

  return 0;
}
