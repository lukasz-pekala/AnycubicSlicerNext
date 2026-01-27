#pragma once
#include <stddef.h>
#include <stdint.h>

uint64_t getKernelVersionAsU64(const char *version);
bool isArmArch(void);

struct operating_system_info {
  const char *arch;           ///< 架构
  const char *name;           ///< 名称
  const char *kernel_version; ///< 内核版本
};

bool getOpratingSystemInfo(operating_system_info **info);

struct cpu_info {
  const char *name; ///< 名称
  int32_t cores;    ///< 物理核数
  int32_t threads;  ///< 逻辑核数
};

bool getCPUInfo(cpu_info **info);

struct gpu_info {
  const char *name; ///< gpu名称
  uint64_t memory;  ///< gpu内存大小
};

bool getGPUInfo(gpu_info **info, size_t *count);

bool getRAMInfo(uint64_t **info, size_t *count);