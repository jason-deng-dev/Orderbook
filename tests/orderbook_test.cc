#include "orderbook_map.h"
#include "orderbook_slab.h"
#include "orderbook_vector.h"
#include "search_policies.h"
#include <gtest/gtest.h>
#include <optional>



// Expected result for an order that rests on the book without matching.
static void ExpectResting(const MatchResult &r, Volume volume) {
  EXPECT_EQ(r.status, OrderStatus::Resting);
  EXPECT_EQ(r.error, OrderbookError::OK);
  EXPECT_EQ(r.filledVolume, 0);
  EXPECT_EQ(r.restingVolume, volume);
}

namespace {

template <typename Orderbook>
void testInit() {
  Orderbook ob;
  EXPECT_EQ(ob.GetBestBid(), std::nullopt);
  EXPECT_EQ(ob.GetBestAsk(), std::nullopt);
  EXPECT_EQ(ob.GetTotalOrderCount(), 0);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Bid), 0);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Ask), 0);
  EXPECT_EQ(ob.GetOrderCountAtPrice(100, Side::Bid), 0);
  EXPECT_EQ(ob.GetOrderCountAtPrice(100, Side::Ask), 0);
}

template <typename Orderbook>
void testAddOrder() {
  Orderbook ob;
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

template <typename Orderbook>
void testAddOrderAggregatesAtSamePrice() {
  Orderbook ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 1), 10);
  ExpectResting(ob.AddOrder(2, Side::Bid, 100, 5, 2), 5);
  ExpectResting(ob.AddOrder(3, Side::Ask, 105, 7, 1), 7);

  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Bid), 15);
  EXPECT_EQ(ob.GetOrderCountAtPrice(100, Side::Bid), 2);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(105, Side::Ask), 7);
  EXPECT_EQ(ob.GetOrderCountAtPrice(105, Side::Ask), 1);
  EXPECT_EQ(ob.GetTotalOrderCount(), 3);
}

template <typename Orderbook>
void testBestBidAndAskAcrossPrices() {
  Orderbook ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 1), 10);
  ExpectResting(ob.AddOrder(2, Side::Bid, 101, 10, 1), 10);
  ExpectResting(ob.AddOrder(3, Side::Bid, 99, 10, 1), 10);
  ExpectResting(ob.AddOrder(4, Side::Ask, 105, 10, 1), 10);
  ExpectResting(ob.AddOrder(5, Side::Ask, 103, 10, 1), 10);
  ExpectResting(ob.AddOrder(6, Side::Ask, 107, 10, 1), 10);

  EXPECT_EQ(ob.GetBestBid(), 101); // highest bid
  EXPECT_EQ(ob.GetBestAsk(), 103); // lowest ask
}

template <typename Orderbook>
void testDeleteOrder() {
  Orderbook ob;
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

template <typename Orderbook>
void testDeleteLastOrderRemovesPriceLevel() {
  Orderbook ob;
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

template <typename Orderbook>
void testReAddAfterDelete() {
  Orderbook ob;
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

template <typename Orderbook>
void testModifyOrder() {
  Orderbook ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 1), 10);

  EXPECT_EQ(ob.ModifyOrder(1, 4), OrderbookError::OK);
  EXPECT_EQ(ob.GetOrder(1)->volume, 4);
  EXPECT_EQ(ob.GetTotalVolumeAtPrice(100, Side::Bid), 4);

  EXPECT_EQ(ob.ModifyOrder(999, 5), OrderbookError::OrderNotFound);
}

template <typename Orderbook>
void testModifyOrderToZeroDeletes() {
  Orderbook ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 1), 10);
  EXPECT_EQ(ob.ModifyOrder(1, 0), OrderbookError::OK);
  EXPECT_FALSE(ob.HasOrder(1));
  EXPECT_EQ(ob.GetTotalOrderCount(), 0);
  EXPECT_EQ(ob.GetBestBid(), std::nullopt);
}

