#include "feed_handler.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <system_error>

bool Feed_Handler::Load(const std::string &filepath) {
  std::error_code ec;
  const auto size =
      std::filesystem::file_size(filepath, ec); // failure handling: file missing, no permission, path is a directory
  if (ec) return false;

  std::ifstream in(filepath, std::ios::binary);
  if (!in) return false;

  buffer.resize(static_cast<size_t>(size));

  // std::istream::read takes char*, but buffer holds uint8_t
  in.read(reinterpret_cast<char *>(buffer.data()), static_cast<std::streamsize>(size));
  cursor = buffer.data();

  // gcount returns number of char extracted by the last unformatted read: equal to size means full read
  return in.gcount() == static_cast<std::streamsize>(size);
};

void Feed_Handler::Read(int n) {
  for (int i = 0; i < n; ++i) {
    // C++23 formats an integer as a lowercase hexadecimal value padded with a leading zero to a minimum wdith of two
    // digits
    std::print("{:02x}", static_cast<unsigned>(*cursor));
    ++cursor;
  }
}

bool Feed_Handler::ReadMessage() {
  
  uint16_t msgLen = (static_cast<uint16_t>(cursor[0]) << 8) | static_cast<uint16_t>(cursor[1]);
  cursor += 2;
  auto msgType = static_cast<char>(cursor[0]);
  ++cursor;
  std::cout << "msgLen:" << msgLen << " msgType" << msgType << '\n';

  return true;
}
