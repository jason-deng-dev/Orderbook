cmake_minimum_required(VERSION 4.3)
project(Orderbook)
 
set(CMAKE_CXX_STANDARD 23)
# make standard required
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

add_executable(orderbook
  src/main.cpp
  src/orderbook.cpp
  src/orders.cpp
  src/trader.cpp
)
