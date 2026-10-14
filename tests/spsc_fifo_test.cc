#include "spsc_fifo.h"
#include "types.h"
#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <thread>

TEST(Spsc_Fifo, Init) {
  SpscFifo<int,4> fifo;
  EXPECT_EQ(fifo.capacity(), 4);
  EXPECT_EQ(fifo.size(), 0);
  EXPECT_TRUE(fifo.empty());
}

TEST(Spsc_Fifo, pushBaseCase) {
  SpscFifo<int,4> fifo;
  EXPECT_EQ(fifo.size(), 0);
  fifo.push(100);
  EXPECT_EQ(fifo.size(), 1);
  EXPECT_FALSE(fifo.empty());
  fifo.push(100);
  EXPECT_EQ(fifo.size(), 2);
  EXPECT_FALSE(fifo.empty());
}

TEST(Spsc_Fifo, popBaseCase) {
  SpscFifo<int,4> fifo;
  fifo.push(1);
  fifo.push(2);
  EXPECT_EQ(fifo.size(), 2);
  EXPECT_FALSE(fifo.empty());
  int val1;
  fifo.pop(val1);
  EXPECT_EQ(val1, 1);
  EXPECT_EQ(fifo.size(), 1);
  int val2;
  fifo.pop(val2);
  EXPECT_EQ(val2, 2);
  EXPECT_EQ(fifo.size(), 0);
  EXPECT_TRUE(fifo.empty());
}

// ---------------------------------------------------------------------------
// Helper type: counts constructions and destructions, so a leaked element or a
// double-destroy shows up as live != 0 once the FIFO is out of scope.
// ---------------------------------------------------------------------------

struct Tracked {
  static inline int live = 0;
  static inline int constructed = 0;
  static inline int destroyed = 0;

  int value = 0;

  explicit Tracked(int v = 0) : value{v} {
    ++live;
    ++constructed;
  }
  Tracked(Tracked const &other) : value{other.value} {
    ++live;
    ++constructed;
  }
  Tracked &operator=(Tracked const &) = default;
  ~Tracked() {
    --live;
    ++destroyed;
  }

  static void reset() { live = constructed = destroyed = 0; }
};

// ---------------------------------------------------------------------------
// Boundary conditions
// ---------------------------------------------------------------------------

TEST(Spsc_Fifo, fullAndPushWhenFull) {
  constexpr size_t kCapacity = 8;
  SpscFifo<int, kCapacity> fifo;

  EXPECT_FALSE(fifo.full());

  for (size_t i = 0; i < kCapacity; ++i) {
    ASSERT_TRUE(fifo.push(static_cast<int>(i))) << "push " << i;
    EXPECT_EQ(fifo.size(), i + 1);
  }

  EXPECT_TRUE(fifo.full());
  EXPECT_FALSE(fifo.empty());
  EXPECT_EQ(fifo.size(), kCapacity);
  EXPECT_EQ(fifo.capacity(), kCapacity);

  // Rejected pushes must not move a cursor or overwrite a live element.
  for (int rejected = 0; rejected < 4; ++rejected) {
    EXPECT_FALSE(fifo.push(rejected)) << "rejected " << rejected;
  }
  EXPECT_EQ(fifo.size(), kCapacity);

  int out = -1;
  ASSERT_TRUE(fifo.pop(out));
  EXPECT_EQ(out, 0);
  EXPECT_FALSE(fifo.full());

  ASSERT_TRUE(fifo.push(99));
  EXPECT_TRUE(fifo.full());

  for (int expected = 1; expected < static_cast<int>(kCapacity); ++expected) {
    ASSERT_TRUE(fifo.pop(out)) << "pop " << expected;
    EXPECT_EQ(out, expected);
  }
  ASSERT_TRUE(fifo.pop(out));
  EXPECT_EQ(out, 99);
  EXPECT_TRUE(fifo.empty());
}

