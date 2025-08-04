#include "anonymous.hpp"

#include <common/utils/bin2ascii.h>
#include <common/utils/md5.h>

#include <vector>

#ifdef __WXMAC__
#import <IOKit/IOKitLib.h>
#include <dlfcn.h>
#elif defined(__WXMSW__)
#include <Iphlpapi.h>
#include <netlistmgr.h>
#include <windows.h>
#else

#endif

namespace Slic3r {
wxString GetPCID(void) {
  std::vector<unsigned char> unique;

#ifdef _WIN32
  // On Windows, get the MAC address of a network adaptor (preferably Ethernet
  // or IEEE 802.11 wireless
  DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
  PIP_ADAPTER_INFO AdapterInfo = (PIP_ADAPTER_INFO)malloc(dwBufLen);

  if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
    free(AdapterInfo);
    AdapterInfo = (IP_ADAPTER_INFO *)malloc(dwBufLen);
  }
  if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
    const IP_ADAPTER_INFO *pAdapterInfo = AdapterInfo;
    std::vector<std::vector<unsigned char>> macs;
    bool ethernet_seen = false;
    while (pAdapterInfo) {
      macs.emplace_back();
      for (unsigned char i = 0; i < pAdapterInfo->AddressLength; ++i)
        macs.back().emplace_back(pAdapterInfo->Address[i]);
      // Prefer Ethernet and IEEE 802.11 wireless
      if (!ethernet_seen) {
        if ((pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET &&
             (ethernet_seen = true)) ||
            pAdapterInfo->Type == IF_TYPE_IEEE80211)
          std::swap(macs.front(), macs.back());
      }
      pAdapterInfo = pAdapterInfo->Next;
    }
    if (!macs.empty())
      unique = macs.front();
  }
  free(AdapterInfo);
#elif __APPLE__
  constexpr int buf_size = 100;
  char buf[buf_size] = "";
  io_registry_entry_t ioRegistryRoot =
      IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
  if (ioRegistryRoot != MACH_PORT_NULL) {
    CFStringRef uuidCf = (CFStringRef)IORegistryEntryCreateCFProperty(
        ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
    IOObjectRelease(ioRegistryRoot);
    CFStringGetCString(uuidCf, buf, buf_size, kCFStringEncodingMacRoman);
    CFRelease(uuidCf);
  }
  unique.resize(strlen(buf));
  memcpy(unique.data(), buf, strlen(buf));
#else // Linux/BSD
  constexpr size_t max_len = 100;
  char cline[max_len] = "";
  FILE *fp = popen("cat /etc/machine-id", "r");
  if (fp != NULL) {
    // Maybe the only way to silence -Wunused-result on gcc...
    // cline is simply not modified on failure, who cares.
    [[maybe_unused]] auto dummy = fgets(cline, max_len, fp);
    pclose(fp);
  }
  // Now convert the string to std::vector<unsigned char>.
  for (char *c = cline; *c != 0; ++c)
    unique.emplace_back((unsigned char)(*c));
#endif

  // In case that we did not manage to get the unique info, just return an empty
  // string, so it is easily detectable and not masked by the hashing.
  if (unique.empty())
    return "";
  return GetMD5HexString(reinterpret_cast<char *>(unique.data()),
                         unique.size());
}
wxString GetMD5HexString(char *data, size_t length) {
  if (data == nullptr || length == 0)
    return wxEmptyString;
  char buffer[+MD5LEN * 2 + 1] = {0};
  char *pmd5str = buffer + MD5LEN;
  md5Sum(data, length, buffer);
  bin2hex(pmd5str, buffer, MD5LEN);
  return wxASCII_STR(pmd5str);
}
} // namespace Slic3r