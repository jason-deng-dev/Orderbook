#pragma once

#include "types.h"
#include <cstdint>
#include <string>
#include <sys/mman.h>
#include <unordered_set>
#include "spsc_fifo.h"
#include <vector>

class Feed_Handler {
public:
  bool Load(const std::string &filePath);
  void Read(int n);
  // bool PushNextToQueue(SpscFifo<QueueItem,5>& queue);

  ~Feed_Handler() {
    if (buffer_) {
      munmap(const_cast<uint8_t *>(buffer_), file_size_);
    }
  }

  // Accessors
  const uint8_t *cursor() const noexcept { return cursor_; }
  const uint8_t *buffer_end() const noexcept { return buffer_ + file_size_; }
  void advance_cursor(size_t bytes) noexcept { cursor_ += bytes; }

private:
  const uint8_t *buffer_ = nullptr; // start of file
  const uint8_t *cursor_ = nullptr;
  size_t file_size_ = 0;
  std::unordered_set<char> modifyStateMsg{'A', 'F', 'E', 'C', 'D', 'U'};
};
