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
// BENCHMARK(BM_BuyEmptyBook);

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
// BENCHMARK(BM_SellEmptyBook);



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
    s.changeInventoryAmount(&ob, 10);
    ob.sell(&s, 10, 10);
    state.ResumeTiming();
  }
};
// BENCHMARK(BM_BuyCrossSingleLevel);

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
    s.changeInventoryAmount(&ob, 10);
    ob.buy(&b, 10, 10);
    state.ResumeTiming();
  }
};
BENCHMARK(BM_SellCrossSingleLevel);








// Taker multi-level walk: buy sweeping 2-3 ask levels
static void BM_BuyCrossMultiLevel();
// Partial fill: taker partially consumed, remainder rests
static void BM_BuyPartialFill();



// Cancel: leaveing price level alive
static void BM_CancelKeepLevel();
// Cancel: cancel emptying a price level (erase that level)
static void BM_CancelEmptyLevel();
