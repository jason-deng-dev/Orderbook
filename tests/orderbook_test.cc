#include "orderbook_map.h"
#include "types.h"
#include <gtest/gtest.h>
#include <optional>

// Expected result for an order that rests on the book without matching.
static void ExpectResting(const MatchResult &r, Volume volume) {
  EXPECT_EQ(r.status, OrderStatus::Resting);
  EXPECT_EQ(r.error, OrderbookError::OK);
  EXPECT_EQ(r.filledVolume, 0);
  EXPECT_EQ(r.restingVolume, volume);
}

TEST(Orderbook_Map, Initalization) {
  Orderbook_Map ob;
  EXPECT_EQ(ob.GetBestBid(), std::nullopt);
  EXPECT_EQ(ob.GetBestAsk(), std::nullopt);
  EXPECT_EQ(ob.GetTotalOrderCount(), 0);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Bid), 0);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Ask), 0);
  EXPECT_EQ(ob.GetOrderCountAtPrice(100, Side::Bid), 0);
  EXPECT_EQ(ob.GetOrderCountAtPrice(100, Side::Ask), 0);
}

TEST(Orderbook_Map, AddOrder) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 1), 10);

  auto dup = ob.AddOrder(1, Side::Ask, 100, 10, 1);
  EXPECT_EQ(dup.status, OrderStatus::Rejected);
  EXPECT_EQ(dup.error, OrderbookError::DuplicateId);
  EXPECT_EQ(dup.filledVolume, 0);

  EXPECT_TRUE(ob.HasOrder(1));
  auto order = ob.GetOrder(1);
  ASSERT_TRUE(order.has_value());
  EXPECT_EQ(order->side, Side::Bid);
  EXPECT_EQ(order->id, 1);
  EXPECT_EQ(order->trader_id, 1);
  EXPECT_EQ(order->price, 100);
  EXPECT_EQ(order->volume, 10);

  EXPECT_EQ(ob.GetTotalOrderCount(), 1);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Bid), 10);
  EXPECT_EQ(ob.GetOrderCountAtPrice(100, Side::Bid), 1);
}

TEST(Orderbook_Map, AddOrderAggregatesAtSamePrice) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 1), 10);
  ExpectResting(ob.AddOrder(2, Side::Bid, 100, 5, 2), 5);
  ExpectResting(ob.AddOrder(3, Side::Ask, 105, 7, 1), 7);

  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Bid), 15);
  EXPECT_EQ(ob.GetOrderCountAtPrice(100, Side::Bid), 2);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(105, Side::Ask), 7);
  EXPECT_EQ(ob.GetOrderCountAtPrice(105, Side::Ask), 1);
  EXPECT_EQ(ob.GetTotalOrderCount(), 3);
}

TEST(Orderbook_Map, BestBidAndAskAcrossPrices) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 1), 10);
  ExpectResting(ob.AddOrder(2, Side::Bid, 101, 10, 1), 10);
  ExpectResting(ob.AddOrder(3, Side::Bid, 99, 10, 1), 10);
  ExpectResting(ob.AddOrder(4, Side::Ask, 105, 10, 1), 10);
  ExpectResting(ob.AddOrder(5, Side::Ask, 103, 10, 1), 10);
  ExpectResting(ob.AddOrder(6, Side::Ask, 107, 10, 1), 10);

  EXPECT_EQ(ob.GetBestBid(), 101); // highest bid
  EXPECT_EQ(ob.GetBestAsk(), 103); // lowest ask
}

TEST(Orderbook_Map, DeleteOrder) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 1), 10);
  ExpectResting(ob.AddOrder(2, Side::Bid, 100, 5, 2), 5);

  EXPECT_EQ(ob.DeleteOrder(1), OrderbookError::OK);
  EXPECT_FALSE(ob.HasOrder(1));
  EXPECT_EQ(ob.GetOrder(1), std::nullopt);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Bid), 5);
  EXPECT_EQ(ob.GetOrderCountAtPrice(100, Side::Bid), 1);
  EXPECT_EQ(ob.GetTotalOrderCount(), 1);

  EXPECT_EQ(ob.DeleteOrder(1), OrderbookError::OrderNotFound);
  EXPECT_EQ(ob.DeleteOrder(999), OrderbookError::OrderNotFound);
}

TEST(Orderbook_Map, DeleteLastOrderRemovesPriceLevel) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 1), 10);
  ExpectResting(ob.AddOrder(2, Side::Bid, 101, 10, 1), 10);
  ExpectResting(ob.AddOrder(3, Side::Ask, 105, 10, 1), 10);
  ExpectResting(ob.AddOrder(4, Side::Ask, 103, 10, 1), 10);

  EXPECT_EQ(ob.DeleteOrder(2), OrderbookError::OK);
  EXPECT_EQ(ob.GetBestBid(), 100);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(101, Side::Bid), 0);

  EXPECT_EQ(ob.DeleteOrder(4), OrderbookError::OK);
  EXPECT_EQ(ob.GetBestAsk(), 105);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(103, Side::Ask), 0);
}

