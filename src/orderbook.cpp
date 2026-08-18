#include "../include/orderbook.h"
#include "../include/trader.h"

bool Orderbook::buy(Trader *trader, int quantity, int price) {
  if (trader->getBalance() < quantity*price) return false;
  buyOrderMap[price].total_quantity += quantity;
  int trade_id = currTradeId++;
  buyOrderMap[price].order_queue[trade_id] = Order(currTradeId++, quantity, trader->getId(), price);
    
  trader->addBuyOrder(this, buyOrderMap[price].order_queue[trade_id]);


  return true;
}

bool Orderbook::cancelBuy(Trader *trader, int quantity, int price) {
  if (buyOrders_.cancel(trader, quantity, price)) return true;
  return false; 
}


bool Orderbook::sell(Trader *trader, int quantity, int price) {
}



bool Orderbook::cancelSell(Trader *trader, int quantity, int price) {}
