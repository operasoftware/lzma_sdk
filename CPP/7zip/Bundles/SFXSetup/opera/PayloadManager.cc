#include "StdAfx.h"

#ifdef OPERA_CUSTOM_CODE

#include "PayloadManager.h"

#include "Files.h"

#include <filesystem>
#include <string_view>
#include <string>
#include <utility>

namespace opera {
namespace {
enum class DataType {
  UNKNOWN,
  TRACKING_DATA,
  CUSTOMIZATION_PACKAGE
};

constexpr size_t kHeaderLength = 3;
constexpr size_t kSizeMarkerLength = 4;

// String comparison.
bool CompareHeaderAtLocation(const std::vector<uint8_t>& file_content,
                             size_t size_pos,
                             std::string_view str) {
  return !strncmp(reinterpret_cast<const char*>(file_content.data() + size_pos),
                  str.data(), str.size());
}

// String comparison to known constant.
// Change to map in case of additional types.
DataType CheckDataType(const std::vector<uint8_t>& file_content,
                       size_t size_pos) {
  const auto compare = [&](std::string_view str) {
    return CompareHeaderAtLocation(file_content, size_pos, str);
  };

  return compare("OPR")   ? DataType::TRACKING_DATA
         : compare("RES") ? DataType::CUSTOMIZATION_PACKAGE
                          : DataType::UNKNOWN;
}

// Reads payload from 'file_content'.
// At the end of payload, at 'size_pos" there must be 4 byte long size of
// payload. Header with file type must be placed before payload. 3 bytes of
// header type are not included in size marker.
std::pair<DataType, std::string> ReadPayloadFromLocation(
    const std::vector<uint8_t>& file_content,
    size_t size_pos) {
  auto data_size =
      *reinterpret_cast<const uint32_t*>(file_content.data() + size_pos);
  if (data_size > size_pos - kHeaderLength) {
    return {};
  }

  auto data_pos = size_pos - data_size;
  auto magic_pos = data_pos - kHeaderLength;
  const auto data_type = CheckDataType(file_content, magic_pos);
  return { data_type, data_type == DataType::UNKNOWN
                          ? ""
                          : std::string(reinterpret_cast<const char*>(
                                            file_content.data() + data_pos),
                                        data_size) };
}

}  // namespace

PayloadManager::PayloadManager(const std::filesystem::path& file) {
  auto file_content = files::Read(file);
  auto payload_size = ReadPayload(file_content);
  if (!payload_size) {
    return;
  }

  payload_size += kHeaderLength + kSizeMarkerLength;
  if (payload_size < file_content.size()) {
    // There could be a second payload. Remove first from the buffer before
    // reading again.
    file_content.resize(file_content.size() - payload_size);
    ReadPayload(file_content);
  }
}

const opera::Data& PayloadManager::GetTrackingData() const {
  return tracking_data_;
}

const opera::Data& PayloadManager::GetCustomizationPackage() const {
  return customization_package_;
}

uint32_t PayloadManager::ReadPayload(std::vector<uint8_t>& data) {
  if (data.size() < kSizeMarkerLength) {
    return 0;
  }

  auto [payload_type, payload] =
      ReadPayloadFromLocation(data, data.size() - kSizeMarkerLength);
  switch (payload_type) {
    case DataType::TRACKING_DATA:
      tracking_data_ = std::move(payload);
      return tracking_data_.GetData().size();
    case DataType::CUSTOMIZATION_PACKAGE:
      customization_package_ = std::move(payload);
      return customization_package_.GetData().size();
    default:
      return 0;
  }
}

Data::Data(std::string data) : data_(std::move(data)) {}

bool Data::GetStatus() const {
  return !data_.empty();
}

const std::string& Data::GetData() const {
  return data_;
}
}  // namespace opera

#endif  // OPERA_CUSTOM_CODE