TEST(Spsc_Fifo, emptyAndPopWhenEmpty) {
  SpscFifo<int, 8> fifo;

  EXPECT_TRUE(fifo.empty());
  EXPECT_FALSE(fifo.full());
  EXPECT_EQ(fifo.size(), 0);

  int out = -1;
  for (int attempt = 0; attempt < 4; ++attempt) {
    EXPECT_FALSE(fifo.pop(out)) << "attempt " << attempt;
    EXPECT_EQ(fifo.size(), 0);
    EXPECT_TRUE(fifo.empty());
  }
}

TEST(Spsc_Fifo, popOnEmptyLeavesValueUntouched) {
  SpscFifo<int, 8> fifo;

  int out = 1234;
  EXPECT_FALSE(fifo.pop(out));
  EXPECT_EQ(out, 1234);
}

TEST(Spsc_Fifo, capacityOne) {
  SpscFifo<int, 1> fifo;
  EXPECT_EQ(fifo.capacity(), 1);
  EXPECT_TRUE(fifo.empty());
  EXPECT_FALSE(fifo.full());

  ASSERT_TRUE(fifo.push(0));
  EXPECT_TRUE(fifo.full());
  EXPECT_EQ(fifo.size(), 1);
  EXPECT_FALSE(fifo.push(1));

  int out = -1;
  ASSERT_TRUE(fifo.pop(out));
  EXPECT_EQ(out, 0);
  EXPECT_TRUE(fifo.empty());

  // Second trip through the same single slot.
  ASSERT_TRUE(fifo.push(1));
  ASSERT_TRUE(fifo.pop(out));
  EXPECT_EQ(out, 1);
  EXPECT_TRUE(fifo.empty());
}

TEST(Spsc_Fifo, capacityTwoWrap) {
  SpscFifo<int, 2> fifo;

  for (int i = 0; i < 10; ++i) {
    ASSERT_TRUE(fifo.push(i)) << "push " << i;
    ASSERT_TRUE(fifo.push(i + 100));
    EXPECT_TRUE(fifo.full());
    EXPECT_FALSE(fifo.push(-1));

    int a = -1;
    int b = -1;
    ASSERT_TRUE(fifo.pop(a));
    ASSERT_TRUE(fifo.pop(b));
    EXPECT_EQ(a, i);
    EXPECT_EQ(b, i + 100);
    EXPECT_TRUE(fifo.empty());
  }
}

// ---------------------------------------------------------------------------
// Ordering and wrap-around
// ---------------------------------------------------------------------------

TEST(Spsc_Fifo, fifoOrdering) {
  constexpr int kCount = 1000;
  SpscFifo<int, 1024> fifo;

  for (int i = 0; i < kCount; ++i) {
    ASSERT_TRUE(fifo.push(i)) << "push " << i;
  }
  EXPECT_EQ(fifo.size(), static_cast<size_t>(kCount));

  for (int i = 0; i < kCount; ++i) {
    int out = -1;
    ASSERT_TRUE(fifo.pop(out)) << "pop " << i;
    EXPECT_EQ(out, i) << "pop " << i;
  }
  EXPECT_TRUE(fifo.empty());
}

// 32 pushes through a 4-slot ring: the index wraps 8 times.
TEST(Spsc_Fifo, wrapAroundEmptyToEmpty) {
  constexpr size_t kCapacity = 4;
  SpscFifo<int, kCapacity> fifo;

  for (int i = 0; i < 32; ++i) {
    ASSERT_TRUE(fifo.push(i)) << "push " << i;
    int out = -1;
    ASSERT_TRUE(fifo.pop(out)) << "pop " << i;
    EXPECT_EQ(out, i) << "pop " << i;
  }
  EXPECT_TRUE(fifo.empty());
}

// Keep two elements in flight while stepping the ring, so the cursors wrap
// while the FIFO is neither empty nor full.
TEST(Spsc_Fifo, wrapAroundWhilePartiallyFull) {
  constexpr size_t kCapacity = 4;
  SpscFifo<int, kCapacity> fifo;

  ASSERT_TRUE(fifo.push(0));
  ASSERT_TRUE(fifo.push(1));

  for (int i = 2; i < 40; ++i) {
    ASSERT_TRUE(fifo.push(i)) << "push " << i;
    int out = -1;
    ASSERT_TRUE(fifo.pop(out)) << "pop " << i;
    EXPECT_EQ(out, i - 2) << "pop " << i;
    EXPECT_EQ(fifo.size(), 2);
  }
}

