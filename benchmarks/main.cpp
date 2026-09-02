#include "orderbook.h"
#include "orders.h"
#include "trader.h"
#include <benchmark/benchmark.h>

// buy/sell into empty book
static void BM_BuyEmptyBook(benchmark::State &state) {
  Orderbook ob("stock1");
  Trader t("t", 1000000);
  int currTradeId = 0;

  for (auto _ : state) {
    ob.buy(&t, 100, 10);
    benchmark::DoNotOptimize(ob);

    // End of timed section
    state.PauseTiming();
    ob.cancelBuy(&t, currTradeId++, 100, 10);
    state.ResumeTiming();
  }
}
BENCHMARK(BM_BuyEmptyBook);

static void BM_SellEmptyBook(benchmark::State &state) {
  Orderbook ob("stock1");
  Trader t("t", 100000);
  int currTradeId = 0;
  t.changeInventoryAmount(&ob, 100);

  for (auto _ : state) {
    ob.sell(&t, 100, 10);
    benchmark::DoNotOptimize(ob);

    // End of timed section
    state.PauseTiming();
    ob.cancelSell(&t, currTradeId++, 100, 10)  ;
    state.ResumeTiming();
  }
}
BENCHMARK(BM_SellEmptyBook);



// Taker full-fill: buy/sell crossing one level single fill

static void BM_BuyCrossSingleLevel(benchmark::State &state) {
  Orderbook ob("stock1");
  Trader b("b", 100);
  Trader s("s", 0);
  s.changeInventoryAmount(&ob, 10);
  ob.sell(&s, 10, 10);

  for (auto _ : state) {
    ob.buy(&b, 10, 10);

    state.PauseTiming();
    b.changeBalance(100);
    s.changeBalance(-100);
    s.changeInventoryAmount(&ob, 10);
    ob.sell(&s, 10, 10);
    state.ResumeTiming();
  }
};
BENCHMARK(BM_BuyCrossSingleLevel);

static void BM_SellCrossSingleLevel(benchmark::State &state) {
  Orderbook ob("stock1");
  Trader b("b", 100);
  Trader s("s", 0);
  s.changeInventoryAmount(&ob, 10);
  ob.buy(&b, 10, 10);

  for (auto _ : state) {
    ob.sell(&s, 10, 10);

    state.PauseTiming();
    b.changeBalance(100);
    s.changeBalance(-100);
    s.changeInventoryAmount(&ob, 10);
    ob.buy(&b, 10, 10);
    state.ResumeTiming();
  }
};
BENCHMARK(BM_SellCrossSingleLevel);



// Taker multi-level walk: buy/sell sweeping 3 ask levels
static void BM_BuyCrossThreeLevels(benchmark::State &state) {
  Orderbook ob("stock1");
  // taker locks 9 * 35 = 315, needs >= that upfront
  Trader b("b", 1000);
  Trader s("s", 0);
  s.changeInventoryAmount(&ob, 9);
  // resting asks 3 @ 10, 3 @ 20, 3 @ 30
  ob.sell(&s, 10, 3);
  ob.sell(&s, 20, 3);
  ob.sell(&s, 30, 3);

  for (auto _ : state) {
    // sweeps all 3 levels in one op: fills at 10/20/30, refunds 25/15/5 per unit
    ob.buy(&b, 35, 9);

    state.PauseTiming();
    // net spend = 3*(10+20+30) = 180; seller earned the same
    b.changeBalance(180);
    s.changeBalance(-180);
    s.changeInventoryAmount(&ob, 9);
    ob.sell(&s, 10, 3);
    ob.sell(&s, 20, 3);
    ob.sell(&s, 30, 3);
    state.ResumeTiming();
  }
};
BENCHMARK(BM_BuyCrossThreeLevels);

static void BM_SellCrossThreeLevels(benchmark::State &state) {
  Orderbook ob("stock1");
  Trader b("b", 1000);
  Trader s("s", 0);
  s.changeInventoryAmount(&ob, 9);
  // resting bids 3 @ 30, 3 @ 20, 3 @ 10 (lock 180 upfront)
  ob.buy(&b, 30, 3);
  ob.buy(&b, 20, 3);
  ob.buy(&b, 10, 3);

  for (auto _ : state) {
    // crosses all 3 bid levels in one op, fills at resting bid prices
    ob.sell(&s, 10, 9);

    state.PauseTiming();
    // buyer's locked 180 was spent, seller earned 180
    b.changeBalance(180);
    s.changeBalance(-180);
    s.changeInventoryAmount(&ob, 9);
    ob.buy(&b, 30, 3);
    ob.buy(&b, 20, 3);
    ob.buy(&b, 10, 3);
    state.ResumeTiming();
  }
};
BENCHMARK(BM_SellCrossThreeLevels);