TEST(Orderbook_Map, ReAddAfterDelete) {
  Orderbook_Map ob;
  EXPECT_EQ(ob.DeleteOrder(1), OrderbookError::OrderNotFound);
  // id must be reusable after deletion
  ExpectResting(ob.AddOrder(1, Side::Ask, 105, 20, 2), 20);
  EXPECT_TRUE(ob.HasOrder(1));
  auto order = ob.GetOrder(1);
  ASSERT_TRUE(order.has_value());
  EXPECT_EQ(order->side, Side::Ask);
  EXPECT_EQ(order->price, 105);
  EXPECT_EQ(order->volume, 20);
}

TEST(Orderbook_Map, ModifyOrder) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 1), 10);

  EXPECT_EQ(ob.ModifyOrder(1, 4), OrderbookError::OK);
  EXPECT_EQ(ob.GetOrder(1)->volume, 4);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Bid), 4);

  EXPECT_EQ(ob.ModifyOrder(999, 5), OrderbookError::OrderNotFound);
}

TEST(Orderbook_Map, ModifyOrderToZeroDeletes) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 1), 10);
  EXPECT_EQ(ob.ModifyOrder(1, 0), OrderbookError::OK);
  EXPECT_FALSE(ob.HasOrder(1));
  EXPECT_EQ(ob.GetTotalOrderCount(), 0);
  EXPECT_EQ(ob.GetBestBid(), std::nullopt);
}

TEST(Orderbook_Map, ModifyOrderIncreaseRejected) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 1), 10);
  // reduce-only modify: increasing volume is rejected, order unchanged
  EXPECT_EQ(ob.ModifyOrder(1, 25), OrderbookError::InvalidVolume);
  EXPECT_EQ(ob.ModifyOrder(1, 10), OrderbookError::InvalidVolume); // same size is a no-op, reject
  EXPECT_EQ(ob.GetOrder(1)->volume, 10);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Bid), 10);
}

// ----------------------------- HandleFill --------------------------------
// HandleFill is exercised through AddOrder: a crossing incoming order
// matches against resting orders on the opposite side.

TEST(Orderbook_Map, FillNoCrossingLiquidity) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Ask, 105, 10, 1), 10);
  // bid below best ask => no match, rests fully
  ExpectResting(ob.AddOrder(2, Side::Bid, 100, 8, 2), 8);

  EXPECT_EQ(ob.GetBestBid(), 100);
  EXPECT_EQ(ob.GetBestAsk(), 105);
  EXPECT_EQ(ob.GetTotalOrderCount(), 2);
}

TEST(Orderbook_Map, FillExactMatchBothRemoved) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Ask, 101, 10, 1), 10);
  auto r = ob.AddOrder(2, Side::Bid, 101, 10, 2);
  EXPECT_EQ(r.status, OrderStatus::Filled);
  EXPECT_EQ(r.error, OrderbookError::OK);
  EXPECT_EQ(r.filledVolume, 10);
  EXPECT_EQ(r.restingVolume, 0);

  // both sides fully consumed
  EXPECT_FALSE(ob.HasOrder(1));
  EXPECT_FALSE(ob.HasOrder(2));
  EXPECT_EQ(ob.GetBestBid(), std::nullopt);
  EXPECT_EQ(ob.GetBestAsk(), std::nullopt);
  EXPECT_EQ(ob.GetTotalOrderCount(), 0);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(101, Side::Ask), 0);
}

TEST(Orderbook_Map, FillIncomingPartiallyFilledRestsRemainder) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Ask, 101, 4, 1), 4);

  auto r = ob.AddOrder(2, Side::Bid, 101, 10, 2);
  EXPECT_EQ(r.status, OrderStatus::PartiallyFilled);
  EXPECT_EQ(r.filledVolume, 4);
  EXPECT_EQ(r.restingVolume, 6);

  // ask side consumed, remainder rests on the bid side
  EXPECT_FALSE(ob.HasOrder(1));
  EXPECT_EQ(ob.GetBestAsk(), std::nullopt);
  ASSERT_TRUE(ob.HasOrder(2));
  EXPECT_EQ(ob.GetOrder(2)->volume, 6);
  EXPECT_EQ(ob.GetBestBid(), 101);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(101, Side::Bid), 6);
}

