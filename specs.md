Source: https://www.youtube.com/watch?v=b1e4t2k2KJY&list=PL91pKiZDEdqqg0a5zJ5RgZg2EwRptJTM_&index=47

# Limit Order book

Buys <name, buy price, quantity>
- sorted in decreasing order

Sales <name, sell price, quantity>
- sorted in increasing order

# Features
- able to add buy/sell orders
- able to cancel buy/sell orders
- multiple instacnes of Limit Order Book for different stocks
- allow simulatnous processing of adding buy/sell orders


# Requirements
Scale
- 3 million messages/second at peak rates
- thousands of investors/connections, trying to coordinate the activity
- several million live orders

Fairness
- A lot of people competing against each other
- trying to have all of the information arriving to those participants at the same time as much as possible

Reliability

Durability
- If tell two partipants that they have traded, set off a chain of events that causes them to commit capital to other places
- if the trade didn't go through, created alot of problems

Robustines to bad clients
- Can't have someone who is badly behaved affecting other participants
- Badly behaved, could just be they are optimizing for something other than what you are (due to an incentive you put in place)


# Order fill logic

## problem 1
Buy
- 30, 500
- 29, 200

Sell
- 31, 100
- 32, 50

if Sell 29, 500 comes in
- give order fill to aggressive order
- encouraging people to send their most aggressive price, incase book is changing at that time

## problem 2
Buy
- 25, 500

person A orders first at 25, 250
person B orders after at 25, 250
- given the exact same price, order that arrived first is first in line
