#pragma once

#include "types.h"
#include <cstdint>
#include <string>
#include <sys/mman.h>
#include <unordered_set>
#include <vector>

class Feed_Handler {
public:
  bool Load(const std::string &filePath);
  bool GetMessage();

  ~Feed_Handler() {
    if (buffer_) {
      munmap(const_cast<uint8_t *>(buffer_), file_size_);
    }
  }

  // Accessors
  const uint8_t *cursor() const { return cursor_; }
  const uint8_t *buffer_end() const { return buffer_ + file_size_; }
  void advance_cursor(size_t bytes) { cursor_ += bytes; }

private:
  const uint8_t *buffer_ = nullptr; // start of file
  const uint8_t *cursor_ = nullptr;
  size_t file_size_ = 0;
  std::unordered_set<char> modifyStateMsg{'A', 'F', 'E', 'C', 'D', 'U'};
};
