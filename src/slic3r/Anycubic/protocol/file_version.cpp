#include "file_version.hpp"
#ifdef _WIN32
#include <string>
#include <tchar.h>
#include <vector>
#include <windows.h>

// 修改函数返回值为点分十进制格式的版本号字符串
std::string getPEFileVersion(const char *filename) {
  DWORD dummy = 0;
  DWORD size = GetFileVersionInfoSizeA(filename, &dummy);
  if (size == 0) {
    return ""; // 失败时返回空字符串
  }

  std::vector<BYTE> buffer(size);
  if (!GetFileVersionInfoA(filename, 0, size, buffer.data())) {
    return ""; // 失败时返回空字符串
  }

  VS_FIXEDFILEINFO *pFileInfo = nullptr;
  UINT len = 0;
  if (!VerQueryValueA(buffer.data(), "\\", (LPVOID *)&pFileInfo, &len)) {
    return ""; // 失败时返回空字符串
  }

  if (len < sizeof(VS_FIXEDFILEINFO)) {
    return ""; // 失败时返回空字符串
  }

  // 提取版本号各部分
  WORD major = HIWORD(pFileInfo->dwFileVersionMS);
  WORD minor = LOWORD(pFileInfo->dwFileVersionMS);
  WORD revision = HIWORD(pFileInfo->dwFileVersionLS);
  WORD build = LOWORD(pFileInfo->dwFileVersionLS);

  // 构建点分十进制格式的版本号字符串
  return std::to_string(major) + "." + std::to_string(minor) + "." +
         std::to_string(revision) + "." + std::to_string(build);
}

#endif