#pragma once

#include <experimental/filesystem>
namespace filesystem = std::experimental::filesystem;

/*
* Project specific extensions
*/
namespace std {

template<>
struct hash<filesystem::path> {
  size_t operator()(const filesystem::path& p) const {
    std::hash<std::string> hasher;
    return hasher(p.string());
  }
};

}  // namespace std

filesystem::path GetBasePath();