template <typename Orderbook>
void testModifyOrderIncreaseRejected() {
  Orderbook ob;
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

template <typename Orderbook>
void testFillNoCrossingLiquidity() {
  Orderbook ob;
  ExpectResting(ob.AddOrder(1, Side::Ask, 105, 10, 1), 10);
  // bid below best ask => no match, rests fully
  ExpectResting(ob.AddOrder(2, Side::Bid, 100, 8, 2), 8);

  EXPECT_EQ(ob.GetBestBid(), 100);
  EXPECT_EQ(ob.GetBestAsk(), 105);
  EXPECT_EQ(ob.GetTotalOrderCount(), 2);
}

template <typename Orderbook>
void testFillExactMatchBothRemoved() {
  Orderbook ob;
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

template <typename Orderbook>
void testFillIncomingPartiallyFilledRestsRemainder() {
  Orderbook ob;
  // Order 1 (Ask, price = 101, volume = 4, tradeId = 1)
  ExpectResting(ob.AddOrder(1, Side::Ask, 101, 4, 1), 4);
  // Order 2 (Bid, price = 101, volume = 10, tradeId = 2)
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

template <typename Orderbook>
void testFillRestingPartiallyFilledRemainsResting() {
  Orderbook ob;
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

template <typename Orderbook>
void testFillWalksMultiplePriceLevels() {
  Orderbook ob;
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

template <typename Orderbook>
void testFillStopsAtPriceLimit() {
  Orderbook ob;
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

template <typename Orderbook>
void testFillRespectsFifoWithinLevel() {
  Orderbook ob;
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

template <typename Orderbook>
void testFillSellSideMatchesBids() {
  Orderbook ob;
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

template <typename Orderbook>
void testFillSellSideStopsAboveLimit() {
  Orderbook ob;
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

// ----------------------------- Event log --------------------------------
// The event log is append-only: every AddOrder / DeleteOrder / ModifyOrder
// call appends exactly one OrderEvent describing that operation and its outcome.

static void ExpectAddEvent(const OrderEvent &e, OrderId id, Side side, Price price, Volume volume, TraderId traderId,
                           Volume filledVolume, Volume restingVolume, OrderStatus status,
                           OrderbookError error = OrderbookError::OK) {
  EXPECT_EQ(e.type, EventType::Add);
  EXPECT_EQ(e.orderId, id);
  EXPECT_EQ(e.side, side);
  EXPECT_EQ(e.price, price);
  EXPECT_EQ(e.volume, volume);
  EXPECT_EQ(e.traderId, traderId);
  EXPECT_EQ(e.filledVolume, filledVolume);
  EXPECT_EQ(e.restingVolume, restingVolume);
  EXPECT_EQ(e.orderStatus, status);
  EXPECT_EQ(e.orderbookError, error);
}

static void ExpectCancelEvent(const OrderEvent &e, OrderId id, OrderbookError error) {
  EXPECT_EQ(e.type, EventType::Cancel);
  EXPECT_EQ(e.orderId, id);
  EXPECT_EQ(e.orderbookError, error);
}

static void ExpectModifyEvent(const OrderEvent &e, OrderId id, Volume newVolume, OrderbookError error) {
  EXPECT_EQ(e.type, EventType::Modify);
  EXPECT_EQ(e.orderId, id);
  EXPECT_EQ(e.volume, newVolume);
  EXPECT_EQ(e.orderbookError, error);
}

template <typename Orderbook>
void testEventLogEmptyOnFreshBook() {
  Orderbook ob;
  EXPECT_TRUE(ob.GetEventLog().empty());
}

template <typename Orderbook>
void testEventLogRecordsRestingAdd() {
  Orderbook ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 7), 10);

  const auto &log = ob.GetEventLog();
  ASSERT_EQ(log.size(), 1);
  // nothing matched => nothing filled, whole volume rests
  ExpectAddEvent(log[0], 1, Side::Bid, 100, 10, 7, 0, 10, OrderStatus::Resting);
}

template <typename Orderbook>
void testEventLogRecordsRejectedDuplicateAdd() {
  Orderbook ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 7), 10);
  auto dup = ob.AddOrder(1, Side::Ask, 101, 5, 8);
  ASSERT_EQ(dup.error, OrderbookError::DuplicateId);

  const auto &log = ob.GetEventLog();
  ASSERT_EQ(log.size(), 2);
  ExpectAddEvent(log[1], 1, Side::Ask, 101, 5, 8, 0, 5, OrderStatus::Rejected, OrderbookError::DuplicateId);
}

template <typename Orderbook>
void testEventLogRecordsFilledAdd() {
  Orderbook ob;
  ExpectResting(ob.AddOrder(1, Side::Ask, 101, 10, 1), 10);
  auto r = ob.AddOrder(2, Side::Bid, 101, 10, 2);
  ASSERT_EQ(r.status, OrderStatus::Filled);

  const auto &log = ob.GetEventLog();
  ASSERT_EQ(log.size(), 2);
  ExpectAddEvent(log[1], 2, Side::Bid, 101, 10, 2, 10, 0, OrderStatus::Filled);
}

template <typename Orderbook>
void testEventLogRecordsPartiallyFilledAdd() {
  Orderbook ob;
  ExpectResting(ob.AddOrder(1, Side::Ask, 101, 4, 1), 4);
  auto r = ob.AddOrder(2, Side::Bid, 101, 10, 2);
  ASSERT_EQ(r.status, OrderStatus::PartiallyFilled);

  const auto &log = ob.GetEventLog();
  ASSERT_EQ(log.size(), 2);
  // filledVolume + restingVolume must account for the whole incoming volume
  ExpectAddEvent(log[1], 2, Side::Bid, 101, 10, 2, 4, 6, OrderStatus::PartiallyFilled);
}

template <typename Orderbook>
void testEventLogRecordsSuccessfulCancel() {
  Orderbook ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 7), 10);
  ASSERT_EQ(ob.DeleteOrder(1), OrderbookError::OK);

  const auto &log = ob.GetEventLog();
  ASSERT_EQ(log.size(), 2);
  ExpectCancelEvent(log[1], 1, OrderbookError::OK);
}

template <typename Orderbook>
void testEventLogRecordsCancelOfMissingOrder() {
  Orderbook ob;
  ASSERT_EQ(ob.DeleteOrder(42), OrderbookError::OrderNotFound);

  const auto &log = ob.GetEventLog();
  ASSERT_EQ(log.size(), 1);
  ExpectCancelEvent(log[0], 42, OrderbookError::OrderNotFound);
}

template <typename Orderbook>
void testEventLogRecordsSuccessfulModify() {
  Orderbook ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 7), 10);
  ASSERT_EQ(ob.ModifyOrder(1, 4), OrderbookError::OK);

  const auto &log = ob.GetEventLog();
  ASSERT_EQ(log.size(), 2);
  ExpectModifyEvent(log[1], 1, 4, OrderbookError::OK);
}

template <typename Orderbook>
void testEventLogRecordsRejectedModify() {
  Orderbook ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 7), 10);
  ASSERT_EQ(ob.ModifyOrder(1, 25), OrderbookError::InvalidVolume);

  const auto &log = ob.GetEventLog();
  ASSERT_EQ(log.size(), 2);
  ExpectModifyEvent(log[1], 1, 25, OrderbookError::InvalidVolume);
}

