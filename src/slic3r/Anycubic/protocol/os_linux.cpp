#include "os.hpp"

#ifdef __linux__
#include "os_common.hpp"

#include <fstream>
#include <regex>
#include <string>
#include <vector>

#include <string.h>
#include <unistd.h>

#include <sys/sysinfo.h>
#include <sys/utsname.h>

bool isArmArch(void) { return false; }

bool getOpratingSystemInfo(operating_system_info **info) {
  EXPEND_INITIALIZER_BEGIN(operating_system_info, os_info, *info = &os_info) {
    // 获取系统架构
    os_info.arch = "x86_64";

    // 获取内核版本
    struct utsname buf;
    if (uname(&buf) == 0) {
      os_info.kernel_version = buf.release;
    } else {
      os_info.kernel_version = nullptr;
    }

    // 获取发行版名称
    std::ifstream osRelease("/etc/os-release");
    if (osRelease.is_open()) {
      std::string line;
      while (std::getline(osRelease, line)) {
        if (line.find("PRETTY_NAME=") == 0) {
          os_info.name = strdup(line.substr(13, line.length() - 14).c_str());
          break;
        }
      }
    } else {
      os_info.name = "Linux";
    }
  }
  EXPEND_INITIALIZER_END(*info = &os_info)
}

bool getCPUInfo(cpu_info **info) {

  EXPEND_INITIALIZER_BEGIN(cpu_info, cpu_info, *info = &cpu_info) {
    // 获取CPU信息
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open()) {
      std::string line;
      int physicalCores = 0;
      int logicalCores = 0;
      int flag = 7;

      while (flag != 0 && std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos) {
          static std::string cpu_name =
              line.substr(line.find(":") + 2); /// 静态就不担心内存释放问题了
          cpu_info.name = cpu_name.c_str();
          flag ^= 4;
        }
        if (line.find("cpu cores") != std::string::npos) {
          physicalCores = std::stoi(line.substr(line.find(":") + 2));
          flag ^= 2;
        }
        if (line.find("siblings") != std::string::npos) {
          logicalCores = std::stoi(line.substr(line.find(":") + 2));
          flag ^= 1;
        }
      }

      cpu_info.cores = physicalCores;
      cpu_info.threads = logicalCores;
    } else {
      cpu_info.name = "Unknown";
      cpu_info.cores = sysconf(_SC_NPROCESSORS_ONLN);
      cpu_info.threads = cpu_info.cores;
    }
  }
  EXPEND_INITIALIZER_END(*info = &cpu_info)
}

bool getGPUInfo(gpu_info **info, size_t *count) {
  EXPEND_INITIALIZER_BEGIN(std::vector<gpu_info>, gpu_infos,
                           *info = gpu_infos.data();
                           *count = gpu_infos.size(); return *count > 0) {

    // 方法1: 尝试通过nvidia-smi获取(NVIDIA显卡)
    FILE *nvidiaPipe = popen("nvidia-smi --query-gpu=name,memory.total "
                             "--format=csv,noheader,nounits 2>/dev/null",
                             "r");
    if (nvidiaPipe) {
      char buffer[256];

      while (fgets(buffer, sizeof(buffer), nvidiaPipe)) {
        gpu_info gpu = {0};
        std::string line(buffer);
        size_t commaPos = line.find(',');
        if (commaPos != std::string::npos) {
          gpu.name = strdup(line.substr(0, commaPos).c_str());
          gpu.memory =
              std::stoull(line.substr(commaPos + 1)) * 1024 * 1024; // MB转字节
          gpu_infos.push_back(gpu);
        }
      }
      pclose(nvidiaPipe);
      if (!gpu_infos.empty()) {
        return true;
      }
    }

    // 方法2: 尝试通过DRM接口获取(AMD/Intel显卡)
    for (int i = 0; i < 4; i++) {
      std::string path = "/sys/class/drm/card" + std::to_string(i) +
                         "/device/mem_info_vram_total";
      std::ifstream vramFile(path);
      if (vramFile.is_open()) {
        gpu_info gpu = {0};
        uint64_t vramSize;
        vramFile >> vramSize;
        gpu.memory = vramSize;

        // 获取GPU名称
        std::string vendorPath =
            "/sys/class/drm/card" + std::to_string(i) + "/device/vendor";
        std::ifstream vendorFile(vendorPath);
        if (vendorFile.is_open()) {
          std::string vendor;
          vendorFile >> vendor;
          gpu.name = strdup((vendor == "0x1002"   ? "AMD"
                             : vendor == "0x8086" ? "Intel"
                                                  : "Unknown"));
        }
        gpu_infos.push_back(gpu);
        break;
      }
    }

    // 方法3: 回退到lspci方法
    if (gpu_infos.empty()) {
      FILE *pipe = popen("lspci -vmm | grep -A 9 'VGA\\|3D\\|Display'", "r");
      if (pipe) {
        char buffer[256];
        gpu_info current_gpu = {0};

        while (fgets(buffer, sizeof(buffer), pipe)) {
          std::string line(buffer);

          if (line.find("Slot:") != std::string::npos) {
            if (current_gpu.name) {
              gpu_infos.push_back(current_gpu);
              current_gpu = {0};
            }
          } else if (line.find("Device:") != std::string::npos) {
            current_gpu.name = strdup(line.substr(line.find(":") + 2).c_str());
          }
        }

        if (current_gpu.name) {
          gpu_infos.push_back(current_gpu);
        }
        pclose(pipe);
      }
    }
  }
  EXPEND_INITIALIZER_END(*info = gpu_infos.data(); *count = gpu_infos.size();
                         return *count > 0)
}

bool getRAMInfo(uint64_t **info, size_t *count) {
  EXPEND_INITIALIZER_BEGIN(std::vector<uint64_t>, ram_info,
                           *info = ram_info.data();
                           *count = ram_info.size(); return *count > 0) {
    // 方法1: 尝试通过EDAC接口获取内存条信息(不需要root权限)
    for (int mc = 0; mc < 4; mc++) {
      for (int csrow = 0; csrow < 8; csrow++) {
        std::string path = "/sys/devices/system/edac/mc/mc" +
                           std::to_string(mc) + "/csrow" +
                           std::to_string(csrow) + "/size";
        std::ifstream sizeFile(path);
        if (sizeFile.is_open()) {
          uint64_t sizeMB;
          sizeFile >> sizeMB;
          if (sizeMB > 0) {
            ram_info.push_back(sizeMB * 1024 * 1024); // 转换为字节
          }
          sizeFile.close();
        }
      }
    }

    // 方法2: 如果前两种方法都失败，使用sysinfo获取总内存
    if (ram_info.empty()) {
      struct sysinfo memInfo;
      if (sysinfo(&memInfo) == 0) {
        ram_info.push_back(memInfo.totalram * memInfo.mem_unit);
      }
    }
  }
  EXPEND_INITIALIZER_END(*info = ram_info.data(); *count = ram_info.size();
                         return *count > 0)
}
#endif