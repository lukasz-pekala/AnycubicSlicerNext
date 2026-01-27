#include "os.hpp"

#include "os_common.hpp"

#include "file_version.hpp"

#include <vector>

#ifdef _WIN32
#include <windows.h>

#include <Wbemidl.h>
#include <comdef.h>
#include <intrin.h>

#include <atomic>
#include <string>
#include <vector>

#include <boost/scope_exit.hpp>

bool isArmArch(void) { return false; }

auto getKernelVersion() {
  char PATH[MAX_PATH] = {0};
  GetWindowsDirectoryA(PATH, MAX_PATH);
  strcat(PATH, "\\System32\\ntoskrnl.exe");
  return getPEFileVersion(PATH);
}

bool getOpratingSystemInfo(operating_system_info **info) {
  EXPEND_INITIALIZER_BEGIN(operating_system_info, os_info, *info = &os_info) {
    // 获取系统架构
    os_info.arch = "x86_64";
    static auto version = getKernelVersion();
    os_info.kernel_version = version.c_str();
    os_info.name = "windows"; ///< 先给一个默认值，一会读不到也算有一个

    // 获取操作系统名称

    // 初始化COM库
    HRESULT initres = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    BOOST_SCOPE_EXIT(initres) {
      if (SUCCEEDED(initres)) {
        CoUninitialize();
      }
    }
    BOOST_SCOPE_EXIT_END;

    // 设置COM安全级别
    CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
                         RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

    // 创建WMI定位器实例
    IWbemLocator *pLoc = NULL;
    HRESULT hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                                    IID_IWbemLocator, (LPVOID *)&pLoc);

    if (FAILED(hres)) {
      return false;
    }

    // 连接到WMI服务
    IWbemServices *pSvc = NULL;
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0,
                               0, &pSvc);

    if (FAILED(hres)) {
      pLoc->Release();
      return false;
    }

    // 设置WMI服务的安全级别
    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                             RPC_C_AUTHN_LEVEL_CALL,
                             RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

    if (FAILED(hres)) {
      pSvc->Release();
      pLoc->Release();
      return false;
    }

    // 执行WMI查询
    IEnumWbemClassObject *pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"), bstr_t("SELECT Caption FROM Win32_OperatingSystem"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL,
        &pEnumerator);

    if (FAILED(hres)) {
      pSvc->Release();
      pLoc->Release();

      return false;
    }

    // 遍历查询结果
    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) ==
        WBEM_S_NO_ERROR) {
      VARIANT vtProp;

      // 获取操作系统名称
      hres = pclsObj->Get(L"Caption", 0, &vtProp, 0, 0);
      if (SUCCEEDED(hres)) {
        // 转换为utf8字符串
        int len = WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, NULL, 0,
                                      NULL, NULL);
        if (len > 0) {
          static std::string buffer(len, 0);
          WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, buffer.data(),
                              buffer.size(), NULL, NULL);
          os_info.name = buffer.c_str();
        }
        VariantClear(&vtProp);
      }
      pclsObj->Release();
    }

    // 释放资源
    pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
  }
  EXPEND_INITIALIZER_END(*info = &os_info)
}
static DWORD CountSetBits(ULONG_PTR bitMask) {
  DWORD count = 0;
  while (bitMask) {
    count += bitMask & 1;
    bitMask >>= 1;
  }
  return count;
}
bool getCPUInfo(cpu_info **info) {
  EXPEND_INITIALIZER_BEGIN(cpu_info, cpuinfo, *info = &cpuinfo) {
    // 获取CPU品牌字符串
    int CPUInfo[4] = {-1};
    static char brand[0x40] = {0}; /// 静态就不担心内存释放问题了

    __cpuid(CPUInfo, 0x80000002);
    memcpy(brand, CPUInfo, sizeof(CPUInfo));
    __cpuid(CPUInfo, 0x80000003);
    memcpy(brand + 16, CPUInfo, sizeof(CPUInfo));
    __cpuid(CPUInfo, 0x80000004);
    memcpy(brand + 32, CPUInfo, sizeof(CPUInfo));

    cpuinfo.name = brand;

    // 获取CPU核心和线程数
    DWORD length = 0;
    GetLogicalProcessorInformation(NULL, &length);
    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(
        length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
    GetLogicalProcessorInformation(buffer.data(), &length);

    DWORD physicalCores = 0;
    DWORD logicalCores = 0;
    for (const auto &info : buffer) {
      if (info.Relationship == RelationProcessorCore) {
        physicalCores++;
        logicalCores += CountSetBits(info.ProcessorMask);
      }
    }

    cpuinfo.cores = physicalCores;
    cpuinfo.threads = logicalCores;
  }
  EXPEND_INITIALIZER_END(*info = &cpuinfo)
}

bool getGPUInfo(gpu_info **info, size_t *count) {

  EXPEND_INITIALIZER_BEGIN(std::vector<gpu_info>, gpu_infos,
                           *info = gpu_infos.data();
                           *count = gpu_infos.size(); return *count > 0) {
    HRESULT hres;

    // 初始化COM
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
      return false;

    // 设置安全级别
    hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
                                RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE,
                                NULL);

    // 创建WMI连接
    IWbemLocator *pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator, (LPVOID *)&pLoc);

    IWbemServices *pSvc = NULL;
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0,
                               0, &pSvc);

    // 查询GPU信息
    IEnumWbemClassObject *pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"), bstr_t("SELECT * FROM Win32_VideoController"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL,
        &pEnumerator);

    // 遍历查询结果
    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
      HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
      if (0 == uReturn)
        break;

      VARIANT vtProp;
      gpu_info &gpu = gpu_infos.emplace_back();

      // 获取GPU名称
      hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
      if (SUCCEEDED(hr)) {
        gpu.name = _strdup(_bstr_t(vtProp.bstrVal));
        VariantClear(&vtProp);
      }

      // 获取显存大小
      hr = pclsObj->Get(L"AdapterRAM", 0, &vtProp, 0, 0);
      if (SUCCEEDED(hr)) {
        gpu.memory = vtProp.uintVal; // 转换为byte

        VariantClear(&vtProp);
      }
      pclsObj->Release();
    }

    // 清理资源
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
  }
  EXPEND_INITIALIZER_END(*info = gpu_infos.data(); *count = gpu_infos.size();
                         return *count > 0)
}

