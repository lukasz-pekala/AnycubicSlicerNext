#include "os.hpp"
#include "os_common.hpp"

#ifdef __APPLE__
#include <string.h>

#include <TargetConditionals.h>
#include <sys/sysctl.h>
#if TARGET_OS_MAC
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <mach/mach.h>
#include <Foundation/Foundation.h>
#endif

#include <atomic>
#include <vector>
#include <string>


std::string getArch(void) {
  char machine[128];
   size_t size = sizeof(machine);
  if (sysctlbyname("hw.machine", machine, &size, NULL, 0) == 0) {
    return std::string(machine);
  }
  return std::string("arm64");
}

bool isArmArch(void) {
  auto arch = getArch();
  return arch == "arm64";
}

std::string getKernelVersion() {
  char kernel_version[256];
  size_t size = sizeof(kernel_version);
  if (sysctlbyname("kern.osrelease", kernel_version, &size, NULL, 0) == 0) {
    return kernel_version;
  }
  return std::string();
}

bool getOpratingSystemInfo(operating_system_info **info) {
  EXPEND_INITIALIZER_BEGIN(operating_system_info, os_info, *info = &os_info) {
    static std::string arch = getArch();
    os_info.arch = arch.c_str();
#if TARGET_OS_MAC
    static std::string version;
    @autoreleasepool {
      NSProcessInfo *processInfo = [NSProcessInfo processInfo];
      NSString *versionString = [processInfo operatingSystemVersionString];
      version =[versionString UTF8String];
    }
    // 根据空格截取版本号
    // Version 13.7.8 (Build 22H730)
    size_t first = version.find(' ');
    if (first != std::string::npos) {
      auto second =  version.find(' ',first+1);
      if (second != std::string::npos) {
        version = version.substr(first+1, second-first-1);
      }else{
        version = version.substr(first+1);
      }
    }
    os_info.kernel_version = version.c_str();
    

#else
    #error "Unsupported operating system"
#endif
      os_info.name = "darwin";
  }
  EXPEND_INITIALIZER_END(*info = &os_info)
}

bool getCPUInfo(cpu_info **info) {
  EXPEND_INITIALIZER_BEGIN(cpu_info, cpu_info, *info = &cpu_info) {
    // 获取CPU品牌字符串
    static char brand[256]; /// 静态就不担心内存释放问题了
    size_t size = sizeof(brand);
    if (sysctlbyname("machdep.cpu.brand_string", brand, &size, NULL, 0) == 0) {
      cpu_info.name = brand;
    } else {
      cpu_info.name = "Unknown";
    }

    // 获取CPU核心数
    int cores = 0;
    size = sizeof(cores);
    sysctlbyname("hw.ncpu", &cores, &size, NULL, 0);
    cpu_info.cores = cores;

    // 获取CPU线程数
    int threads = 0;
    size = sizeof(threads);
    sysctlbyname("hw.logicalcpu", &threads, &size, NULL, 0);
    cpu_info.threads = threads;
  }
  EXPEND_INITIALIZER_END(*info = &cpu_info)
}

