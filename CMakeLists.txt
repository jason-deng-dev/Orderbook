cmake_minimum_required(VERSION 4.3)
project(Orderbook)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Link-time optimization for all Release targets.
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported LANGUAGES CXX)
if(ipo_supported)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

add_library(orderbook_core
  src/orderbook_map.cpp
  src/feed_handler.cpp
)

# tells CMake compile every source file in orderbook_core with include/ added to compiler's search path
target_include_directories(orderbook_core PUBLIC include)

# Match benchmark codegen so map and vector are not compared across different ISA flags.
target_compile_options(orderbook_core PRIVATE
  $<$<CONFIG:Release>:-march=native -mtune=native>
)

add_executable(orderbook
  src/main.cpp
)

# provides extra directories to search for libraries
target_link_libraries(orderbook PRIVATE orderbook_core)

enable_testing()

# add testing
add_executable(orderbook_test
  tests/orderbook_map_test.cc
  tests/search_policies_test.cc
  tests/orderbook_vector_test.cc
  tests/spsc_fifo_test.cc
  tests/feed_handler_test.cc
  tests/slab_allocator_test.cc
  tests/orderbook_slab_test.cc
)

target_link_libraries(orderbook_test PRIVATE
  GTest::gtest_main
  orderbook_core
)

include(GoogleTest)
gtest_discover_tests(orderbook_test)



# setting up google benchmark

# configures google benchmark to skip its own tests and docs
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.5
)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(benchmarks
  benchmarks/main.cpp
  benchmarks/bm_fifo.cpp
  benchmarks/bm_feedHandler.cpp
  benchmarks/bm_orderbook.cpp
)
target_link_libraries(benchmarks PRIVATE
  benchmark::benchmark
  benchmark::benchmark_main
  orderbook_core
)

target_compile_definitions(orderbook PRIVATE
  FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)

target_compile_definitions(orderbook_test PRIVATE
  FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)

target_compile_definitions(benchmarks PRIVATE
  FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)

target_compile_options(benchmarks PRIVATE
  $<$<CONFIG:Release>:-O3 -march=native -mtune=native>
)

