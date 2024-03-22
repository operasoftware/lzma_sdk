#ifndef OPERA_PAYLOAD_MANAGER_H_
#define OPERA_PAYLOAD_MANAGER_H_

#include <string>
#include <string_view>
#include <filesystem>

namespace opera {
class PayloadManager {
 public:
   enum class PayloadStatus
   {
     NOT_FOUND,      // Attempts to find a payload failed
     FOUND,			 // Payload found at the end of file
     EXTERNAL,       // Payload was supplied externally
   };

  PayloadManager(const std::filesystem::path& file);

  /**
   * @return the status of payload found by ReadPayload.
   */
  PayloadStatus GetStatus() const;

  /**
   * @return the payload found by ReadPayload as a string.
   */
  const std::string& GetPayload() const;

  /**
   * Manually sets the payload to use, in case it was provided from an
   * external source.
   */
  void SetPayload(const std::string& data);
private:
  PayloadStatus ReadPayload(const std::filesystem::path& file);

  std::vector<uint8_t> ReadFile(const std::filesystem::path& file);

  bool CompareHeaderAtLocation(
    const std::vector<uint8_t>& file_content,
    size_t size_pos,
    std::string_view str);

  std::string ReadPayloadFromLocation(
    const std::vector<uint8_t>& file_content,
    size_t size_pos);

  std::string payload_;
  PayloadStatus payload_status_ = PayloadStatus::NOT_FOUND;
};

}  // namespace opera

#endif  // OPERA_PAYLOAD_MANAGER_H_
