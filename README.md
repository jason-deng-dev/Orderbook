# Low-Latency Order Matching Engine & Market Data Replay System (C++23)


**Feed Handler**
[feed_handler.h](include/feed_handler.h) / [feed_handler.md](docs/feed_handler.md)
- ITCH 5.0 parser over an mmap'ed file
- replays `07302019.NASDAQ_ITCH50` into `QueueItem` messages
- optional stock-locate filter, `PushToQueue` into an SPSC FIFO

**Matching Engine**
[orderbook.md](docs/orderbook.md)
- price-time priority engine with self-trade prevention and partial-fill semantics
- 3 implementations
  - `std::map` based price levels implementation [orderbook_map.h](include/orderbook_map.h)
  - `std::vector` based price levels implementation [orderbook_vector.h](include/orderbook_vector.h)
  - Slab-allocated intrusive node price level implementation [orderbook_slab.h](include/orderbook_slab.h)

**Slab Allocator**
[slab_allocator.h](include/slab_allocator.h)
- fixed-size slots carved from one contiguous pool
- free-list allocate/deallocate, no per-order `malloc`

**SPSC FIFO**
[spsc_fifo.h](include/spsc_fifo.h), [spsc_fifo_cached.h](include/spsc_fifo_cached.h), [fifo_helper.h](include/fifo_helper.h), [spsc.md](docs/spsc.md)
- lock-free single-producer single-consumer ring
- cached-cursor variant that removes an atomic load per slot
- used as the feed handler to book builder handoff

**Search Policies**
[search_policies.h](include/search_policies.h)
- linear, binary, and branchless binary price-level search

**Types**
[types.h](include/types.h)
- `Order`, `Order_Intrusive`, `PriceLevel`, `PriceLevel_Intrusive`
- `MatchResult`, `OrderbookError`, `OrderStatus`, `QueueItem`

****


# Testing
```bash
cmake --preset default
cmake --build build
ctest --test-dir build --output-on-failure
```
- 6 test files, 94 tests
- [orderbook_test.cc](tests/orderbook_test.cc) - Map / Vector / Slab share one conformance suite
- [pricelevel_intrusive_test.cc](tests/pricelevel_intrusive_test.cc) - intrusive list links, head/tail/middle deletion
- [slab_allocator_test.cc](tests/slab_allocator_test.cc)
- [spsc_fifo_test.cc](tests/spsc_fifo_test.cc)
- [search_policies_test.cc](tests/search_policies_test.cc)
- [feed_handler_test.cc](tests/feed_handler_test.cc)
- all tests: [tests/](tests/)


# Benchmark
Benchmark specifics are located in [benchmark.md](docs/benchmark.md)

```bash
cmake --preset default -DCMAKE_BUILD_TYPE=Release
cmake --build build
taskset -c 2,4 ./build/benchmarks --benchmark_filter=BM_Orderbook --benchmark_repetitions=500 --benchmark_report_aggregates_only=true
```

3 benchmarks implemented
- [bm_orderbook.cpp](benchmarks/bm_orderbook.cpp) - Map / Vector / Slab throughput over the ITCH replay
- [bm_fifo.cpp](benchmarks/bm_fifo.cpp) - SPSC FIFO throughput
- [bm_feedHandler.cpp](benchmarks/bm_feedHandler.cpp) - ITCH parser throughput

Results and analysis: [optimization_log.md](docs/optimization_log.md)


# Sources:
- [David Gross - When Nanoseconds Matter: Ultrafast Trading Systems in C++ (CppCon 2024)](https://www.youtube.com/watch?v=sX2nF1fW7kI&t=4499s) - orderbook layout and hot-path design notes, see [orderbook_notes.md](docs/notes/orderbook_notes.md)
- [Charles Frasch - Single Producer Single Consumer Lock-free FIFO From the Ground Up (CppCon 2023)](https://www.youtube.com/watch?v=K3P_Lmq6pw0&t=650s) - SPSC ring design, see [spsc_fifo_notes.md](docs/notes/spsc_fifo_notes.md)
- [Kernel Exploitation - Slab Allocators](https://www.youtube.com/watch?v=4UucTCiQLXE) - slab and free-list design, see [slab_allocator_notes.md](docs/notes/slab_allocator_notes.md)
- [Kevin Carpenter - Custom Allocators (CppCon 2025)](https://www.youtube.com/watch?v=RpD-0oqGEzE) - allocator takeaways, see [custom_allocator_notes.md](docs/notes/custom_allocator_notes.md)
- [understanding mmap](https://www.youtube.com/watch?v=8hVLcyBkSXY), [mmap(2)](https://man7.org/linux/man-pages/man2/mmap.2.html), [madvise(2)](https://man7.org/linux/man-pages/man2/madvise.2.html) - feed handler file mapping, see [mmap_notes.md](docs/notes/mmap_notes.md)
- [Mathieu Ropert - The Basics of Profiling (CppCon 2021)](https://www.youtube.com/watch?v=dToaepIXW4s&t=107s) - profiling and benchmark methodology, see [profiling_notes.md](docs/notes/profiling_notes.md)
- [Concurrency Performance Testing and Benchmarking](https://awesome-embedded-learning-studio.github.io/Tutorial_AwesomeModernCPP/en/vol5-concurrency/ch08-debug-testing-perf/02-concurrency-benchmarks) - Google Benchmark setup, see [benchmark_notes.md](docs/notes/benchmark_notes.md)
- [Nasdaq ITCH 5.0 specification](https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHspecification.pdf), [sample data](https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/) - message formats and replay fixture, see [feed_handler.md](docs/feed_handler.md)
- [Agner Fog - Instruction tables](https://www.agner.org/optimize/instruction_tables.pdf) - latency and throughput references, see [spsc.md](docs/spsc.md)
