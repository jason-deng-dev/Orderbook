#pragma once

#include <cstdint>
#include <string>
#include <vector>
class Feed_Handler {
public:
  bool Load(const std::string &filePath);

private:
  std::vector<uint8_t> buffer{};
  const uint8_t* cursor {nullptr};
};
