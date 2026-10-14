# Orderbook — Matching Engine (C++23)

Price-time priority matching engine with self-trade prevention, partial-fill, and
cancellation semantics, written from scratch as a study in container choice, memory
layout, and the measurement discipline behind low-latency claims.

## Quick start

```bash
cmake --preset default -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```
