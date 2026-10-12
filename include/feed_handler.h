#pragma once

#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>
#include "types.h"


class Feed_Handler {
public:
  bool Load(const std::string &filePath);
  void Read(int n);
  bool ReadMessage();

private:
  std::vector<uint8_t> buffer{};
  const uint8_t* cursor {nullptr};
  std::unordered_set<char> modifyStateMsg {'A', 'F', 'E', 'C', 'D', 'U'};
};

