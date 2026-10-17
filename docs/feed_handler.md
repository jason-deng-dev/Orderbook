# NASDAQ ITCH 5.0 feed handler
NASDAQ ITCH 5.0 feed handler: parser → SPSC ring → book builder; measured handoff cost at [X]ns and pipelined throughput at [N]M msgs/sec vs. [M]M single-threaded; book-state hashes identical across modes

Source: https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/ (07302019.NASDAQ_ITCH50.gz)
- this is the file used for benchmarking/correctness testing

# API
Load()
- loads the contents of the NASDAQ_ITCH50 file into buffer

GetMessage()
- gets next message

# Buffer data structure
Why not `std::unique_ptr<uint8_t[]>` or `std::vector`
- because it forces you to copy the entire NASDAQ_ITCH50 file from disk into RAM

`mmap`
Zero-Copy from disk
- allows the OS to simply map the file's existing pages into process's virtual address space. Zero bytes copied (just get a pointer).

Lazy Loading
- Since ITCH files can be massive, by using mmap, memory footprint stays tiny and startup time is effecively instant


# Workflow
- loads NASDAQ_ITCH50 file into buffer
- then pushes order messages into SPSC queue
- Orderbook thread pops the parsed QueueItem and executes based on order type

# header data structure to SPSC queue
```c++
enum class OrderType : uint8_t { Add, Cancel, Delete, Replace };

struct alignas(32) QueueItem {
  // 8-byte types (24 total)
  OrderId orderId;
  OrderId removeId; // used for ReplaceOrder
  Price price;

  // 4-byte types (4 total)
  Volume volume;

  // 1-byte types (2 total)
  OrderType orderType;
  Side side;
};
```
AddOrder
- Buy/Sell Indicator
- Shares
- Stock
- Price
- Order Reference Number

CancelOrder (partial cancelation)
- Order Reference Number
- Cancelled Shares

DeleteOrder (full cancelation)
- Order Reference Number

ReplaceOrder
- Original ORN
- New ORN
- new Shares
- new Price

Note that we do not pass over Order Executed order messages
- since Order Executed are outputs of my Orderbook's matching logic, not inputs
- thus our FeedHandler only passes through Add/Cancel/Delete/Replace

# Tracking Stock Locate
If we want to filter stock data to specify a specific stock that we want to process we can track Stock locate which is always at offset 1, and stable per session.

Wheen we first parse an 'A' or 'F' message type, we set private variable stock locate.

going forward we only forward messages with that stock locate value




# Nasdaq Totalview ITCH 5.0 Notes 
Source: https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHspecification.pdf
## Messages that do not modify book state
|Message Type | msgType | total length|
|--|--|--|
System Event Messages | S | 12
Stock Related Messages | R | 39
Stock Trading Action | H | 25
Reg SHO Short Sale Price Test | Y | 20
Market Participant Position | L |  26
MWCB Decline Level | V |  35
MWCB Status | W | 12
IPO Quoting Period Update | K | 28
LULD Auction Collar | J | 35
Operational Halt | h | 21
NOII | I | 50
RPII | N | 20
Direct Listing with Capital Raise | O | 48
Trade (Non-Cross) | P | 44
Cross Trade | Q | 40
Broken Trade | B | 19

## Messages that modify book state
|Message Type | msgType | Total Length (Bytes)|
|--|--|--|
Add Order (No MPID) | A | 36
Add Order with MPID | F | 40
Order Executed | E | 31
Order Executed with Price | C | 36
Order Cancel | X | 23
Order Delete | D | 19
Order Replace | U | 35


**Add Order : No MPID Attribution**
![alt text](images/nasdaq/image.png)

**Add Order with MIPD**
![alt text](images/nasdaq/image-1.png)

**Order Executed**
- sent whenever an order on book is excuted in whole or in part
- possible to recieve several Order Exectued Message for the same order reference number if that order is excuted in several parts
- by combining executions from Order Executed Messages and Trade Message, possible to build a complete view of all non-cross executions that happen on Nasdaq 

![alt text](images/nasdaq/image-2.png)

**Order Executed with Price**
![alt text](images/nasdaq/image-3.png)
![alt text](images/nasdaq/image-4.png)

**Order Cancel**
- sent whenever order on the book is modified as a result of partial cancellation

![alt text](images/nasdaq/image-5.png)

**Order Delete Message**
- sent whenever order on book is being cancelled.
- All remaining shares are no longer accessible so the order must be removed from the book
![alt text](images/nasdaq/image-6.png)

**Order Replace Message**
- sent whenever order on book has been cancel-replaced
- all remaining shares from the original  are no longer accesible, and must be removed
- new order details are provided for the replacement, along wit hteh new order reference number which will be used henceforth
- Since the side/stock symbol/attribution (if any) cannot be changed by an Order Replace event, these fields are not included in the message
  - should retain the side/stock symbol and MPID from original Add Order message

![alt text](images/nasdaq/image-7.png)

## Output shape
viewing 256 bytes via  `xxd -l 256 benchmarks/fixtures/07302019.NASDAQ_ITCH50`
```
00000000: 000c 5300 0000 0009 fa4d 3ad2 6b4f 0027  ..S......M:.kO.'
00000010: 5200 0100 000a 392d 5f03 8c41 2020 2020  R.....9-_..A    
00000020: 2020 204e 2000 0000 644e 435a 2050 4e20     N ...dNCZ PN 
00000030: 314e 0000 0000 4e00 2752 0002 0000 0a39  1N....N.'R.....9
00000040: 2d60 ee35 4141 2020 2020 2020 4e20 0000  -`.5AA      N ..
00000050: 0064 4e43 5a20 504e 2031 4e00 0000 014e  .dNCZ PN 1N....N
00000060: 0027 5200 0300 000a 392d 61a2 0541 4141  .'R.....9-a..AAA
00000070: 5520 2020 2050 2000 0000 644e 5149 2050  U    P ...dNQI P
00000080: 4e20 3259 0000 0001 4e00 2752 0004 0000  N 2Y....N.'R....
00000090: 0a39 2d62 74af 4141 4241 2020 2020 514e  .9-bt.AABA    QN
000000a0: 0000 0064 4e43 5120 504e 4e32 4e00 0000  ...dNCQ PNN2N...
000000b0: 014e 0027 5200 0500 000a 392d 6320 0a41  .N.'R.....9-c .A
000000c0: 4143 2020 2020 204e 2000 0000 644e 435a  AC     N ...dNCZ
000000d0: 2050 4e20 324e 0000 0000 4e00 2752 0006   PN 2N....N.'R..
000000e0: 0000 0a39 2d63 c35f 4141 4452 2020 2020  ...9-c._AADR    
000000f0: 5020 0000 0064 4e51 4920 504e 2032 5900  P ...dNQI PN 2Y.
```

`000c 5300 0000 0009 fa4d 3ad2 6b4f 0027'`
|byte | info |
|--|--|
000c | length 12
53 | 'S' System Event
00 00 |stock locate 0
00 00 | tracking 0
09 fa 4d 3a d2 6b | timestamp, nanoseconds since midnight 
4f | event code 'O' = Start of messages

[00 0c] [53 00 00 00 00 09 fa 4d 3a d2 6b 4f]
- [00 0c] is the length prefix, takes up 2 bytes
- thus given our Message length 12 bytes, the next record starts 2+12=14 bytes later

