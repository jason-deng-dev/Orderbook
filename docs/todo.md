# Orderbook
- [x] `Orderbook_Map`
  - [x] AddOrder
  - [x] DeleteOrder
  - [x] ModifyOrder
  - [x] FillOrder
    - [x] self-trade prevention
  - [x] EventLog
- [x] `Orderbook_Vector` (reverse-ordered level vector, search policy as template parameter)
- [x] Search policies: `LinearSearch`, `BinarySearch`, `BranchlessBinarySearch`


# Feed Handler
- [x] setup
- [x] load
- [x] push to SPSC Fifo

# SPSC Fifo
- [x] base version working
  - [x] tests working
  - [x] benchmark 
- [x] optimizations
  - [x] memory ordering
  - [x] false sharing prevention via padding

# Optimizations
- [x] cached push/popCursor for SPSC Fifo
- [x] Intrusive slab orderbook

# Benchmark
- [x] pin each thread with `pthread_setaffinity_np`

# Cleanup
- [x] Hardware section on README
- [x] Architecture diagram on README


- [ ] p99/p99.9 for Orderbook
- [ ] correct SPSC fifo benchmark
