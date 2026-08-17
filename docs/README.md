traders
- hold buy and sell orders
- able to see total outcoming/incoming order costs etc
- has unique ID's
- should be able to see all orders for different stocks

Orderbook
- for a particular stock
- Who are the market participants, what are their prices/order amount
- needs to track order history
  - track time of orders etc
- need to maintain for a given price who put order down first

# Classes

## Trader
- static next_id (to help set incrementing id on each new Trader)
- id
- name
- balance
- buyOrders<Orderbook* , <price, <trade_ids>>>
- sellOrders<Orderbook* , <price, <trade_ids>>>

Trader(name, balance)

## BuyOrders
BuyOrders.add(trader_id, quantity, price)
BuyOrders.cancel(trader_id, quantity, price)

## SellOrders
SellOrders.add(trader_id, quantity, price)
SellOrders.cancel(trader_id, quantity, price)

## Orderbook
Orderbook.buy(Trader*, quanity, price)
Orderbook.sell(Trader*, quantity, price)
Orderbook.cancelBuy(Trader*, quanity, price)
Orderbook.cancelSell(Trader*, quantity, price)

# Design decisions

## data structure of BuyOrders/Sellorders
Issue:
currently BuyOrders/Sellorders implemented data structure is minHeap/maxHeap, to facilate finding best offers

- need to be able to iterate through orders to display all stock
- also to allow changing values when users add/cancel buyOrders/sellOrders

Idea:
map<price, OrdersAtPrice>
- begin() = minOrder O(1)
- rbegin() = maxOrder O(1)
- lookup O(log n)
- insert O(log n)
- erase(key) O(log n)
- allows iteration

so that we get O(1) to access best offers, which is the most critical operation

## easy manipulation of BuyOrders/SellOrders
Issue:
- how to allow efficient changing of BuyOrders/Sellorders, as traders buy/sell stocks
- while also retaining FIFO
- current implementation through queue, allows FIFO, but does not support easy removing orders

Idea:
- assign each order an unique id, inside OrdersAtPrice
- store orders as map
- unique id globally monotonic (based on arrival order, across all price level)
map<order_id, Order*>

so that can access oldest order with begin() O(1)
and then alter orders with look up O(log n)
to erase orders, if filled or canceled O(log n)




