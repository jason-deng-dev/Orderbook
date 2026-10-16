#include "feed_handler.h"
#include "orderbook_map.h"
#include "spsc_fifo.h"
#include "types.h"
#include <iostream>
int main() {
  Feed_Handler fh ;
  fh.Load(FIXTURES_DIR "/07302019.NASDAQ_ITCH50");
  
  SpscFifo<QueueItem> fifo;

  Orderbook_Map ob;
  

}
