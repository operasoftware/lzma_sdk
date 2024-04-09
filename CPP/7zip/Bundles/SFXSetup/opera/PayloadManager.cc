#include "StdAfx.h"

#ifdef OPERA_CUSTOM_CODE

#include "PayloadManager.h"

#include <string>
#include <filesystem>
#include <fstream>


namespace opera {
  PayloadManager::PayloadManager(const std::filesystem::path& file)
  {
    payload_status_ = ReadPayload(file);
  }

  bool PayloadManager::CompareHeaderAtLocation(
    const std::vector<uint8_t>& file_content,
    size_t size_pos,
    std::string_view str)
  {
    return !strncmp(
      reinterpret_cast<const char*>(file_content.data() + size_pos),
      str.data(), str.size());
  }

  std::string PayloadManager::ReadPayloadFromLocation(
    const std::vector<uint8_t>& file_content,
    size_t size_pos)
  {
    const uint8_t* d = file_content.data() + size_pos;
    size_t data_size = (d[0] << 0) | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
    if (data_size > size_pos - 3) {
      return {};
    }

    size_t data_pos = size_pos - data_size;
    size_t magic_pos = data_pos - 3;
    if (!CompareHeaderAtLocation(file_content, magic_pos, "OPR")) {
      return {};
    }

    return std::string(
      reinterpret_cast<const char*>(file_content.data() + data_pos),
      data_size);
  }

  PayloadManager::PayloadStatus PayloadManager::ReadPayload(
    const std::filesystem::path& file)
  {
    std::vector<uint8_t> file_content = ReadFile(file);
    if (file_content.empty()) {
      return PayloadStatus::NOT_FOUND;
    }

    payload_ = ReadPayloadFromLocation(
      file_content,
      file_content.size() - sizeof(uint32_t));

    return !payload_.empty() ? PayloadStatus::FOUND : PayloadStatus::NOT_FOUND;
  }


  std::vector<uint8_t> PayloadManager::ReadFile(const std::filesystem::path& file)
  {
    std::ifstream stream(file, std::ios::binary);
    if (!stream)
      return {};

    return std::vector<uint8_t>{std::istreambuf_iterator<char>(stream), {}};
  }

  void PayloadManager::SetPayload(const std::string& data)
  {
    payload_ = data;
    payload_status_ = PayloadStatus::EXTERNAL;
  }

  PayloadManager::PayloadStatus PayloadManager::GetStatus() const
  {
    return payload_status_;
  }

  const std::string& PayloadManager::GetPayload() const
  {
    return payload_;
  }
}  // namespace opera

#endif // OPERA_CUSTOM_CODE
