https://www.youtube.com/watch?v=sX2nF1fW7kI&t=4499s

# Current implementation
Orderbook:
map<int price, OrdersAtPrice> buy/sellOrderMap
unordered_map<int, Trade*> traderRegistery
vector<Trade> tradeHistory

Trader:
unordered_map<Orderbook*, map<int price, vector<int tradeId>>> buy/sellOrders
unordered_map<Orderbook*, int amount> inventory

# Takeaways from "David Gross - CppCon 2024"

