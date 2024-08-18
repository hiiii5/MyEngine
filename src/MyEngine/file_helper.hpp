#pragma once

#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <iterator>

namespace MyEngine {
namespace fs {
enum FsReadStatus { SUCCESS = 1, ERR_NOT_REGULAR_FILE = 0, ERR_NOT_OPEN = -1 };

static std::string FsReadStatusToString(FsReadStatus status) {
  switch (status) {
  case SUCCESS:
    return "SUCCESS";
    break;
  case ERR_NOT_REGULAR_FILE:
    return "NOT A REGULAR FILE";
    break;
  case ERR_NOT_OPEN:
    return "UNABLE TO OPEN FILE";
    break;
  default:
    return "";
  }
}

static std::filesystem::path MakePath(const std::string &path,
                                      bool allowNonRegularFiles = false) {
  std::filesystem::path filePath{path};
  if (!std::filesystem::is_regular_file(filePath) && !allowNonRegularFiles) {
    return std::filesystem::path();
  }

  return filePath;
}

static std::filesystem::path
MakeAbsolutePath(const std::filesystem::path &path) {
  return std::filesystem::absolute(path);
}

static FsReadStatus ReadFile(const std::string &path, std::string *pContents) {
  std::filesystem::path filePath{path};
  filePath = MakeAbsolutePath(filePath);
  if (!std::filesystem::is_regular_file(filePath)) {
    fmt::print("not a regular file: {}\n", filePath.c_str());
    *pContents = "";
    return ERR_NOT_REGULAR_FILE;
  }

  // Binary mode to get bytes of the file on the filesystem
  std::ifstream file(filePath, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    fmt::print("unable to open file: {}\n", filePath.c_str());
    *pContents = "";
    return ERR_NOT_OPEN;
  }

  std::string contents{std::istreambuf_iterator<char>(file),
                       std::istreambuf_iterator<char>()};
  file.close();

  *pContents = contents;
  return SUCCESS;
}
} // namespace fs
} // namespace MyEngine
