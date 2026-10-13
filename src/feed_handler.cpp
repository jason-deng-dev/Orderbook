#include "feed_handler.h"
#include <cstdint>

#include <fcntl.h>
#include <iostream>
#include <print>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

bool Feed_Handler::Load(const std::string &filepath) {
  int fd = open(filepath.c_str(), O_RDONLY);
  if (fd == -1) {
    std::cerr << "Failed to open file: " << filepath << "\n";
    return false;
  }
  // 2. Get file size
  struct stat sb;
  if (fstat(fd, &sb) == -1) { // retrieves metadata about open file
    close(fd);
    return false;
  }
  file_size_ = static_cast<size_t>(sb.st_size);
  if (file_size_ == 0) {
    close(fd);
    return false;
  }

  void *map = mmap(nullptr, file_size_, PROT_READ, MAP_PRIVATE, fd, 0);
  close(fd);

  if (map == MAP_FAILED) {
    std::cerr << "mmap failed\n";
    return false;
  }

  madvise(map, file_size_, MADV_SEQUENTIAL); // tells OS will read sequentially (optimizes read-ahead)

  buffer_ = static_cast<const uint8_t *>(map);
  cursor_ = buffer_;
  return true;
};



void Feed_Handler::Read(int n) {
  for (int i = 0; i < n; ++i) {
    std::print("{:02x}", static_cast<unsigned>(*cursor_));
    ++cursor_;
  }
}

// push to Spsc queue, then skip to next state modifying message (no delay when sending next message)
bool Feed_Handler::PushNextToQueue(SpscQueue& queue) {
  uint16_t msgLen = (static_cast<uint16_t>(cursor_[0]) << 8) | static_cast<uint16_t>(cursor_[1]);
  cursor_ += 2;
  auto msgType = static_cast<char>(cursor_[0]);
  ++cursor_;
  std::cout << "msgLen:" << msgLen << " msgType" << msgType << '\n';

  return true;
}
