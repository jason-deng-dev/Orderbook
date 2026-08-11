#ifndef TRADER_H
#define TRADER_H

#include <string>
#include <string_view>
#include "orderbook.h"

class Trader {

private:
  static int next_id_;
  int id_;
  std::string name_;
  int balance_;

public:
  Trader(const std::string &name, double balance = 0.0)
      : id_(next_id_++), name_(name), balance_(balance) {}

  int getId() const { return id_; }
  int getBalance() const {return balance_;}
  std::string_view getName() const {return name_;}
};

#endif
