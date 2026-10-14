#include "spsc_fifo.h"
#include "types.h"
#include <gtest/gtest.h>

TEST(Spsc_Fifo, Init) {
  SpscFifo<int> fifo;
  EXPECT_EQ(fifo.capacity(), 65536);
  EXPECT_EQ(fifo.size(), 0);
  EXPECT_TRUE(fifo.empty());
}

TEST(Spsc_Fifo, pushBaseCase) {
  SpscFifo<int> fifo;
  EXPECT_EQ(fifo.size(), 0);
  fifo.push(100);
  EXPECT_EQ(fifo.size(), 1);
  EXPECT_FALSE(fifo.empty());
  fifo.push(100);
  EXPECT_EQ(fifo.size(), 2);
  EXPECT_FALSE(fifo.empty());
}

TEST(Spsc_Fifo, popBaseCase) {
  SpscFifo<int> fifo;
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
