#include "feed_handler.h"
#include "types.h"
#include <cstdint>

#include <cstring>
#include <expected>
#include <fcntl.h>
#include <iostream>
#include <optional>
#include <print>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>
#include <type_traits>
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

template <typename T>
T read_value(const uint8_t *ptr) {
  static_assert(std::is_integral_v<T>, "wire types are integral");
  T value;
  std::memcpy(&value, ptr, sizeof(T));
  if constexpr (std::endian::native == std::endian::little) {
    value = std::byteswap(value); // reverse big-endian wire order to host
  }
  return value;
}

std::expected<QueueItem, MessageStatus> Feed_Handler::GetNextMessage() {
  const size_t remaining = static_cast<size_t>(buffer_end() - cursor_);
  if (remaining < 2) {
    return std::unexpected(MessageStatus::End);
  }

  uint16_t msgLen = read_value<uint16_t>(cursor_);
  if (msgLen < 1 || remaining < 2 + msgLen) {
    return std::unexpected(MessageStatus::End);
  }

  auto startMsgInfo = cursor_ + 2;
  char msgType = read_value<char>(startMsgInfo);
  uint16_t stockLocate = read_value<uint16_t>(startMsgInfo + 1);

  advance_cursor(msgLen + 2);

  if (filter_by_stock_ && !found_stock_locate_ && (msgType == 'A' || msgType == 'F')) {
    stock_locate_ = stockLocate;
    found_stock_locate_ = true;
  }

  if (filter_by_stock_ && found_stock_locate_ && stockLocate != stock_locate_) {
    return std::unexpected(MessageStatus::Skipped);
  }

  switch (msgType) {
  case 'A':
  case 'F': {
    OrderId ORN = read_value<OrderId>(startMsgInfo + 11);
    Side side = read_value<char>(startMsgInfo + 19) == 'B' ? Side::Bid : Side::Ask;
    Volume volume = read_value<Volume>(startMsgInfo + 20);
    Price price = read_value<uint32_t>(startMsgInfo + 32);

    return {{
        .orderId = ORN,
        .price = price,
        .volume = volume,
        .orderType = OrderType::Add,
        .side = side,
    }};
  }

  case 'X': {
    OrderId ORN = read_value<OrderId>(startMsgInfo + 11);
    Volume volume = read_value<Volume>(startMsgInfo + 19);
    return {{.orderId = ORN, .volume = volume, .orderType = OrderType::Cancel}};
  }

  case 'D': {
    OrderId ORN = read_value<OrderId>(startMsgInfo + 11);
    return {{
        .orderId = ORN,
        .orderType = OrderType::Delete,
    }};
  }

  case 'U': {
    OrderId newORN = read_value<OrderId>(startMsgInfo + 19);
    OrderId removeORN = read_value<OrderId>(startMsgInfo + 11);
    Volume volume = read_value<Volume>(startMsgInfo + 27);
    Price price = read_value<uint32_t>(startMsgInfo + 31);

    return {
        {.orderId = newORN, .removeId = removeORN, .price = price, .volume = volume, .orderType = OrderType::Replace}};
  }
  default: {
    return std::unexpected(MessageStatus::Skipped);
  }
  }
}

// push to Spsc queue, then skip to next state modifying message (no delay when sending next message)
template <typename T>
bool Feed_Handler::PushToQueue(SpscFifo<T> &fifo) {
  return true;
}
