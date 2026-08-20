#include "../include/orderbook.h"
#include "../include/orders.h"
#include "../include/trader.h"
#include <iostream>

int main() {
  Orderbook ob1{"Stock1"};

  Trader t1("jason", 200);
  ob1.displayOrders();
  ob1.buy(&t1, 10, 10);

  ob1.displayOrders();
  Trader t2("Kam", 200);
  ob1.buy(&t2, 5, 2);
  ob1.buy(&t2, 5, 10);

  ob1.displayOrders();
  t1.changeInventoryAmount(&ob1, 20);
  ob1.sell(&t1, 20, 20);

  ob1.displayOrders();

  t1.displayInventory();
  t2.displayInventory();

  t1.displayBook(&ob1);

  ob1.cancelBuy(&t1, 0, 10, 5);
  ob1.cancelSell(&t1, 3, 20, 5);

  t1.displayBook(&ob1);

  return 0;
}
