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

64 bit unsigned integers so there are lots of numbers available for use

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
  - we achieve false sharing prevention through `alignas(kCacheLineSize)` between push cursor and pop cursor
  - achieve false sharing prevention between push/pop cursors and adjacent objects through `char padding_[skCacheLineSize - sizeof(std::size_t)];`
- caching push/popCursor via cachedPopCursor and cachedPushCursor

# False sharing prevention

Because of MESI cache coherence protocol, when Core 0 (Producer) writes to pushCursor, the cache line holding both cursors is marked "Modified" and invaliadted on Core 1 (Consumer).

- Core 1 is forced to fetch it from L3 or main memory on next read

By using alignas(kCacheLineSize) which we set to 64, on both our push and pop cursor

```c
alignas(kCacheLineSize) std::atomic<std::size_t> pushCursor_{};
alignas(kCacheLineSize) std::atomic<std::size_t> popCursor_{};
```

- it provides us with the offset between 2 objects to avoid false sharing on most modern CPU with cache line of 64 bytes.

# Why cache push / pop cursor?

```c
bool push(T const &value) {
    auto pushCursor = pushCursor_.load(std::memory_order::relaxed);
    auto popCursor = popCursor_.load(std::memory_order::acquire);
    if (full(pushCursor, popCursor)) return false;
    new (&ring_[pushCursor & (capacity_ - 1)]) T(value);
    pushCursor_.store(pushCursor + 1, std::memory_order_release);
    return true;
}

bool pop(T &value) {
  auto pushCursor = pushCursor_.load(std::memory_order::acquire);
  auto popCursor = popCursor_.load(std::memory_order::relaxed);
  if (empty(pushCursor, popCursor)) return false;
  value = ring_[popCursor & (capacity_ - 1)];
  ring_[popCursor & (capacity_ - 1)].~T();
  popCursor_.store(popCursor + 1, std::memory_order_release);
  return true;
}
```

Currently on push and pop operations, we must always perform a load with `std::memory_order::acquire` and a store with `std::memory_order_release`.
This is to ensure that we establish a happens-before relationship between consumer (pop) and producer (push) thread.

In push(), loading our `popCursor_` with `acquire` pairs with the `release` store of `popCursor_` in pop().
When the acquire load observes the released value, it syncrhonizes with that store, so everything sequenced-before the release store in pop(), happens-before everything sequenced after the acquire load in push().

However, performing atomic loads with `acquire` semantics on every single push and pop operation causes unnecessary cache-line bounching and memory barrier overhead.

Importantly, the cursors only monotonically increment

- If the producer keeps a local (non-atomic) copy of the last observed popCursor* and the queue is not full according to this cached value, it is mathematically impossible for the queue to be full `pushCursor*-popCursor\_ == capacity` in reality.
  - we know cachedPopCursor < popCursor\_
  - thus as long as `pushCursor_-cachedPopCursor != capacity`, we know that `pushCursor_-popCursor_ != capacity` and that
  - only when `pushCursor_- cachedPopCursor == capacity` do we need to load the popCursor\_ to update our cachedPopCursor
- likewise, if consumer keeps a local (non-atomic) copy of last observed pushCursor* and the queue is not empty according to this cached value, it is mathematically impossible for the queue to be empty `pushCursor* - popCursor\_ == 0` in reality
  - since cachedPushCursor < pushCursor\_
  - as long as `cachedPushCursor - popCursor_ != 0` we know that `pushCursor_ - popCursor_ != 0`
  - only when `cachedPushCursor - popCursor_ == 0` do we to load the pushCursor\_ to update our cachedPushCursor

```c
alignas(kCacheLineSize) std::size_t cachedPushCursor_{};
alignas(kCacheLineSize) std::size_t cachedPopCursor_{};

alignas(kCacheLineSize) std::atomic<std::size_t> pushCursor_{};
alignas(kCacheLineSize) std::atomic<std::size_t> popCursor_{};

bool push(T const &value) {
    auto pushCursor = pushCursor_.load(std::memory_order::relaxed);
    if (full(pushCursor, cachedPopCursor_)) {
      cachedPopCursor_ = popCursor_.load(std::memory_order::acquire);
      if (full(pushCursor, cachedPopCursor_)) return false;
    }
    new (&ring_[pushCursor & (capacity_ - 1)]) T(value);
    pushCursor_.store(pushCursor + 1, std::memory_order_release);
    return true;
  }

bool pop(T &value) {
  auto popCursor = popCursor_.load(std::memory_order_relaxed);
  if (empty(cachedPushCursor_, popCursor)) {
    cachedPushCursor_ = pushCursor_.load(std::memory_order::acquire);
    if (empty(cachedPushCursor_, popCursor)) return false;
  }
  value = ring_[popCursor & (capacity_ - 1)];
  ring_[popCursor & (capacity_ - 1)].~T();
  popCursor_.store(popCursor + 1, std::memory_order_release);
  return true;
}
```

# Benchmark

| Measurement                                                    | Why                                         |
| -------------------------------------------------------------- | ------------------------------------------- |
| Parse-to-book p50/p99/p99.9, single-threaded vs SPSC-pipelined | Does ring actually help?                    |
| Ring handoff cost (ns/msg)                                     | overhead paid                               |
| Throughput (msg/sec) both modes                                | does pipelining improve throughput          |
| Ring occupancy distribution                                    | is the producer or consumer the bottleneck? |
| Cycles/msg producer, cycles/msg consumer                       | where the time goes                         |
| Book-state hashes, both modes                                  | correctness                                 |