bool getGPUInfo(gpu_info **info, size_t *count) {
#if TARGET_OS_MAC
  EXPEND_INITIALIZER_BEGIN(std::vector<gpu_info>, gpu_infos,
                           *info = gpu_infos.data();
                           *count = gpu_infos.size(); return *count > 0) {
    CFMutableDictionaryRef matchDict =
        IOServiceMatching(kIOAcceleratorClassName);
    io_iterator_t iterator;

    // 兼容 macOS 11.3 及更早版本
    mach_port_t masterPort;
    if (IOMasterPort(MACH_PORT_NULL, &masterPort) == kIOReturnSuccess) {
      if (IOServiceGetMatchingServices(masterPort, matchDict, &iterator) ==
          kIOReturnSuccess) {
        io_registry_entry_t regEntry;

        while ((regEntry = IOIteratorNext(iterator))) {
          auto &info = gpu_infos.emplace_back();
          CFMutableDictionaryRef properties;
          if (IORegistryEntryCreateCFProperties(
                  regEntry, &properties, kCFAllocatorDefault, kNilOptions) ==
              kIOReturnSuccess) {
            // 获取GPU型号
            CFStringRef model =
                (CFStringRef)CFDictionaryGetValue(properties, CFSTR("model"));
            if (model && CFGetTypeID(model) == CFStringGetTypeID()) {
              char modelStr[256];
              if (CFStringGetCString(model, modelStr, sizeof(modelStr),
                                     kCFStringEncodingUTF8)) {
                info.name = strdup(modelStr);
              }
            }

            CFStringRef vramKeys[] = {CFSTR("VRAM,totalMB"),CFSTR( "vram-size"), CFSTR("VRAM Size"), CFSTR("vram-size-bytes")};
            for(auto &key : vramKeys){
              // 获取显存大小
              CFNumberRef vram = (CFNumberRef)CFDictionaryGetValue(
                  properties, key);
              if (vram && CFGetTypeID(vram) == CFNumberGetTypeID()) {
                int vramMB = 0;
                if (CFNumberGetValue(vram, kCFNumberIntType, &vramMB)) {
                  info.memory = vramMB;
                }

                // 假设键名包含"MB"的单位是MB，否则是字节
                char keyStr[256];
                if (CFStringGetCString(key, keyStr, sizeof(keyStr), kCFStringEncodingUTF8)) {
                    if (strstr(keyStr, "MB") != nullptr) {
                        info.memory *=  1024 * 1024; // 转换为bytes
                    } 
                }

               
                break;
              }
            }

            

            CFRelease(properties);
          }

          IOObjectRelease(regEntry);
        }
        IOObjectRelease(iterator);
      }
      mach_port_deallocate(mach_task_self(), masterPort);
    }
  }
  EXPEND_INITIALIZER_END(*info = gpu_infos.data(); *count = gpu_infos.size();
                         return *count > 0)
#else
  return false;
#endif
}

bool getRAMInfo(uint64_t **info, size_t *count) {
#if TARGET_OS_MAC
  EXPEND_INITIALIZER_BEGIN(std::vector<uint64_t>, ram_sizes,
                           *info = ram_sizes.data();
                           *count = ram_sizes.size(); return *count > 0) {
    io_iterator_t iter;
    mach_port_t masterPort;

    // 兼容 macOS 11.3 及更早版本
    if (IOMasterPort(MACH_PORT_NULL, &masterPort) == kIOReturnSuccess) {
      if (IOServiceGetMatchingServices(masterPort,
                                       IOServiceMatching("IOPlatformDevice"),
                                       &iter) == kIOReturnSuccess) {

        io_object_t device;
        while ((device = IOIteratorNext(iter))) {
          CFMutableDictionaryRef properties;
          if (IORegistryEntryCreateCFProperties(
                  device, &properties, kCFAllocatorDefault, kNilOptions) ==
              kIOReturnSuccess) {
            CFStringRef name =
                (CFStringRef)CFDictionaryGetValue(properties, CFSTR("name"));
            if (name && CFGetTypeID(name) == CFStringGetTypeID() && 
                CFStringCompare(name, CFSTR("mem"), 0) == kCFCompareEqualTo) {
              CFDataRef data =
                  (CFDataRef)CFDictionaryGetValue(properties, CFSTR("reg"));
              if (data) {
                const UInt8 *bytes = CFDataGetBytePtr(data);
                CFIndex length = CFDataGetLength(data);
                if (length >= 8) {
                  uint64_t size = *(uint64_t *)(bytes + length - 8);
                  ram_sizes.push_back(size);
                }
              }
            }
            CFRelease(properties);
          }
          IOObjectRelease(device);
        }
        IOObjectRelease(iter);
      }

      // 如果没有获取到具体内存条信息，则返回总内存
      if (ram_sizes.empty()) {
        uint64_t total_mem = 0;
        size_t size = sizeof(total_mem);
        sysctlbyname("hw.memsize", &total_mem, &size, NULL, 0);
        ram_sizes.push_back(total_mem);
      }
    }
  }
  EXPEND_INITIALIZER_END(*info = ram_sizes.data(); *count = ram_sizes.size();
                         return *count > 0);
#else
  return false;
#endif
}
#endif