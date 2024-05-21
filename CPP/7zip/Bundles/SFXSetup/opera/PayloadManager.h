#ifdef OPERA_CUSTOM_CODE

#ifndef OPERA_PAYLOAD_MANAGER_H_
#define OPERA_PAYLOAD_MANAGER_H_

#include <string>
#include <filesystem>
#include <vector>

namespace opera {
// Stores the information found in payload.
class Data {
 public:
  Data() = default;
  Data(std::string data);

  // @return true if data have been found.
  bool GetStatus() const;


  // @return the data as a string.
  const std::string& GetData() const;

 private:
  std::string data_;
};

// Searches for payload that can be added by server without repacking the whole
// file. It does not perform checks on the data. It is left to be done by
// archived installer.
class PayloadManager {
 public:
  // Constructor reads the file starting at the end in search of payload
  // headers.
  PayloadManager(const std::filesystem::path& file);

  // Getters:
  const Data& GetTrackingData() const;
  const Data& GetCustomizationPackage() const;

 private:
  // Read and store payload in 'tracking_data_' or 'customization_package_'
  // depending on payload header.
  // @return number of bytes stored.
  uint32_t ReadPayload(std::vector<uint8_t>& data);

  Data tracking_data_;
  Data customization_package_;
};
}  // namespace opera

#endif  // OPERA_PAYLOAD_MANAGER_H_
#endif  // OPERA_CUSTOM_CODE
