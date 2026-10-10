#include "search_policies.h"
#include "types.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <utility>
#include <vector>

namespace {

using Level = std::pair<Price, PriceLevel>;
using Levels = std::vector<Level>;

// Bids are held in ascending order, so the best bid (highest price) sits at the
// back of the vector. Asks are held in descending order, so the best ask
// (lowest price) sits at the back. Prices are deliberately sparse: a gap
// between two levels is what makes the "price fits between two levels" insert
// position expressible at all.
const Levels bidLevels = {{95, {}}, {100, {}}, {105, {}}};
const Levels askLevels = {{110, {}}, {105, {}}, {100, {}}};

using BidCompare = std::less<Price>;    // ascending
using AskCompare = std::greater<Price>; // descending

// Every level in these generated ranges has a distinct price, and the lowest
// price is always 100. Both directions use the same price set; only the order
// differs.
constexpr Price kLowestLevelPrice = 100;

// Ascending bid range: 100, 101, ... Best bid (highest price) at the back.
Levels MakeBidLevels(std::size_t levelCount) {
  Levels levels;
  levels.reserve(levelCount);
  for (std::size_t i = 0; i < levelCount; ++i) {
    levels.emplace_back(static_cast<Price>(kLowestLevelPrice + i), PriceLevel{});
  }
  return levels;
}

// Descending ask range: 100 + count - 1, ... , 100. Best ask (lowest price) at
// the back.
Levels MakeAskLevels(std::size_t levelCount) {
  Levels levels;
  levels.reserve(levelCount);
  for (std::size_t i = 0; i < levelCount; ++i) {
    levels.emplace_back(static_cast<Price>(kLowestLevelPrice + levelCount - 1 - i), PriceLevel{});
  }
  return levels;
}

// Reduces a policy result to (result, index) so that assertions compare plain
// values: enums and indexes print usefully on failure, iterators do not.
// index == levels.size() is the one-past-the-back position, i.e. "insert at the
// end".
template <typename Policy, typename Compare>
std::pair<SearchResult, std::ptrdiff_t> SearchIndex(const Levels &levels, Price price, Compare comp) {
  auto [result, it] = Policy::search(levels.begin(), levels.end(), price, comp);
  return {result, it - levels.begin()};
}

// ---------------------------------------------------------------------------
// Independent reference
// ---------------------------------------------------------------------------

// A deliberately naive reference for the insert position: the first index whose
// key does not compare "before" the searched price, or keys.size() when no such
// index exists. That is the lower_bound position under comp.
//
// Written as a linear scan over plain prices so that it shares no code and no
// element-projection logic with any of the three policies. If a policy and this
// reference disagree, the policy is wrong.
template <typename Compare>
std::ptrdiff_t ReferenceIndex(const std::vector<Price> &sortedKeys, Price price, Compare comp) {
  for (std::size_t i = 0; i < sortedKeys.size(); ++i) {
    if (!comp(sortedKeys[i], price)) {
      return static_cast<std::ptrdiff_t>(i);
    }
  }
  return static_cast<std::ptrdiff_t>(sortedKeys.size());
}

std::vector<Price> ExtractKeys(const Levels &levels) {
  std::vector<Price> keys;
  keys.reserve(levels.size());
  for (const Level &level : levels) {
    keys.push_back(level.first);
  }
  return keys;
}

// Drives every price in [firstPrice, lastPrice] through all three policies and
// against the reference. This is both an agreement check (all three must equal
// the same expected value) and a correctness check (that value must be the one
// the reference computes).
template <typename Compare>
void ExpectAllPoliciesMatchReference(const Levels &levels, Price firstPrice, Price lastPrice, Compare comp) {
  const std::vector<Price> keys = ExtractKeys(levels);

  for (Price price = firstPrice; price <= lastPrice; ++price) {
    const std::ptrdiff_t expectedIndex = ReferenceIndex(keys, price, comp);
    const bool present = expectedIndex < static_cast<std::ptrdiff_t>(keys.size()) &&
                         keys[static_cast<std::size_t>(expectedIndex)] == price;
    const SearchResult expectedResult = present ? SearchResult::found : SearchResult::notFound;

    const auto linear = SearchIndex<LinearSearch>(levels, price, comp);
    const auto binary = SearchIndex<BinarySearch>(levels, price, comp);
    const auto branchless = SearchIndex<BranchlessBinarySearch>(levels, price, comp);

    EXPECT_EQ(linear.first, expectedResult) << "LinearSearch price " << price;
    EXPECT_EQ(linear.second, expectedIndex) << "LinearSearch price " << price;

    EXPECT_EQ(binary.first, expectedResult) << "BinarySearch price " << price;
    EXPECT_EQ(binary.second, expectedIndex) << "BinarySearch price " << price;

    EXPECT_EQ(branchless.first, expectedResult) << "BranchlessBinarySearch price " << price;
    EXPECT_EQ(branchless.second, expectedIndex) << "BranchlessBinarySearch price " << price;
  }
}

// ---------------------------------------------------------------------------
// Typed suite: every policy runs the same battery
// ---------------------------------------------------------------------------

template <typename Policy>
class SearchPolicyTest : public ::testing::Test {};

using SearchPolicies = ::testing::Types<LinearSearch, BinarySearch, BranchlessBinarySearch>;
TYPED_TEST_SUITE(SearchPolicyTest, SearchPolicies);

// ---------------------------------------------------------------------------
// Empty range
// ---------------------------------------------------------------------------

// Nothing to search. The result must report notFound and the returned iterator
// must be the back position, so that inserting there is a plain push_back.
TYPED_TEST(SearchPolicyTest, EmptyRangeReturnsNotFoundAtBack) {
  const Levels empty;

  const auto result = SearchIndex<TypeParam>(empty, 100, BidCompare{});

  EXPECT_EQ(result.first, SearchResult::notFound);
  EXPECT_EQ(result.second, 0);
}

// ---------------------------------------------------------------------------
// Price present -- bids
// ---------------------------------------------------------------------------

TYPED_TEST(SearchPolicyTest, FoundBidLevelAtFront) {
  const auto result = SearchIndex<TypeParam>(bidLevels, 95, BidCompare{});

  EXPECT_EQ(result.first, SearchResult::found);
  EXPECT_EQ(result.second, 0);
}

TYPED_TEST(SearchPolicyTest, FoundBidLevelInMiddle) {
  const auto result = SearchIndex<TypeParam>(bidLevels, 100, BidCompare{});

  EXPECT_EQ(result.first, SearchResult::found);
  EXPECT_EQ(result.second, 1);
}

TYPED_TEST(SearchPolicyTest, FoundBidLevelAtBack) {
  const auto result = SearchIndex<TypeParam>(bidLevels, 105, BidCompare{});

  EXPECT_EQ(result.first, SearchResult::found);
  EXPECT_EQ(result.second, 2);
}

// ---------------------------------------------------------------------------
// Price absent -- bid insert position
// ---------------------------------------------------------------------------

// Better than every resting bid: it becomes the new best bid, so it belongs at
// the back of an ascending range.
TYPED_TEST(SearchPolicyTest, BidAboveEveryLevelInsertsAtBack) {
  const auto result = SearchIndex<TypeParam>(bidLevels, 110, BidCompare{});

  EXPECT_EQ(result.first, SearchResult::notFound);
  EXPECT_EQ(result.second, 3);
}

// Worse than every resting bid: the back position is not right, the front is.
TYPED_TEST(SearchPolicyTest, BidBelowEveryLevelInsertsAtFront) {
  const auto result = SearchIndex<TypeParam>(bidLevels, 90, BidCompare{});

  EXPECT_EQ(result.first, SearchResult::notFound);
  EXPECT_EQ(result.second, 0);
}

// 97 sits above 95 and below 100. Inserting before index 1 keeps the range
// ascending: 95, 97, 100, 105.
TYPED_TEST(SearchPolicyTest, BidBetweenLevelsInsertsBeforeUpperBid) {
  const auto result = SearchIndex<TypeParam>(bidLevels, 97, BidCompare{});

  EXPECT_EQ(result.first, SearchResult::notFound);
  EXPECT_EQ(result.second, 1);
}

// One level covers the three cases that a multi-level range can hide: the
// boundary between "found" and "not found at an end".
TYPED_TEST(SearchPolicyTest, SingleLevelBidHandlesMatchAboveAndBelow) {
  const Levels single = {{100, {}}};

  const auto match = SearchIndex<TypeParam>(single, 100, BidCompare{});
  EXPECT_EQ(match.first, SearchResult::found) << "exact match on the only level";
  EXPECT_EQ(match.second, 0) << "exact match on the only level";

  const auto above = SearchIndex<TypeParam>(single, 105, BidCompare{});
  EXPECT_EQ(above.first, SearchResult::notFound) << "above the only level becomes the best bid";
  EXPECT_EQ(above.second, 1) << "above the only level becomes the best bid";

  const auto below = SearchIndex<TypeParam>(single, 95, BidCompare{});
  EXPECT_EQ(below.first, SearchResult::notFound) << "below the only level";
  EXPECT_EQ(below.second, 0) << "below the only level";
}

// ---------------------------------------------------------------------------
// Price present -- asks
// ---------------------------------------------------------------------------

TYPED_TEST(SearchPolicyTest, FoundAskLevelAtFront) {
  const auto result = SearchIndex<TypeParam>(askLevels, 110, AskCompare{});

  EXPECT_EQ(result.first, SearchResult::found);
  EXPECT_EQ(result.second, 0);
}

TYPED_TEST(SearchPolicyTest, FoundAskLevelInMiddle) {
  const auto result = SearchIndex<TypeParam>(askLevels, 105, AskCompare{});

  EXPECT_EQ(result.first, SearchResult::found);
  EXPECT_EQ(result.second, 1);
}

TYPED_TEST(SearchPolicyTest, FoundAskLevelAtBack) {
  const auto result = SearchIndex<TypeParam>(askLevels, 100, AskCompare{});

  EXPECT_EQ(result.first, SearchResult::found);
  EXPECT_EQ(result.second, 2);
}

// ---------------------------------------------------------------------------
// Price absent -- ask insert position
// ---------------------------------------------------------------------------

// The ordering is reversed relative to bids, so the meaning of the ends flips.
// A cheaper ask than every resting ask is the new best ask and belongs at the
// back.
TYPED_TEST(SearchPolicyTest, AskBelowEveryLevelInsertsAtBack) {
  const auto result = SearchIndex<TypeParam>(askLevels, 95, AskCompare{});

  EXPECT_EQ(result.first, SearchResult::notFound);
  EXPECT_EQ(result.second, 3);
}

TYPED_TEST(SearchPolicyTest, AskAboveEveryLevelInsertsAtFront) {
  const auto result = SearchIndex<TypeParam>(askLevels, 115, AskCompare{});

  EXPECT_EQ(result.first, SearchResult::notFound);
  EXPECT_EQ(result.second, 0);
}

// 102 sits below 105 and above 100. Inserting before index 2 keeps the range
// descending: 110, 105, 102, 100.
TYPED_TEST(SearchPolicyTest, AskBetweenLevelsInsertsBeforeLowerAsk) {
  const auto result = SearchIndex<TypeParam>(askLevels, 102, AskCompare{});

  EXPECT_EQ(result.first, SearchResult::notFound);
  EXPECT_EQ(result.second, 2);
}

// The mirror of SingleLevelBidHandlesMatchAboveAndBelow. The comparison
// direction is inverted, so the two ends swap meaning: a dearer ask is worse
// and goes to the front, a cheaper ask is better and goes to the back.
TYPED_TEST(SearchPolicyTest, SingleLevelAskHandlesMatchAboveAndBelow) {
  const Levels single = {{100, {}}};

  const auto match = SearchIndex<TypeParam>(single, 100, AskCompare{});
  EXPECT_EQ(match.first, SearchResult::found) << "exact match on the only level";
  EXPECT_EQ(match.second, 0) << "exact match on the only level";

  const auto above = SearchIndex<TypeParam>(single, 105, AskCompare{});
  EXPECT_EQ(above.first, SearchResult::notFound) << "above the only level";
  EXPECT_EQ(above.second, 0) << "above the only level";

  const auto below = SearchIndex<TypeParam>(single, 95, AskCompare{});
  EXPECT_EQ(below.first, SearchResult::notFound) << "below the only level becomes the best ask";
  EXPECT_EQ(below.second, 1) << "below the only level becomes the best ask";
}

// ---------------------------------------------------------------------------
// Whole-sweep invariants
// ---------------------------------------------------------------------------

// The result flag must be exactly "the price is present". A policy that scans
// from one end can be tempted to report found for a neighbouring level; this
// catches that for every price in and around the range.
TYPED_TEST(SearchPolicyTest, FoundFlagMatchesPresenceAcrossBidSweep) {
  for (Price price = 85; price <= 115; ++price) {
    const bool present = std::any_of(bidLevels.begin(), bidLevels.end(),
                                     [price](const Level &level) { return level.first == price; });
    const auto result = SearchIndex<TypeParam>(bidLevels, price, BidCompare{});

    EXPECT_EQ(result.first == SearchResult::found, present) << "bid price " << price;
  }
}

TYPED_TEST(SearchPolicyTest, FoundFlagMatchesPresenceAcrossAskSweep) {
  for (Price price = 95; price <= 115; ++price) {
    const bool present = std::any_of(askLevels.begin(), askLevels.end(),
                                     [price](const Level &level) { return level.first == price; });
    const auto result = SearchIndex<TypeParam>(askLevels, price, AskCompare{});

    EXPECT_EQ(result.first == SearchResult::found, present) << "ask price " << price;
  }
}

// The contract that actually matters to AddOrder: inserting at the returned
// position leaves the range sorted, and a found result points at a level whose
// price equals the searched price. Checked for every price, present or not.
//
// Note this test does not verify the result flag: inserting a duplicate price
// still leaves the range sorted, so a policy that reported notFound for a
// present price would pass here. The flag is covered by the sweeps above and by
// the reference checks below.
TYPED_TEST(SearchPolicyTest, InsertAtReturnedPositionKeepsBidRangeSorted) {
  for (Price price = 90; price <= 110; ++price) {
    Levels levels = bidLevels;
    const auto [result, it] = TypeParam::search(levels.begin(), levels.end(), price, BidCompare{});

    if (result == SearchResult::found) {
      EXPECT_EQ(it->first, price) << "bid price " << price;
      continue;
    }

    levels.insert(it, {price, PriceLevel{}});

    EXPECT_TRUE(std::is_sorted(levels.begin(), levels.end(),
                               [](const Level &a, const Level &b) { return a.first < b.first; }))
        << "bid price " << price;
    EXPECT_EQ(levels.size(), std::size_t{4}) << "bid price " << price;
  }
}

TYPED_TEST(SearchPolicyTest, InsertAtReturnedPositionKeepsAskRangeSorted) {
  for (Price price = 95; price <= 115; ++price) {
    Levels levels = askLevels;
    const auto [result, it] = TypeParam::search(levels.begin(), levels.end(), price, AskCompare{});

    if (result == SearchResult::found) {
      EXPECT_EQ(it->first, price) << "ask price " << price;
      continue;
    }

    levels.insert(it, {price, PriceLevel{}});

    EXPECT_TRUE(std::is_sorted(levels.begin(), levels.end(),
                               [](const Level &a, const Level &b) { return a.first > b.first; }))
        << "ask price " << price;
    EXPECT_EQ(levels.size(), std::size_t{4}) << "ask price " << price;
  }
}

// ---------------------------------------------------------------------------
// Cross-policy agreement, and correctness against the reference
// ---------------------------------------------------------------------------

TEST(SearchPolicyReferenceTest, MatchesReferenceOnBidPreset) {
  ExpectAllPoliciesMatchReference(bidLevels, 90, 110, BidCompare{});
}

TEST(SearchPolicyReferenceTest, MatchesReferenceOnAskPreset) {
  ExpectAllPoliciesMatchReference(askLevels, 95, 115, AskCompare{});
}

TEST(SearchPolicyReferenceTest, MatchesReferenceOnEmptyRange) {
  ExpectAllPoliciesMatchReference(Levels{}, 95, 105, BidCompare{});
}

// Range size drives which branches the two binary policies take: whether a
// halving is exact, whether the step is the full half or the remainder, and how
// many probes a search costs. A three-element range -- the size every other
// test in this file uses -- exercises almost none of that. Sizes here cover
// 0/1 boundaries, even and odd counts, powers of two either side, and a range
// large enough for the halving chain to run to its end.
TEST(SearchPolicyReferenceTest, MatchesReferenceAcrossRangeSizes) {
  for (std::size_t levelCount : {std::size_t{1}, std::size_t{2}, std::size_t{3}, std::size_t{4}, std::size_t{5},
                                 std::size_t{8}, std::size_t{9}, std::size_t{16}, std::size_t{64}}) {
    SCOPED_TRACE("level count " + std::to_string(levelCount));

    const Price lastLevelPrice = static_cast<Price>(kLowestLevelPrice + levelCount - 1);
    // One below the cheapest level and one above the dearest, so the sweep
    // covers "worse than all", every exact match, every gap, and "better than
    // all".
    const Price firstPrice = kLowestLevelPrice - 2;
    const Price lastPrice = lastLevelPrice + 2;

    ExpectAllPoliciesMatchReference(MakeBidLevels(levelCount), firstPrice, lastPrice, BidCompare{});
    ExpectAllPoliciesMatchReference(MakeAskLevels(levelCount), firstPrice, lastPrice, AskCompare{});
  }
}

} // namespace
