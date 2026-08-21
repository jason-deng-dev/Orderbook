#include "../include/orderbook.h"
#include "../include/orders.h"
#include "../include/trader.h"
#include <iostream>

int main() {
  Orderbook ob1{"Stock1"};

  Trader t1("jason", 200);
  Trader t2("Kam", 200);
  t1.changeInventoryAmount(&ob1, 20);



  ob1.displayOrders();




  ob1.buy(&t2, 10, 10);
  std::cout<< "\n\nInventory before: \n";
  t1.displayInventory();
  t2.displayInventory();



  ob1.displayOrders();
  ob1.sell(&t1, 10, 10);

  ob1.displayOrders();

  std::cout<< "\n\nInventory after: \n";
  t1.displayInventory();
  t2.displayInventory();



  return 0;
}
