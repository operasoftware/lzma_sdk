#ifdef OPERA_CUSTOM_CODE

#ifndef OPERA_FILES_H_
#define OPERA_FILES_H_

#include <filesystem>
#include <string_view>
#include <vector>

// Simple access to file in binary form.
// There are equivalents in lzma sdk, that should be used instead.
// Removing this namespace and its dependencies will help shrink the sfx file.
namespace opera::files {
std::vector<uint8_t> Read(const std::filesystem::path& file);
void Write(const std::filesystem::path& file, std::string_view data);
}  // namespace opera::files

#endif // OPERA_FILES_H_

#endif // OPERA_CUSTOM_CODE

