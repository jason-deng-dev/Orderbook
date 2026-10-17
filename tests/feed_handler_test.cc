#include "feed_handler.h"
#include "types.h"
#include <gtest/gtest.h>

TEST(Feed_Handler, Initalization) {
  Feed_Handler fh;
  EXPECT_FALSE(fh.Load(FIXTURES_DIR "/wrongfile"));
  EXPECT_TRUE(fh.Load(FIXTURES_DIR "/07302019.NASDAQ_ITCH50"));
  EXPECT_EQ(fh.size(), 8661679413ULL); // stat -c '%s' 07302019.NASDAQ_ITCH50
};

// fixtures/parse_fixture.itch: S, R, A, F, X, D, U in that order.
// GetNextMessage returns one message per call: nullopt for non-book messages,
// a QueueItem for state-modifying messages, and nullopt at end of buffer.
TEST(Feed_Handler, ParsesMessageSequence) {
  Feed_Handler fh;
  ASSERT_TRUE(fh.Load(FIXTURES_DIR "/parse_fixture.itch"));

  // 'S' System Event: filtered, cursor still advances.
  EXPECT_FALSE(fh.GetNextMessage().has_value());
  // 'R' Stock Directory: filtered, cursor still advances.
  EXPECT_FALSE(fh.GetNextMessage().has_value());

  auto add = fh.GetNextMessage();
  ASSERT_TRUE(add.has_value());
  EXPECT_EQ(add->orderType, OrderType::Add);
  EXPECT_EQ(add->orderId, 1u);
  EXPECT_EQ(add->side, Side::Bid);
  EXPECT_EQ(add->volume, 300u);
  EXPECT_EQ(add->price, 1'000'000);

  auto addMpid = fh.GetNextMessage();
  ASSERT_TRUE(addMpid.has_value());
  EXPECT_EQ(addMpid->orderType, OrderType::Add);
  EXPECT_EQ(addMpid->orderId, 2u);
  EXPECT_EQ(addMpid->side, Side::Ask);
  EXPECT_EQ(addMpid->volume, 200u);
  EXPECT_EQ(addMpid->price, 2'000'000);

  auto cancel = fh.GetNextMessage();
  ASSERT_TRUE(cancel.has_value());
  EXPECT_EQ(cancel->orderType, OrderType::Cancel);
  EXPECT_EQ(cancel->orderId, 2u);
  EXPECT_EQ(cancel->volume, 50u);

  auto del = fh.GetNextMessage();
  ASSERT_TRUE(del.has_value());
  EXPECT_EQ(del->orderType, OrderType::Delete);
  EXPECT_EQ(del->orderId, 1u);

  auto replace = fh.GetNextMessage();
  ASSERT_TRUE(replace.has_value());
  EXPECT_EQ(replace->orderType, OrderType::Replace);
  EXPECT_EQ(replace->orderId, 3u);
  EXPECT_EQ(replace->removeId, 1u);
  EXPECT_EQ(replace->volume, 400u);
  EXPECT_EQ(replace->price, 1'000'000);

  EXPECT_FALSE(fh.GetNextMessage().has_value());
}