TEST(Spsc_Fifo, manyWrapCycles) {
  constexpr int kCount = 10000;
  SpscFifo<int, 2> fifo;

  for (int i = 0; i < kCount; ++i) {
    ASSERT_TRUE(fifo.push(i)) << "push " << i;
    int out = -1;
    ASSERT_TRUE(fifo.pop(out)) << "pop " << i;
    EXPECT_EQ(out, i) << "pop " << i;
  }
}

// Uneven push/pop bursts, sized against the reported free space and occupancy
// so the test never asks for an operation it knows will be rejected.
TEST(Spsc_Fifo, interleavedPushPop) {
  constexpr size_t kCapacity = 8;
  SpscFifo<int, kCapacity> fifo;

  int nextPush = 0;
  int nextPop = 0;

  for (int round = 0; round < 500; ++round) {
    size_t room = kCapacity - fifo.size();
    int pushes = std::min<int>(1 + (round % 4), static_cast<int>(room));
    for (int i = 0; i < pushes; ++i) {
      ASSERT_TRUE(fifo.push(nextPush))
          << "round " << round << " push " << nextPush;
      ++nextPush;
    }

    int pops =
        std::min<int>(1 + ((round + 2) % 4), static_cast<int>(fifo.size()));
    for (int i = 0; i < pops; ++i) {
      int out = -1;
      ASSERT_TRUE(fifo.pop(out)) << "round " << round << " pop " << nextPop;
      EXPECT_EQ(out, nextPop) << "round " << round;
      ++nextPop;
    }

    EXPECT_EQ(fifo.size(), static_cast<size_t>(nextPush - nextPop))
        << "round " << round;
  }

  EXPECT_GT(nextPush, 100);
  EXPECT_GT(nextPop, 100);
}

TEST(Spsc_Fifo, indicesStayInRange) {
  constexpr size_t kCapacity = 4;
  SpscFifo<int, kCapacity> fifo;

  for (int i = 0; i < 64; ++i) {
    ASSERT_TRUE(fifo.push(i)) << "i " << i;
    EXPECT_LT(fifo.pushIndex(), kCapacity) << "i " << i;

    int out = -1;
    ASSERT_TRUE(fifo.pop(out)) << "i " << i;
    EXPECT_LT(fifo.popIndex(), kCapacity) << "i " << i;
  }
}

// ---------------------------------------------------------------------------
// Element lifetime
// ---------------------------------------------------------------------------

TEST(Spsc_Fifo, popDestroysPoppedElement) {
  Tracked::reset();
  SpscFifo<Tracked, 8> fifo;

  ASSERT_TRUE(fifo.push(Tracked{7}));

  Tracked out;
  ASSERT_TRUE(fifo.pop(out));
  EXPECT_EQ(out.value, 7);
  EXPECT_EQ(fifo.size(), 0);
  EXPECT_EQ(Tracked::live, 1) << "only the pop destination should still be alive";
}

TEST(Spsc_Fifo, destroysElementsOnScopeExit) {
  Tracked::reset();
  {
    SpscFifo<Tracked, 8> fifo;
    ASSERT_TRUE(fifo.push(Tracked{1}));
    ASSERT_TRUE(fifo.push(Tracked{2}));
    ASSERT_TRUE(fifo.push(Tracked{3}));
  }
  EXPECT_EQ(Tracked::live, 0);
  EXPECT_EQ(Tracked::constructed, Tracked::destroyed);
}