bool getRAMInfo(uint64_t **info, size_t *count) {
  EXPEND_INITIALIZER_BEGIN(std::vector<uint64_t>, ram_info,
                           *info = ram_info.data();
                           *count = ram_info.size(); return *count > 0) {
    HRESULT hres;

    // 初始化COM
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
      return false;

    // 设置安全级别
    hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
                                RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE,
                                NULL);

    // 创建WMI连接
    IWbemLocator *pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator, (LPVOID *)&pLoc);

    IWbemServices *pSvc = NULL;
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0,
                               0, &pSvc);

    // 查询内存信息
    IEnumWbemClassObject *pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"), bstr_t("SELECT * FROM Win32_PhysicalMemory"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL,
        &pEnumerator);

    // 遍历查询结果
    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
      HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
      if (0 == uReturn)
        break;

      VARIANT vtProp;

      // 获取单条内存大小(以字节为单位)
      hr = pclsObj->Get(L"Capacity", 0, &vtProp, 0, 0);
      if (SUCCEEDED(hr)) {
        if (vtProp.vt == VT_BSTR) {
          ram_info.push_back(wcstoull(vtProp.bstrVal, NULL, 10));
        } else if (vtProp.vt == VT_UI4) {
          ram_info.push_back(vtProp.ullVal);
        }

        VariantClear(&vtProp);
      }

      pclsObj->Release();
    }

    // 如果没有获取到内存信息，则获取总内存
    if (ram_info.empty()) {
      IEnumWbemClassObject *pEnum = NULL;
      hres = pSvc->ExecQuery(
          bstr_t("WQL"),
          bstr_t("SELECT TotalPhysicalMemory FROM Win32_ComputerSystem"),
          WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnum);

      if (SUCCEEDED(hres)) {
        IWbemClassObject *pObj = NULL;
        ULONG ret = 0;

        if (pEnum->Next(WBEM_INFINITE, 1, &pObj, &ret) == WBEM_S_NO_ERROR) {
          VARIANT vt;
          if (SUCCEEDED(pObj->Get(L"TotalPhysicalMemory", 0, &vt, 0, 0))) {
            ram_info.push_back(vt.ullVal);
            VariantClear(&vt);
          }
          pObj->Release();
        }
        pEnum->Release();
      }
    }

    // 清理资源
    pSvc->Release();
    pLoc->Release();
    if (pEnumerator)
      pEnumerator->Release();
    CoUninitialize();
  }
  EXPEND_INITIALIZER_END(*info = ram_info.data(); *count = ram_info.size();
                         return *count > 0)
}
#endif