template <typename Orderbook>
void testEventLogModifyToZeroIsRecordedAsCancel() {
  Orderbook ob;
  ExpectResting(ob.AddOrder(1, Side::Bid, 100, 10, 7), 10);
  ASSERT_EQ(ob.ModifyOrder(1, 0), OrderbookError::OK);

  const auto &log = ob.GetEventLog();
  ASSERT_EQ(log.size(), 2);
  // ModifyOrder(0) delegates to DeleteOrder, so it audits as a Cancel
  ExpectCancelEvent(log[1], 1, OrderbookError::OK);
}

template <typename Orderbook>
void testEventLogPreservesOperationOrder() {
  Orderbook ob;
  ASSERT_EQ(ob.AddOrder(1, Side::Bid, 100, 10, 1).status, OrderStatus::Resting);
  ASSERT_EQ(ob.ModifyOrder(1, 5), OrderbookError::OK);
  ASSERT_EQ(ob.AddOrder(2, Side::Ask, 100, 5, 2).status, OrderStatus::Filled);
  ASSERT_EQ(ob.DeleteOrder(2), OrderbookError::OrderNotFound); // already filled, nothing to cancel

  const auto &log = ob.GetEventLog();
  ASSERT_EQ(log.size(), 4);
  EXPECT_EQ(log[0].type, EventType::Add);
  EXPECT_EQ(log[1].type, EventType::Modify);
  EXPECT_EQ(log[2].type, EventType::Add);
  EXPECT_EQ(log[3].type, EventType::Cancel);

  EXPECT_EQ(log[0].orderId, 1);
  EXPECT_EQ(log[1].orderId, 1);
  EXPECT_EQ(log[2].orderId, 2);
  EXPECT_EQ(log[3].orderId, 2);
}

