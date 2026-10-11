# Nasdaq Totalview ITCH 5.0 Notes 
Source: https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/Totalview-ITCH-cloud.pdf#6#1



# Messages that do not modify book state
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



# Messages that modify book state
|Message Type | msgType | Total Length (Bytes)|
|--|--|--|
Add Order (No MPID) | A | 36
Add Order with MPID | F | 40
Order Executed | E | 31
Order Executed with Price | C | 36
Order Cancel | X | 23
Order Delete | D | 19
Order Replace | U | 35

