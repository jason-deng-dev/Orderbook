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
- [ ] PMR
- [ ] Intrusive
- [ ] slab-backed
- [ ] offload cost of logging to Async Event Ring Buffer
  - [ ] Producer (Orderbook), when match or cancel happens, orderbook synchronously copies OrderEvent into SPSC queue
  - [ ] Consumer (Logger thread), separate, dedicated CPU core constatly pops from this SPSC queue, taking the raw OrderEvent, formats it, and writes it to disk

# Feed Handler
- [x] setup
- [x] load
- [x] push to SPSC Fifo

# SPSC Fifo
- [x] base version working
  - [x] tests working
  - [x] benchmark 
- [ ] optimizations
  - [x] memory ordering
  - [x] false sharing prevention via padding

# Optimizations
- [x] cached push/popCursor for SPSC Fifo
- [ ] Intrusive slab orderbook
- [ ] event log spsc for orderbook

# Benchmark
- [x] pin each thread with `pthread_setaffinity_np`
- [ ] Feedhandler buffer RAII to guarantee cleanup
- [ ] 
