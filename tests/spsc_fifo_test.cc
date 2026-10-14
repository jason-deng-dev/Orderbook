#include "spsc_fifo.h"
#include "types.h"
#include <gtest/gtest.h>

TEST(Spsc_Fifo, Init) {
  SpscFifo<int> fifo;
  EXPECT_EQ(fifo.capacity(), 65536);
  EXPECT_EQ(fifo.size(), 0);
  EXPECT_TRUE(fifo.empty());
}
