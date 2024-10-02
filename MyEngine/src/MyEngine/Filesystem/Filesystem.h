#pragma once

#include <filesystem>
#include <fstream>
#include <iterator>
#include <unistd.h>

namespace MyEngine {
namespace Filesystem {
enum FsReadStatus {
  READ_SUCCESS = 1,
  ERR_NOT_REGULAR_FILE = 0,
  ERR_NOT_OPEN = -1
};
enum FsWriteStatus {
  WRITE_SUCCESS = 1,
  ERR_WRITE_STATUS_INVALID_DIRECTORIES = 0,
  ERR_WRITE_STATUS_UNREACHABLE_FILE = -1
};

static std::string FsReadStatusToString(FsReadStatus status) {
  switch (status) {
  case READ_SUCCESS:
    return "READ SUCCESS";
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

static std::string FsWriteStatusToString(FsWriteStatus status) {
  switch (status) {
  case WRITE_SUCCESS:
    return "WRITE SUCCESS";
    break;
  case ERR_WRITE_STATUS_INVALID_DIRECTORIES:
    return "UNABLE TO WRITE DIRECTORY";
  case ERR_WRITE_STATUS_UNREACHABLE_FILE:
    return "UNABLE TO WRITE TO FILE";
    break;
  default:
    return "";
  }
}

static std::filesystem::path GetWorkingDirectory() {
  return std::filesystem::current_path();
}

static std::string GetFilename(const std::string &filepath) {
  return std::filesystem::path(filepath).filename().string();
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
    *pContents = "";
    return ERR_NOT_REGULAR_FILE;
  }

  // Binary mode to get bytes of the file on the filesystem
  std::ifstream file(filePath, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    *pContents = "";
    return ERR_NOT_OPEN;
  }

  std::string contents{std::istreambuf_iterator<char>(file),
                       std::istreambuf_iterator<char>()};
  file.close();

  *pContents = contents;
  return READ_SUCCESS;
}

static FsReadStatus ReadSpvFile(const std::string &path,
                                std::vector<uint32_t> *pContents) {
  std::filesystem::path filePath{path};
  filePath = MakeAbsolutePath(filePath);
  if (!std::filesystem::is_regular_file(filePath)) {
    *pContents = std::vector<uint32_t>();
    return ERR_NOT_REGULAR_FILE;
  }

  std::ifstream in(path, std::ios::in | std::ios::binary);
  if (!in.is_open()) {
    *pContents = std::vector<uint32_t>();
    return ERR_NOT_OPEN;
  }

  in.seekg(0, std::ios::end);
  auto size = in.tellg();
  in.seekg(0, std::ios::beg);

  auto &data = pContents;
  data->resize(size / sizeof(uint32_t));
  in.read((char *)data->data(), size);
  in.close();

  return READ_SUCCESS;
}

static bool Exists(const std::string &path) {
  return std::filesystem::exists(path);
}

static FsWriteStatus WriteSpvFile(const std::string &path,
                                  const std::vector<uint32_t> &contents) {
  std::filesystem::path filepath{path};
  filepath = MakeAbsolutePath(filepath);

  if (!Filesystem::Exists(filepath.parent_path())) {
    std::filesystem::create_directories(filepath.parent_path());
  }

  std::ofstream out(filepath, std::ios::out | std::ios::binary);
  if (!out.is_open()) {
    return ERR_WRITE_STATUS_UNREACHABLE_FILE;
  }

  auto &data = contents;
  out.write((char *)data.data(), data.size() * sizeof(uint32_t));
  out.flush();
  out.close();

  return WRITE_SUCCESS;
}

static std::string GetCacheDirectory() {
  std::string path =
      MakeAbsolutePath(GetWorkingDirectory()).string() + "/assets/cache";
  if (!std::filesystem::exists(path)) {
    std::filesystem::create_directories(path);
  }

  return path;
}
} // namespace Filesystem
} // namespace MyEngine
