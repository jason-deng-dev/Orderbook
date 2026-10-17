#pragma once

#include "spsc_fifo.h"
#include "types.h"
#include <cstdint>
#include <optional>
#include <string>
#include <sys/mman.h>


class Feed_Handler {
public:
  ~Feed_Handler() {
    if (buffer_) {
      munmap(const_cast<uint8_t *>(buffer_), file_size_);
    }
  }

  bool Load(const std::string &filePath);
  void Read(int n);
  template <typename T>
  bool PushToQueue(SpscFifo<T> &fifo);
  std::optional<QueueItem> GetNextMessage();

  // Accessors
  const size_t size() const noexcept { return file_size_; }
  const uint8_t *cursor() const noexcept { return cursor_; }
  const uint8_t *buffer_end() const noexcept { return buffer_ + file_size_; }
  void advance_cursor(size_t bytes) noexcept { cursor_ += bytes; }

private:
  const uint8_t *buffer_ = nullptr; // start of file
  const uint8_t *cursor_ = nullptr;
  size_t file_size_ = 0;
};
