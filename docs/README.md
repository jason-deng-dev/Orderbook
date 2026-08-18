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
addBuyOrder(Orderbook*, price, trade_id)
cancelBuyOrder(Orderbook*, price, trade_id)
addSellOrder(Orderbook*, price, trade_id)
cancelSellOrder(Orderbook*, price, trade_id)


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

## trader keeping track of trades, and Orderbook keeping track of trades, two sources of truth (sync bugs)
Currently in Trader
<Orderbook*, <price, <trade_ids>>>
std::unordered_map<Orderbook*, std::map<int, std::vector<int>>> buyOrders;
std::unordered_map<Orderbook*, std::map<int, std::vector<int>>> sellOrders;

in BuyOrders/SellOrders
// <price, orders>
std::map<int, OrdersAtPrice *> buyOrderMap

in OrdersAtPrice
// <trade_id_, Order*>
std::map<int, Order *> order_queue;

need to allow trader access to all their buy orders/sell orders
- so that can display and cancel orders

Idea 1:

Trader
<Orderbook*, vector<Order*>>
add price value to Order

- so that have access to all Orders in a Orderbook as objects and can display data as needed
- and can change the order amount easily
- if want to cancel order completely
  - Orderbook->BuyOrders->buyOrderMap[price]
  - OrdersAtPrice->order_queue.erase(trade_id)


## Simplify class structure
Current:

Order 

OrdersAtPrice{
  map<trade_id, Order*>
}

Buy/SellOrders {
  
}

Orderbook{
  BuyOrders
  SellOrders
}

Idea:
- remove BuyOrders/SellOrders since they contain only a map
- just keep map<price, OrdersAtPrice*> in Orderbook
- have functions manipulate them as needed

Order 

OrdersAtPrice{
  map<trade_id, Order*>
}

Orderbook{
  map<price, OrdersAtPrice*> buyOrders
  map<price, OrdersAtPrice*> sellOrders
}

## Id generation tied to creation of Order
Current:
struct Order {
  static int currId;
  int price_;
  int trade_id_;
  int quantity_;
  int trader_id_;
  Order(int quantity, int trader_id, int price)
      : price_(price), quantity_(quantity), trader_id_(trader_id) {
    trade_id_ = currId++;
  }
};

currently can only know current id when Order object is created, which is not convienent for storing in map as we are creating the object

Idea:
refactor to have the incrementing currId be stored in Orderbook, and use that when creating Orders to assign id.