template <typename Orderbook>
void testEventLogTimestampsAreMonotonic() {
  Orderbook ob;
  ASSERT_EQ(ob.AddOrder(1, Side::Bid, 100, 10, 1).status, OrderStatus::Resting);
  ASSERT_EQ(ob.AddOrder(2, Side::Bid, 99, 10, 1).status, OrderStatus::Resting);
  ASSERT_EQ(ob.ModifyOrder(1, 5), OrderbookError::OK);
  ASSERT_EQ(ob.DeleteOrder(2), OrderbookError::OK);
  ASSERT_EQ(ob.DeleteOrder(999), OrderbookError::OrderNotFound);

  const auto &log = ob.GetEventLog();
  ASSERT_EQ(log.size(), 5);
  EXPECT_GT(log[0].ts, 0);
  for (size_t i = 1; i < log.size(); ++i) {
    EXPECT_LE(log[i - 1].ts, log[i].ts);
  }
}

template <typename Orderbook>
void testOrderbook() {
  std::cerr << "RUN testInit\n";
  testInit<Orderbook>();
  std::cerr << "RUN testAddOrder\n";
  testAddOrder<Orderbook>();
  testAddOrderAggregatesAtSamePrice<Orderbook>();
  testBestBidAndAskAcrossPrices<Orderbook>();
  testDeleteOrder<Orderbook>();
  testDeleteLastOrderRemovesPriceLevel<Orderbook>();
  testReAddAfterDelete<Orderbook>();
  testModifyOrder<Orderbook>();
  testModifyOrderToZeroDeletes<Orderbook>();
  testModifyOrderIncreaseRejected<Orderbook>();

  testFillNoCrossingLiquidity<Orderbook>();
  testFillExactMatchBothRemoved<Orderbook>();
  testFillIncomingPartiallyFilledRestsRemainder<Orderbook>();
  testFillRestingPartiallyFilledRemainsResting<Orderbook>();
  testFillWalksMultiplePriceLevels<Orderbook>();
  testFillStopsAtPriceLimit<Orderbook>();
  testFillRespectsFifoWithinLevel<Orderbook>();
  testFillSellSideMatchesBids<Orderbook>();
  testFillSellSideStopsAboveLimit<Orderbook>();

  testEventLogEmptyOnFreshBook<Orderbook>();
  testEventLogRecordsRestingAdd<Orderbook>();
  testEventLogRecordsRejectedDuplicateAdd<Orderbook>();
  testEventLogRecordsFilledAdd<Orderbook>();
  testEventLogRecordsPartiallyFilledAdd<Orderbook>();
  testEventLogRecordsSuccessfulCancel<Orderbook>();
  testEventLogRecordsCancelOfMissingOrder<Orderbook>();
  testEventLogRecordsSuccessfulModify<Orderbook>();
  testEventLogRecordsRejectedModify<Orderbook>();
  testEventLogModifyToZeroIsRecordedAsCancel<Orderbook>();
  testEventLogPreservesOperationOrder<Orderbook>();
  testEventLogTimestampsAreMonotonic<Orderbook>();
}

} // namespace

TEST(Orderbook_Vector, testOrderbook) { testOrderbook<Orderbook_Vector<LinearSearch>>(); }
TEST(Orderbook_Map, testOrderbook) { testOrderbook<Orderbook_Map>(); }
TEST(Orderbook_Slab, testOrderbook) { testOrderbook<Orderbook_Slab<LinearSearch, 10>>(); }