TEST(Orderbook_Map, FillRestingPartiallyFilledRemainsResting) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Ask, 101, 10, 1), 10);

  auto r = ob.AddOrder(2, Side::Bid, 101, 4, 2);
  EXPECT_EQ(r.status, OrderStatus::Filled);
  EXPECT_EQ(r.filledVolume, 4);
  EXPECT_EQ(r.restingVolume, 0);

  // resting ask keeps the leftover volume
  ASSERT_TRUE(ob.HasOrder(1));
  EXPECT_EQ(ob.GetOrder(1)->volume, 6);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(101, Side::Ask), 6);
  EXPECT_EQ(ob.GetOrderCountAtPrice(101, Side::Ask), 1);
  // incoming order fully filled => not on the book
  EXPECT_FALSE(ob.HasOrder(2));
  EXPECT_EQ(ob.GetBestBid(), std::nullopt);
}

TEST(Orderbook_Map, FillWalksMultiplePriceLevels) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Ask, 100, 5, 1), 5);
  ExpectResting(ob.AddOrder(2, Side::Ask, 101, 5, 1), 5);

  auto r = ob.AddOrder(3, Side::Bid, 101, 8, 2);
  EXPECT_EQ(r.status, OrderStatus::Filled);
  EXPECT_EQ(r.filledVolume, 8);
  EXPECT_EQ(r.restingVolume, 0);

  // best price level fully consumed first, then next level partially
  EXPECT_FALSE(ob.HasOrder(1));
  ASSERT_TRUE(ob.HasOrder(2));
  EXPECT_EQ(ob.GetOrder(2)->volume, 2);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Ask), 0);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(101, Side::Ask), 2);
  EXPECT_EQ(ob.GetBestAsk(), 101);
}

TEST(Orderbook_Map, FillStopsAtPriceLimit) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Ask, 100, 5, 1), 5);
  ExpectResting(ob.AddOrder(2, Side::Ask, 103, 5, 1), 5);

  // bid at 101 crosses 100 but not 103
  auto r = ob.AddOrder(3, Side::Bid, 101, 8, 2);
  EXPECT_EQ(r.status, OrderStatus::PartiallyFilled);
  EXPECT_EQ(r.filledVolume, 5);
  EXPECT_EQ(r.restingVolume, 3);

  // 103 level untouched, remainder rests at 101
  ASSERT_TRUE(ob.HasOrder(2));
  EXPECT_EQ(ob.GetOrder(2)->volume, 5);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(103, Side::Ask), 5);
  EXPECT_EQ(ob.GetBestBid(), 101);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(101, Side::Bid), 3);
}

TEST(Orderbook_Map, FillRespectsFifoWithinLevel) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Ask, 100, 5, 1), 5);
  ExpectResting(ob.AddOrder(2, Side::Ask, 100, 5, 2), 5);

  auto r = ob.AddOrder(3, Side::Bid, 100, 7, 3);
  EXPECT_EQ(r.status, OrderStatus::Filled);
  EXPECT_EQ(r.filledVolume, 7);

  // first-in order fully filled, second keeps the remainder
  EXPECT_FALSE(ob.HasOrder(1));
  ASSERT_TRUE(ob.HasOrder(2));
  EXPECT_EQ(ob.GetOrder(2)->volume, 3);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Ask), 3);
  EXPECT_EQ(ob.GetOrderCountAtPrice(100, Side::Ask), 1);
}

TEST(Orderbook_Map, FillSellSideMatchesBids) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 5, 1), 5);
  ExpectResting(ob.AddOrder(2, Side::Bid, 99, 5, 1), 5);

  // incoming ask at 99 crosses both bid levels
  auto r = ob.AddOrder(3, Side::Ask, 99, 8, 2);
  EXPECT_EQ(r.status, OrderStatus::Filled);
  EXPECT_EQ(r.filledVolume, 8);
  EXPECT_EQ(r.restingVolume, 0);

  // highest bid consumed first
  EXPECT_FALSE(ob.HasOrder(1));
  ASSERT_TRUE(ob.HasOrder(2));
  EXPECT_EQ(ob.GetOrder(2)->volume, 2);
  EXPECT_EQ(ob.GetBestBid(), 99);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Bid), 0);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(99, Side::Bid), 2);
}

TEST(Orderbook_Map, FillSellSideStopsAboveLimit) {
  Orderbook_Map ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 5, 1), 5);
  ExpectResting(ob.AddOrder(2, Side::Bid, 98, 5, 1), 5);

  // ask at 99 crosses bid 100 but not bid 98
  auto r = ob.AddOrder(3, Side::Ask, 99, 8, 2);
  EXPECT_EQ(r.status, OrderStatus::PartiallyFilled);
  EXPECT_EQ(r.filledVolume, 5);
  EXPECT_EQ(r.restingVolume, 3);

  ASSERT_TRUE(ob.HasOrder(2));
  EXPECT_EQ(ob.GetOrder(2)->volume, 5);
  EXPECT_EQ(ob.GetBestAsk(), 99);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(99, Side::Ask), 3);
}
