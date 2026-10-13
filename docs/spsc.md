# SPSC FIFO ring

```
ITCH file → Parser thread → [SPSC ring] → Book builder thread → Orderbook
```

Why SPSC?

- parser can run ahead while orderbook thread consumes the pushed queue items
- Zero allocation on handoff: preallocated ring, fixed-size slots. No malloc on hot path
- it may improve throughput over a single thread doing all these tasks
  - parsing from ITCH file
  - queuing up the order messages (skipping over irrelevent order messages)
  - executing the queued up order message
- it may improve resiliency of the program to spikes in message traffic

Disadvantages:

- have to manage more threads
- uses more memory, since each FIFO needs place to store its messages

# Specs

`Single Producer Single Consumer Lock-Free Wait-Free Fifo`

- Single producer: one producer/writer thread
- Single consumer: one consumer/reader thread
- Lock-free: doesn't use mutex locks. At any point of time, some thread will make progress
- Wait-free: each thread moves forward regardless of other threads
- Circular Fifo : a single, fixed-size buffer as if it were connected end-to-end. Oldest entry is processed first.

# push/pop cursor implementation

64 bit unsigned integers so there are ltos of numbers available for use

- start at 0, everytime we push we increment push cursor
- probally won't run out (especially if system reboots periodically, which is standard practice for orderbook implementations)
- when you pop a value, you remove it from the pop cursor location and increment pop cursor
- eventually if no new value are added, pop cursor == push cursor and fifo becomes empty

There are different ways to convert continously increasing cursor value to offset in array

- via % operator constrain cursor to allowable range of indices on the array that is storing the value
- constrain capacity of FIFO to integral power of 2, then using & operator

## Why not use the remainder operator?

On AMD Zen 5 (9800x3d) per "https://www.agner.org/optimize/instruction_tables.pdf"

```
AND, OR, XOR

Operands    μops    Latency    Recip. Throughput
-------------------------------------------------
r, r        1       1          0.20
r, i        1       1          0.19
r, m        1       —          0.25
m, r/i      1       1          0.67

DIV

Operands    μops    Latency    Recip. Throughput
-------------------------------------------------
r8/m8       1       12         4
r16/m16     2       12         4
r32/m32     2       11–14      6
r64/m64     2       11–19      7–13
```

- which is significantly higher latency and Recipricoal throughput
- comparing AND r,r to DIV r64
  - 35-65x throughput gap

## AND(&) operator implemantion

By constraining queue's capacity to integral power of 2, can replace %operator with single-cycle bitwise AND(&) operation

Mathematically, for any non-negative integer cursor and any power of 2 capacity:
cursor % capacity == cursor & (capacity - 1)

```
capacity = 8 => binary representation = 1000
mask = capacity - 1 = 8 - 1 = 7 => binary representation = 0111

say cursor implemented to 10
10%8 = 2

10 & mask (7) => 1010 & 0111 = 0010 = 2
```

# Optimizations
- %operator vs &operator implementation
- memory ordering relaxed vs all sequentially consistent
- false sharing prevention vs no false sharing prevention
  - we achieve false sharing prevention through `alignas(std::hardware_destructive_interference_size)` between push cursor and pop cursor
  - achieve false sharing prevention between push/pop cursors and adjacent objects through `char padding_[std::hardware_destructive_interference_size - sizeof(std::size_t)];`
- caching push/popCursor via cachedPopCursor and cachedPushCursor


# Measurements

| Measurement                                                    | Why                                         |
| -------------------------------------------------------------- | ------------------------------------------- |
| Parse-to-book p50/p99/p99.9, single-threaded vs SPSC-pipelined | Does ring actually help?                    |
| Ring handoff cost (ns/msg)                                     | overhead paid                               |
| Throughput (msg/sec) both modes                                | does pipelining improve throughput          |
| Ring occupancy distribution                                    | is the producer or consumer the bottleneck? |
| Cycles/msg producer, cycles/msg consumer                       | where the time goes                         |
| Book-state hashes, both modes                                  | correctness                                 |
