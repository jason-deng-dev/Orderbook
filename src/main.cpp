#include "feed_handler.h"
#include <iostream>
int main() {
  Feed_Handler fh ;
  fh.Load(FIXTURES_DIR "/07302019.NASDAQ_ITCH50");
  std::cout << "Start printing:";
  fh.GetMessage();
  std::cout << "\n END\n";
}
