#include "StdAfx.h"

#ifdef OPERA_CUSTOM_CODE

#include "Files.h"

#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

namespace opera::files {
std::vector<uint8_t> Read(const std::filesystem::path& file) {
  std::ifstream stream(file, std::ios::binary);
  if (!stream) {
    return {};
  }

  return std::vector<uint8_t>{ std::istreambuf_iterator<char>(stream), {} };
}

void Write(const std::filesystem::path& file, std::string_view data) {
  std::ofstream stream(file, std::ios::binary);
  if (stream) {
    stream.write(data.data(), data.size());
  }
}

}  // namespace opera::files

#endif OPERA_CUSTOM_CODE
