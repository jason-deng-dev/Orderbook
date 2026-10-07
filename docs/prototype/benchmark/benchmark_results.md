```bash
./build/orderbook_bench --benchmark_min_time=0.5s --benchmark_report_aggregates_only=true
2026-09-03T10:28:23+08:00
Running ./build/orderbook_bench
Run on (16 X 4621.06 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 98304 KiB (x1)
Load Average: 0.90, 0.74, 0.74
-------------------------------------------------------------------------------------
Benchmark                                           Time             CPU   Iterations
-------------------------------------------------------------------------------------
BM_BuyEmptyBook/repeats:100_mean                  142 ns          142 ns          100
BM_BuyEmptyBook/repeats:100_median                142 ns          142 ns          100
BM_BuyEmptyBook/repeats:100_stddev              0.539 ns        0.523 ns          100
BM_BuyEmptyBook/repeats:100_cv                   0.38 %          0.37 %           100
BM_BuyEmptyBook/repeats:100_p50                   142 ns          142 ns          100
BM_BuyEmptyBook/repeats:100_p99                   144 ns          145 ns          100
BM_BuyEmptyBook/repeats:100_p99.9                 144 ns          145 ns          100
BM_SellEmptyBook/repeats:100_mean                 147 ns          147 ns          100
BM_SellEmptyBook/repeats:100_median               147 ns          147 ns          100
BM_SellEmptyBook/repeats:100_stddev             0.573 ns        0.554 ns          100
BM_SellEmptyBook/repeats:100_cv                  0.39 %          0.38 %           100
BM_SellEmptyBook/repeats:100_p50                  147 ns          147 ns          100
BM_SellEmptyBook/repeats:100_p99                  149 ns          150 ns          100
BM_SellEmptyBook/repeats:100_p99.9                149 ns          150 ns          100
BM_BuyCrossSingleLevel/repeats:100_mean           234 ns          235 ns          100
BM_BuyCrossSingleLevel/repeats:100_median         234 ns          234 ns          100
BM_BuyCrossSingleLevel/repeats:100_stddev        1.65 ns         1.63 ns          100
BM_BuyCrossSingleLevel/repeats:100_cv            0.71 %          0.69 %           100
BM_BuyCrossSingleLevel/repeats:100_p50            234 ns          234 ns          100
BM_BuyCrossSingleLevel/repeats:100_p99            239 ns          239 ns          100
BM_BuyCrossSingleLevel/repeats:100_p99.9          239 ns          239 ns          100
BM_SellCrossSingleLevel/repeats:100_mean          242 ns          243 ns          100
BM_SellCrossSingleLevel/repeats:100_median        242 ns          243 ns          100
BM_SellCrossSingleLevel/repeats:100_stddev       1.80 ns         1.80 ns          100
BM_SellCrossSingleLevel/repeats:100_cv           0.74 %          0.74 %           100
BM_SellCrossSingleLevel/repeats:100_p50           242 ns          243 ns          100
BM_SellCrossSingleLevel/repeats:100_p99           247 ns          248 ns          100
BM_SellCrossSingleLevel/repeats:100_p99.9         247 ns          248 ns          100
BM_BuyCrossThreeLevels/repeats:100_mean           358 ns          357 ns          100
BM_BuyCrossThreeLevels/repeats:100_median         357 ns          357 ns          100
BM_BuyCrossThreeLevels/repeats:100_stddev        2.31 ns         2.29 ns          100
BM_BuyCrossThreeLevels/repeats:100_cv            0.65 %          0.64 %           100
BM_BuyCrossThreeLevels/repeats:100_p50            357 ns          357 ns          100
BM_BuyCrossThreeLevels/repeats:100_p99            364 ns          364 ns          100
BM_BuyCrossThreeLevels/repeats:100_p99.9          364 ns          364 ns          100
BM_SellCrossThreeLevels/repeats:100_mean          369 ns          369 ns          100
BM_SellCrossThreeLevels/repeats:100_median        368 ns          369 ns          100
BM_SellCrossThreeLevels/repeats:100_stddev       2.41 ns         2.37 ns          100
BM_SellCrossThreeLevels/repeats:100_cv           0.65 %          0.64 %           100
BM_SellCrossThreeLevels/repeats:100_p50           368 ns          369 ns          100
BM_SellCrossThreeLevels/repeats:100_p99           376 ns          376 ns          100
BM_SellCrossThreeLevels/repeats:100_p99.9         376 ns          376 ns          100
BM_BuyPartialFill/repeats:100_mean                203 ns          203 ns          100
BM_BuyPartialFill/repeats:100_median              202 ns          202 ns          100
BM_BuyPartialFill/repeats:100_stddev             1.35 ns         1.33 ns          100
BM_BuyPartialFill/repeats:100_cv                 0.67 %          0.66 %           100
BM_BuyPartialFill/repeats:100_p50                 202 ns          202 ns          100
BM_BuyPartialFill/repeats:100_p99                 207 ns          207 ns          100
BM_BuyPartialFill/repeats:100_p99.9               207 ns          207 ns          100
BM_SellPartialFill/repeats:100_mean               210 ns          211 ns          100
BM_SellPartialFill/repeats:100_median             210 ns          210 ns          100
BM_SellPartialFill/repeats:100_stddev            2.15 ns         2.11 ns          100
BM_SellPartialFill/repeats:100_cv                1.02 %          1.00 %           100
BM_SellPartialFill/repeats:100_p50                210 ns          210 ns          100
BM_SellPartialFill/repeats:100_p99                218 ns          217 ns          100
BM_SellPartialFill/repeats:100_p99.9              218 ns          217 ns          100
BM_CancelBuyKeepLevel/repeats:100_mean            125 ns          125 ns          100
BM_CancelBuyKeepLevel/repeats:100_median          124 ns          125 ns          100
BM_CancelBuyKeepLevel/repeats:100_stddev        0.242 ns        0.233 ns          100
BM_CancelBuyKeepLevel/repeats:100_cv             0.19 %          0.19 %           100
BM_CancelBuyKeepLevel/repeats:100_p50             124 ns          125 ns          100
BM_CancelBuyKeepLevel/repeats:100_p99             125 ns          126 ns          100
BM_CancelBuyKeepLevel/repeats:100_p99.9           125 ns          126 ns          100
BM_CancelSellKeepLevel/repeats:100_mean           128 ns          129 ns          100
BM_CancelSellKeepLevel/repeats:100_median         128 ns          129 ns          100
BM_CancelSellKeepLevel/repeats:100_stddev       0.477 ns        0.484 ns          100
BM_CancelSellKeepLevel/repeats:100_cv            0.37 %          0.38 %           100
BM_CancelSellKeepLevel/repeats:100_p50            128 ns          129 ns          100
BM_CancelSellKeepLevel/repeats:100_p99            131 ns          131 ns          100
BM_CancelSellKeepLevel/repeats:100_p99.9          131 ns          131 ns          100
BM_CancelBuyEmptyLevel/repeats:100_mean           125 ns          126 ns          100
BM_CancelBuyEmptyLevel/repeats:100_median         125 ns          126 ns          100
BM_CancelBuyEmptyLevel/repeats:100_stddev       0.563 ns        0.581 ns          100
BM_CancelBuyEmptyLevel/repeats:100_cv            0.45 %          0.46 %           100
BM_CancelBuyEmptyLevel/repeats:100_p50            125 ns          126 ns          100
BM_CancelBuyEmptyLevel/repeats:100_p99            127 ns          128 ns          100
BM_CancelBuyEmptyLevel/repeats:100_p99.9          127 ns          128 ns          100
BM_CancelSellEmptyLevel/repeats:100_mean          133 ns          133 ns          100
BM_CancelSellEmptyLevel/repeats:100_median        133 ns          133 ns          100
BM_CancelSellEmptyLevel/repeats:100_stddev      0.429 ns        0.494 ns          100
BM_CancelSellEmptyLevel/repeats:100_cv           0.32 %          0.37 %           100
BM_CancelSellEmptyLevel/repeats:100_p50           133 ns          133 ns          100
BM_CancelSellEmptyLevel/repeats:100_p99           134 ns          134 ns          100
BM_CancelSellEmptyLevel/repeats:100_p99.9         134 ns          134 ns          100

```
