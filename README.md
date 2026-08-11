Design



traders
- hold buy and sell orders
- able to see total outcoming/incoming order costs etc
- has unique ID's

Orderbook
- for a particular stock
- Who are the market participants, what are their prices/order amount
- needs to track order history
  - track time of orders etc
- need to maintain for a given price who put order down first

# Interface

## Trader
Trader.buy(Orderbook*, quantity, price)
Trader.sell(Orderbook*, quantity, price)
Trader.cancelBuy(Orderbook*, quantity, price)
Trader.cancelSell(Orderbook*, quantity, price)

## BuyOrders
BuyOrders.add(trader_id, quantity, price)
BuyOrders.cancel(trader_id, quantity, price)

## SellOrders
SellOrders.add(trader_id, quantity, price)
SellOrders.cancel(trader_id, quantity, price)


