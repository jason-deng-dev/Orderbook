#include "../include/orders.h"
#include "../include/trader.h"

int Order::currId = 1;

bool BuyOrders::add(Trader *trader, int quantity, int price) {
  if (trader->getBalance() < quantity*price) return false;

  if (!buyOrderMap.count(price)) {
    buyOrderMap[price] = new OrdersAtPrice();
  }
  buyOrderMap[price]->total_quantity += quantity;
  buyOrderMap[price]->total_orders += 1;
  Order *order = new Order(quantity, trader->getId(), price);
  buyOrderMap[price]->order_queue[order->trade_id_] = order;

  // handle trader
  trader.


  return true;
}

bool BuyOrders::cancel(Trader *trader, int quanity, int price) {
  if (!buyOrderMap.count(price))
}
