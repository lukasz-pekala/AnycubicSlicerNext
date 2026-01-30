#include "os.hpp"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

uint64_t getKernelVersionAsU64(const char *version) {
  if (version == nullptr) {
    return 0;
  }

  char *endptr;
  uint64_t major = strtol(version, &endptr, 10);

  if (*endptr != '.' || major > 0xFFFF)
    return 0; // 检查主版本号范围

  uint64_t minor = strtol(endptr + 1, &endptr, 10);
  if (*endptr != '.' || minor > 0xFFFF)
    return 0; // 检查次版本号范围

  uint64_t patch = strtol(endptr + 1, &endptr, 10);
  if (*endptr != '\0' && !isspace(*endptr))
    return 0; // 检查字符串结束

  return (major << 32) | (minor << 16) | patch;
}
