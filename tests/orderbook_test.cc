#include "orderbook_map.h"
#include "types.h"
#include <gtest/gtest.h>
#include <optional>

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
  EXPECT_EQ(ob.AddOrder(1, Side::Bid, 100, 10, 1), OrderbookError::OK);
  EXPECT_EQ(ob.AddOrder(1, Side::Ask, 100, 10, 1), OrderbookError::DuplicateId);

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
  EXPECT_EQ(ob.AddOrder(1, Side::Bid, 100, 10, 1), OrderbookError::OK);
  EXPECT_EQ(ob.AddOrder(2, Side::Bid, 100, 5, 2), OrderbookError::OK);
  EXPECT_EQ(ob.AddOrder(3, Side::Ask, 100, 7, 1), OrderbookError::OK);

  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Bid), 15);
  EXPECT_EQ(ob.GetOrderCountAtPrice(100, Side::Bid), 2);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Ask), 7);
  EXPECT_EQ(ob.GetOrderCountAtPrice(100, Side::Ask), 1);
  EXPECT_EQ(ob.GetTotalOrderCount(), 3);
}

TEST(Orderbook_Map, BestBidAndAskAcrossPrices) {
  Orderbook_Map ob;
  EXPECT_EQ(ob.AddOrder(1, Side::Bid, 100, 10, 1), OrderbookError::OK);
  EXPECT_EQ(ob.AddOrder(2, Side::Bid, 101, 10, 1), OrderbookError::OK);
  EXPECT_EQ(ob.AddOrder(3, Side::Bid, 99, 10, 1), OrderbookError::OK);
  EXPECT_EQ(ob.AddOrder(4, Side::Ask, 105, 10, 1), OrderbookError::OK);
  EXPECT_EQ(ob.AddOrder(5, Side::Ask, 103, 10, 1), OrderbookError::OK);
  EXPECT_EQ(ob.AddOrder(6, Side::Ask, 107, 10, 1), OrderbookError::OK);

  EXPECT_EQ(ob.GetBestBid(), 101); // highest bid
  EXPECT_EQ(ob.GetBestAsk(), 103); // lowest ask
}

TEST(Orderbook_Map, DeleteOrder) {
  Orderbook_Map ob;
  EXPECT_EQ(ob.AddOrder(1, Side::Bid, 100, 10, 1), OrderbookError::OK);
  EXPECT_EQ(ob.AddOrder(2, Side::Bid, 100, 5, 2), OrderbookError::OK);

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
  EXPECT_EQ(ob.AddOrder(1, Side::Bid, 100, 10, 1), OrderbookError::OK);
  EXPECT_EQ(ob.AddOrder(2, Side::Bid, 101, 10, 1), OrderbookError::OK);
  EXPECT_EQ(ob.AddOrder(3, Side::Ask, 105, 10, 1), OrderbookError::OK);
  EXPECT_EQ(ob.AddOrder(4, Side::Ask, 103, 10, 1), OrderbookError::OK);

  EXPECT_EQ(ob.DeleteOrder(2), OrderbookError::OK);
  EXPECT_EQ(ob.GetBestBid(), 100);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(101, Side::Bid), 0);

  EXPECT_EQ(ob.DeleteOrder(4), OrderbookError::OK);
  EXPECT_EQ(ob.GetBestAsk(), 105);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(103, Side::Ask), 0);
}

TEST(Orderbook_Map, ReAddAfterDelete) {
  Orderbook_Map ob;
  EXPECT_EQ(ob.AddOrder(1, Side::Bid, 100, 10, 1), OrderbookError::OK);
  EXPECT_EQ(ob.DeleteOrder(1), OrderbookError::OK);
  // id must be reusable after deletion
  EXPECT_EQ(ob.AddOrder(1, Side::Ask, 105, 20, 2), OrderbookError::OK);
  EXPECT_TRUE(ob.HasOrder(1));
  auto order = ob.GetOrder(1);
  ASSERT_TRUE(order.has_value());
  EXPECT_EQ(order->side, Side::Ask);
  EXPECT_EQ(order->price, 105);
  EXPECT_EQ(order->volume, 20);
}

TEST(Orderbook_Map, ModifyOrder) {
  Orderbook_Map ob;
  EXPECT_EQ(ob.AddOrder(1, Side::Bid, 100, 10, 1), OrderbookError::OK);

  EXPECT_EQ(ob.ModifyOrder(1, 4), OrderbookError::OK);
  EXPECT_EQ(ob.GetOrder(1)->volume, 4);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Bid), 4);

  EXPECT_EQ(ob.ModifyOrder(999, 5), OrderbookError::OrderNotFound);
}

TEST(Orderbook_Map, ModifyOrderToZeroDeletes) {
  Orderbook_Map ob;
  EXPECT_EQ(ob.AddOrder(1, Side::Bid, 100, 10, 1), OrderbookError::OK);
  EXPECT_EQ(ob.ModifyOrder(1, 0), OrderbookError::OK);
  EXPECT_FALSE(ob.HasOrder(1));
  EXPECT_EQ(ob.GetTotalOrderCount(), 0);
  EXPECT_EQ(ob.GetBestBid(), std::nullopt);
}

TEST(Orderbook_Map, ModifyOrderIncreaseRejected) {
  Orderbook_Map ob;
  EXPECT_EQ(ob.AddOrder(1, Side::Bid, 100, 10, 1), OrderbookError::OK);
  // reduce-only modify: increasing volume is rejected, order unchanged
  EXPECT_EQ(ob.ModifyOrder(1, 25), OrderbookError::InvalidVolume);
  EXPECT_EQ(ob.ModifyOrder(1, 10), OrderbookError::InvalidVolume); // same size is a no-op, reject
  EXPECT_EQ(ob.GetOrder(1)->volume, 10);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Bid), 10);
}
