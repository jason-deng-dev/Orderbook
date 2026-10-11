# Orderbook — Matching Engine (C++23)

Price-time priority matching engine with self-trade prevention, partial-fill, and
cancellation semantics, written from scratch as a study in container choice, memory
layout, and the measurement discipline behind low-latency claims.

## Highlights

- **Low-Latency Core:** Reverse-ordered contiguous price levels, best price at the back so book updates never shift the container, with the search policy as a compile-time template parameter — linear, binary, and branchless binary.
- **Correctness:** 123 tests passing, including differential search-policy tests run against a reference implementation. Whole-book validation extends the same method: feed one deterministic generated flow through every backend and assert identical book-state hashes.
- **Measurement:** Performance rows publish only behind the gates in [Results](#results). Pooled allocation is verified by interposing `malloc`/`free` and `operator new`/`delete`, never by inspection.

## Status

| Component | State |
| --- | --- |
| `Orderbook_Map` — node-based levels | built |
| `Orderbook_Vector<LinearSearch / BinarySearch / BranchlessBinarySearch>` | built |
| Test suite incl. search-policy differential tests against a reference | built — 123 passing |
| NASDAQ ITCH 5.0 feed handler | next |
| Benchmark matrix over the NASDAQ feed — 3 policies, `perf stat`, alloc interposition | next |
| `Orderbook_Pool` — slab arena, intrusive FIFO, zero hot-path allocation | last, gated on the matrix |

Built and planned are listed separately on purpose. A latency claim is worth exactly
what the harness behind it can reproduce, so the numbers below stay empty until the
harness exists to produce them.

## Quick start

```bash
cmake --preset default -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

## Design

| Backend | Price levels | Orders within a level | Search |
| --- | --- | --- | --- |
| `Orderbook_Map` | `std::map<Price, PriceLevel>` | `std::list<Order>` | tree lookup |
| `Orderbook_Vector<Policy>` | reverse-ordered `std::vector`, best price at the back | `std::list<Order>` | `Policy::search` |
| `Orderbook_Pool<Policy>` | reverse-ordered `std::vector` | intrusive FIFO over a slab arena | `Policy::search` |

Keeping the best price at the *back* of the vector is deliberate: it keeps inserts and
erases near the end, which is where a matching engine actually operates. Putting the
best price first makes every book update shift the whole container.

## Results

Pending the workload harness. The matrix below is the shape the numbers will take, per
operation type rather than aggregated, because the aggregate hides exactly the
bimodality worth looking at:

| Backend | Policy | p50 | p99 | p99.9 | p99.99 | LLC-misses/op | dTLB-misses/op | allocs/op |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `Orderbook_Map` | — | | | | | | | |
| `Orderbook_Vector` | Linear | | | | | | | |
| `Orderbook_Vector` | Binary | | | | | | | |
| `Orderbook_Vector` | Branchless | | | | | | | |
| `Orderbook_Pool` | Linear | | | | | | | |

Gates that must hold before any row is published:

- **Correctness** — every backend produces an identical final book-state hash on the
  same capture, with a bit-identical fill sequence.
- **Zero allocation** — no hot-path allocation after warmup, verified by interposing
  `malloc` / `free` / `realloc` and `operator new` / `operator delete`.
- **Flat rows stay in the table.** If an optimization does not move the counters, the
  row says so with its number. A null with a mechanism is a result; deleting it is how
  a table becomes marketing.
