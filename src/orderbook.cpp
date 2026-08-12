#include "../include/orderbook.h"
#include "../include/trader.h"

bool Orderbook::buy(Trader *trader, int quantity, int price) {
  int balance = trader->getBalance();
  if (balance < quantity*price) return false;
  
}

bool Orderbook::sell(Trader *trader, int quantity, int price) {}

bool Orderbook::cancelBuy(Trader *trader, int quantity, int price) {}
bool Orderbook::cancelSell(Trader *trader, int quantity, int price) {}
