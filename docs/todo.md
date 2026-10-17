# Orderbook — TODO

## Done

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
- [ ] optimizations
  - [x] memory ordering
  - [x] false sharing prevention via padding
  - [ ] cache push/popCursor
