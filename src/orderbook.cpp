#include "../include/orderbook.h"
#include "../include/trader.h"

bool Orderbook::buy(Trader *trader, int quantity, int price) {
  if (buyOrders_.add(trader, quantity, price)) return true;
  return false;
}

bool Orderbook::cancelBuy(Trader *trader, int quantity, int price) {
  if (buyOrders_.cancel(trader, quantity, price)) return true;
  return false; 
}


bool Orderbook::sell(Trader *trader, int quantity, int price) {
}



bool Orderbook::cancelSell(Trader *trader, int quantity, int price) {}
