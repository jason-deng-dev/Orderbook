# Signal Roadmap — Orderbook Project

Goal: standalone project that reads as "understands market microstructure AND C++" — not another generic orderbook tutorial clone.

## Phase 0 — Correctness Gate (non-negotiable)

- Finish test suites: orderbook, orders, trader — happy paths + edge cases (self-trade, partial fills, empty book, zero/negative qty, price-time priority)
- Fix the currently failing `TraderTest.InventoryOperations` — know WHY it fails before moving on
- **Rule: no benchmark counts until correctness is airtight. A fast wrong engine is a liability; interviewers ask "how do you know it's correct?" before they ask speed.**

## Phase 1 — Benchmark Harness

- Add Google Benchmark via FetchContent (pattern already in CMakeLists)
- Microbenchmarks per operation: add / cancel / match / partial-fill / price-cross
- Report **p50 / p99 / p99.9 / p999 per operation** — tails, not throughput averages
- Machine + compiler flags + `-march=native` documented in benchmark output

## Phase 2 — Workload Realism

- Synthetic first: Poisson arrival bursts, cancellation-heavy mix (real venues: 60–80% of messages are cancels), depth 10–20
- Then **replay a real order stream** — LOBSTER (free LSE sample) or crypto tick data (Bitstamp/XBT)
- Deliverable sentence target: "replayed 1M-message real order stream at p99 X ns"

## Phase 3 — Measurement Rigor (where juniors die)

- Steady/TSC clock, pin thread to core, warmup runs, median-of-N with spread
- Hardware counters via `perf stat`, not just wall time
- Methodology section written in docs/ — interviewer WILL ask "how did you measure?"
- Sloppy methodology is worse than no benchmark

## Phase 4 — Optimization Narrative (the resume arc)

2–3 experiments, each one paragraph: hypothesis → change → measured result.

Idea seed list:
1. `std::map` level store → flat vector + binary search
2. Arena/object-pool allocator for orders (reduces alloc/free churn)
3. Reserve/compact hot paths; `-O2` vs `-O3 -march=native`
4. Hot/cold data splitting (per-order data vs per-price-level data)

## Phase 5 — Concurrency at the Edges (see discussion — engine stays single-threaded)

Production shape, not "mutex around the map":
- Order-entry threads pushing orders through a lock-free SPSC queue → engine thread
- Reader threads querying top-of-book via seqlock (lock-free reads)
- This matches how real matching engines are deployed

## Phase 6 — Docs + Packaging

- docs/methodology.md: measurement setup, reproducibility
- README: architecture diagram, benchmark table, optimization story
- Resume line carries: real data, tails, a change that moved a number, why

## Milestone order

Correctness → harness → synthetic benchmarks → real-data replay → 2–3 experiments → concurrency phase → writeup

---

# Resume Positioning

## Context: where this fits the resume

Projects section is currently empty — this is the C++ credibility artifact. Existing C++ line in work history (Monte Carlo engine) is full-stack-adjacent; this project is the one that says **"I build performance-critical C++ with domain awareness"**. Coursework (Capital Markets 93%) + this = finance + systems signal. Every bullet below must be defensible in an interview — no number survives that you can't reproduce.

## End-state project description draft

**Order Book Matching Engine — C++23**
(github.com/jason-deng-dev/... )

- Built price-time priority matching engine with self-trade prevention, partial-fill and cancellation semantics; 90%+ line coverage across 3 gtest suites
- Replayed 1M-message real order stream (LOBSTER/Bitstamp): p50 X ns / p99 X ns / p999 X ns per operation; reported full latency distribution, not throughput
- Profiled with perf → replaced std::map level store with contiguous flat storage + arena allocator: 2.4× p50, 6× p999, 40% fewer cache misses
- Concurrent order entry via lock-free SPSC queue (acquire/release ordering); matching engine serialized by design for determinism — mirrors production exchange architecture
- C++23, CMake + presets + CTest + FetchContent; benchmark-driven development with documented methodology

## Why each line earns its place

| Line | What it signals |
|---|---|
| Correctness + coverage % | "fast AND right" — the ordering interviewers probe first |
| Real stream + tail percentiles | domain realism (cancels, bursts) + tail-awareness, not throughput bragging |
| perf → redesign + deltas | benchmark-driven judgment; can run an experiment and read a number |
| SPSC + serialized engine + "why" | knows where concurrency belongs in real exchanges — top 10% junior answer |
| Build tooling | CMake/CTest discipline = ships code teams can build |

## Interview talking points (memorize, say naturally)

1. "Matching engines are single-threaded by design on real exchanges — determinism. Concurrency lives at the edges: order entry, market data."
2. "60–80% of real messages are cancellations, so I benchmarked against a cancellation-heavy replayed stream, not uniform inserts."
3. "Averages hide the tail. I report p50/p99/p999 because a 1ms outlier at p99.9 is a broken promise, not an average."

## Honest caveats

- Placeholder numbers must be replaced with real measured values before this ships
- Do not write "lock-free" unless the SPSC queue has tests proving single-producer single-consumer correctness
- Concurrency phase is last for a reason — debug order: correct → measured → optimized → concurrent