// Partial fill: taker order partially consumed, remainder rests
static void BM_BuyPartialFill(benchmark::State &state) {
  Orderbook ob("stock1");
  Trader b("b", 100);
  Trader s("s", 0);
  s.changeInventoryAmount(&ob, 3);
  // resting ask 3 @ 10 (id 0)
  ob.sell(&s, 10, 3);
  // next order id after the resting ask
  int buyId = 1;

  for (auto _ : state) {
    // buys 5, only 3 fill -> leftover bid 2 @ 10 rests
    ob.buy(&b, 10, 5);

    state.PauseTiming();
    // remove leftover (refunds 20), net spend was 3*10 = 30
    ob.cancelBuy(&b, buyId, 10, 2);
    b.changeBalance(30);
    s.changeBalance(-30);
    s.changeInventoryAmount(&ob, 3);
    ob.sell(&s, 10, 3);
    state.ResumeTiming();
    buyId += 2;
  }
};
BENCHMARK(BM_BuyPartialFill);

static void BM_SellPartialFill(benchmark::State &state) {
  Orderbook ob("stock1");
  Trader rb("rb", 100);   // resting buyer
  Trader ts("ts", 0);     // taker seller
  ts.changeInventoryAmount(&ob, 100);
  // resting bid 3 @ 10 (id 0)
  ob.buy(&rb, 10, 3);
  int sellId = 1;

  for (auto _ : state) {
    // sells 5, only 3 fill -> leftover ask 2 @ 10 rests
    ob.sell(&ts, 10, 5);

    state.PauseTiming();
    // remove leftover (returns 2 stock), resting buyer spent 3*10
    ob.cancelSell(&ts, sellId, 10, 2);
    rb.changeBalance(30);
    ts.changeBalance(-30);
    ts.changeInventoryAmount(&ob, 3);   // restock the 3 units actually sold
    ob.buy(&rb, 10, 3);
    state.ResumeTiming();
    sellId += 2;
  }
};
BENCHMARK(BM_SellPartialFill);



// Cancel: order removed but price level stays (other order resting at price)
static void BM_CancelBuyKeepLevel(benchmark::State &state) {
  Orderbook ob("stock1");
  Trader t("t", 1000000);
  Trader t2("t2", 1000000);
  // permanent resting order keeps the level alive (id 0)
  ob.buy(&t2, 50, 10);
  int tradeId = 1;

  for (auto _ : state) {
    state.PauseTiming();
    ob.buy(&t, 50, 10);
    state.ResumeTiming();

    ob.cancelBuy(&t, tradeId, 50, 10);
    benchmark::DoNotOptimize(ob);
    ++tradeId;
  }
};
BENCHMARK(BM_CancelBuyKeepLevel);

static void BM_CancelSellKeepLevel(benchmark::State &state) {
  Orderbook ob("stock1");
  Trader t("t", 0);
  Trader t2("t2", 0);
  t.changeInventoryAmount(&ob, 200);
  t2.changeInventoryAmount(&ob, 200);
  // permanent resting order keeps the level alive (id 0)
  ob.sell(&t2, 50, 10);
  int tradeId = 1;

  for (auto _ : state) {
    state.PauseTiming();
    ob.sell(&t, 50, 10);
    state.ResumeTiming();

    ob.cancelSell(&t, tradeId, 50, 10);
    benchmark::DoNotOptimize(ob);
    ++tradeId;
  }
};
BENCHMARK(BM_CancelSellKeepLevel);

// Cancel: cancel empties the price level (level erased from map)
static void BM_CancelBuyEmptyLevel(benchmark::State &state) {
  Orderbook ob("stock1");
  Trader t("t", 1000000);
  int tradeId = 0;

  for (auto _ : state) {
    state.PauseTiming();
    ob.buy(&t, 50, 10);
    state.ResumeTiming();

    ob.cancelBuy(&t, tradeId, 50, 10);
    benchmark::DoNotOptimize(ob);
    ++tradeId;
  }
};
BENCHMARK(BM_CancelBuyEmptyLevel);

static void BM_CancelSellEmptyLevel(benchmark::State &state) {
  Orderbook ob("stock1");
  Trader t("t", 0);
  t.changeInventoryAmount(&ob, 200);
  int tradeId = 0;

  for (auto _ : state) {
    state.PauseTiming();
    ob.sell(&t, 50, 10);
    state.ResumeTiming();

    ob.cancelSell(&t, tradeId, 50, 10);
    benchmark::DoNotOptimize(ob);
    ++tradeId;
  }
};
BENCHMARK(BM_CancelSellEmptyLevel);