TEST(Spsc_Fifo, destroysFullRingOnScopeExit) {
  Tracked::reset();
  {
    SpscFifo<Tracked, 8> fifo;
    for (int i = 0; i < 8; ++i) {
      ASSERT_TRUE(fifo.push(Tracked{i}));
    }
    EXPECT_TRUE(fifo.full());
  }
  EXPECT_EQ(Tracked::live, 0);
  EXPECT_EQ(Tracked::constructed, Tracked::destroyed);
}

TEST(Spsc_Fifo, destroysRemainingElementsAfterPartialDrain) {
  Tracked::reset();
  {
    SpscFifo<Tracked, 8> fifo;
    for (int i = 0; i < 8; ++i) {
      ASSERT_TRUE(fifo.push(Tracked{i}));
    }

    Tracked out;
    for (int i = 0; i < 3; ++i) {
      ASSERT_TRUE(fifo.pop(out));
      EXPECT_EQ(out.value, i);
    }
    EXPECT_EQ(fifo.size(), 5);
  }
  EXPECT_EQ(Tracked::live, 0);
  EXPECT_EQ(Tracked::constructed, Tracked::destroyed);
}

TEST(Spsc_Fifo, reusesSlotAfterDestroy) {
  Tracked::reset();
  {
    SpscFifo<Tracked, 2> fifo;

    for (int i = 0; i < 20; ++i) {
      ASSERT_TRUE(fifo.push(Tracked{i})) << "push " << i;

      Tracked out;
      ASSERT_TRUE(fifo.pop(out));
      EXPECT_EQ(out.value, i);
    }
  }
  EXPECT_EQ(Tracked::live, 0);
  EXPECT_EQ(Tracked::constructed, Tracked::destroyed);
}

// ---------------------------------------------------------------------------
// Concurrency
//
// Only meaningful under ThreadSanitizer: no single-threaded test above can
// observe a missing acquire/release.
// ---------------------------------------------------------------------------

TEST(Spsc_Fifo, threadedProducerConsumer) {
  constexpr int kCount = 200000;
  SpscFifo<int, 1024> fifo;

  std::atomic<int> popped{0};
  std::atomic<int> mismatches{0};

  std::thread producer([&fifo] {
    for (int i = 0; i < kCount; ++i) {
      while (!fifo.push(i)) {
        std::this_thread::yield();
      }
    }
  });

  std::thread consumer([&] {
    while (popped.load(std::memory_order_relaxed) < kCount) {
      int out = -1;
      if (!fifo.pop(out)) {
        std::this_thread::yield();
        continue;
      }
      // Only this thread writes `popped`, so the relaxed read is the sequence
      // number this element should carry.
      if (out != popped.load(std::memory_order_relaxed)) {
        mismatches.fetch_add(1, std::memory_order_relaxed);
      }
      popped.fetch_add(1, std::memory_order_relaxed);
    }
  });

  producer.join();
  consumer.join();

  EXPECT_EQ(mismatches.load(), 0);
  EXPECT_EQ(popped.load(), kCount);
  EXPECT_TRUE(fifo.empty());
}

// Same shape on a 2-slot ring: forces the producer to see a full FIFO and the
// consumer to see an empty one many times.
TEST(Spsc_Fifo, threadedTightRing) {
  constexpr int kCount = 100000;
  SpscFifo<int, 2> fifo;

  std::atomic<int> popped{0};
  std::atomic<int> mismatches{0};

  std::thread producer([&fifo] {
    for (int i = 0; i < kCount; ++i) {
      while (!fifo.push(i)) {
        std::this_thread::yield();
      }
    }
  });

  std::thread consumer([&] {
    while (popped.load(std::memory_order_relaxed) < kCount) {
      int out = -1;
      if (!fifo.pop(out)) {
        std::this_thread::yield();
        continue;
      }
      if (out != popped.load(std::memory_order_relaxed)) {
        mismatches.fetch_add(1, std::memory_order_relaxed);
      }
      popped.fetch_add(1, std::memory_order_relaxed);
    }
  });

  producer.join();
  consumer.join();

  EXPECT_EQ(mismatches.load(), 0);
  EXPECT_EQ(popped.load(), kCount);
  EXPECT_TRUE(fifo.empty());
}
