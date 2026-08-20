#include "../include/trader.h"
#include "../include/orderbook.h"
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

int Trader::next_id_ = 0;
void displayBookHelper(const Orderbook *orderbook,
                       const std::map<int, std::vector<int>> &orders,
                       const std::string &type) {
  for (auto &[price, vec] : orders) {
    for (auto trade_id : vec) {

      // need to change depending on
      const Order *order = (type == "buy")
                               ? orderbook->getBuyOrder(price, trade_id)
                               : orderbook->getSellOrder(price, trade_id);
      if (!order) {
        std::cerr << "WARNING: trade id:" << trade_id << " does not exist in "
                  << type
                  << "OrderMap, SYNC ERROR between "
                     "trader and orderbook\n";
        return;
      }

      // format timestamp as HH:MM:SS.mmm
      auto t = std::chrono::system_clock::to_time_t(order->ts);
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    order->ts.time_since_epoch()) %
                1000;
      std::tm tm = *std::localtime(&t);
      std::stringstream ss;
      ss << std::put_time(&tm, "%H:%M:%S") << '.' << std::setw(3)
         << std::setfill('0') << ms.count();
      std::string timeStr = ss.str();

      std::string idStr = std::to_string(trade_id);
      std::string pxStr = std::to_string(price);
      std::string qtyStr = std::to_string(order->quantity_);

      std::cout << timeStr << std::string(12 - timeStr.length(), ' ') << "| "
                << idStr << std::string(10 - idStr.length(), ' ') << "| "
                << pxStr << std::string(8 - pxStr.length(), ' ') << "| "
                << qtyStr << '\n';
    }
  }
}

void Trader::displayInventory() {
  std::cout << name_ << " inventory \nbalance : " << balance_ << '\n';
  for (auto &[orderbook, amount] : inventory) {
    std::cout << orderbook->getName() << " : " << amount << '\n';
  }
  std::cout << '\n';
}
void Trader::displayBook(Orderbook *orderbook) {
  std::cout << orderbook->getName() << ": \n\nBuy Orders:\n";
  
  std::cout << std::string(60, '-') << '\n';
  std::cout << "HH:MM:SS.mmm" << "| " << "Trade ID" << std::string(2, ' ')
            << "| " << "Price" << std::string(3, ' ') << "| " << "Qty"
            << std::string(5, ' ') << "\n";
  std::cout << std::string(60, '-') << '\n';

  displayBookHelper(orderbook, buyOrders[orderbook], "buy");

  std::cout << std::string(60, '-') << '\n';
  std::cout << "\n\nSell Orders:\n";
  std::cout << std::string(60, '-') << '\n';
  std::cout << "HH:MM:SS.mmm" << "| " << "Trade ID" << std::string(2, ' ')
            << "| " << "Price" << std::string(3, ' ') << "| " << "Qty"
            << std::string(5, ' ') << "\n";
  std::cout << std::string(60, '-') << '\n';

  displayBookHelper(orderbook, sellOrders[orderbook], "sell");
}
