cmake_minimum_required(VERSION 4.3)
project(Orderbook)
 
set(CMAKE_CXX_STANDARD 23)
# make standard required
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

add_library(orderbook_core 
  src/orderbook.cpp
  src/orders.cpp
  src/trader.cpp
)
# tells CMake compile every source file in orderbook_core with include/ added to compiler's search path
target_include_directories(orderbook_core PUBLIC include)

add_executable(orderbook
  src/main.cpp
)

# provides extra directories to search for libraries
target_link_libraries(orderbook PRIVATE orderbook_core)

enable_testing()

# add testing
add_executable(orderbook_test
  tests/orderbook_test.cc
)

target_link_libraries(orderbook_test
  GTest::gtest_main
  orderbook_core
)

include(GoogleTest)
gtest_discover_tests(orderbook_test orders_test trader_test)

