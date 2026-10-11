#include "feed_handler.h"
#include <fstream>
#include <filesystem>
#include <system_error>

bool Feed_Handler::Load(const std::string &filepath) {
  std::error_code ec;
  const auto size = std::filesystem::file_size(filepath, ec); // failure handling: file missing, no permission, path is a directory
  if (ec) return false;

  std::ifstream in(filepath, std::ios::binary);
  if (!in) return false;

  buffer.resize(static_cast<size_t>(size));
  
  // std::istream::read takes char*, but buffer holds uint8_t
  in.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(size));
  cursor = buffer.data();

  // gcount returns number of cahracters extracted by the last unformatted read: equal to size means full read
  return in.gcount() == static_cast<std::streamsize>(size);
};


