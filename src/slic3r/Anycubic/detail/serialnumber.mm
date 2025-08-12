#include <easy_log/log.hxx>

#include <wx/string.h>
#include <wx/osx/core/cfstring.h>

#include <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <IOKit/IOKitLib.h>
namespace Slic3r {
wxString getSerialNumber(void) {
    wxString serialNumber = "";
    
    // 使用 Objective-C 代码获取序列号
    io_service_t platformExpert = IOServiceGetMatchingService(kIOMasterPortDefault, 
        IOServiceMatching("IOPlatformExpertDevice"));
    
    if (platformExpert) {
        CFStringRef serialNumberRef = (CFStringRef) IORegistryEntryCreateCFProperty(
            platformExpert, CFSTR("IOPlatformSerialNumber"), kCFAllocatorDefault, 0);
        
        if (serialNumberRef) {
            wxString temp = wxCFStringRef::AsString(serialNumberRef);
            if (!temp.IsEmpty()) {
                serialNumber = temp;
            } else {
                // 在黑苹果上可能获取到空字符串
                LOG_ERROR("Failed to get valid serial number from IOPlatformSerialNumber");
            }
            CFRelease(serialNumberRef);
        } else {
            // 在黑苹果上可能无法获取serialNumberRef
            LOG_ERROR("Failed to get IOPlatformSerialNumber property");
        }
        
        IOObjectRelease(platformExpert);
    } else {
        // 在黑苹果上可能找不到platformExpert
        LOG_ERROR("Failed to get IOPlatformExpertDevice");
    }
    
    // 如果获取失败，可以考虑使用其他标识符作为备选方案
    if (serialNumber.IsEmpty()) {
        LOG_ERROR("Serial number is empty, possibly on a Hackintosh system");
        
        // 使用 kIOPlatformUUIDKey 作为备份实现
        constexpr int buf_size = 100;
        char buf[buf_size] = "";
        io_registry_entry_t ioRegistryRoot =
            IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
        if (ioRegistryRoot != MACH_PORT_NULL) {
            CFStringRef uuidCf = (CFStringRef)IORegistryEntryCreateCFProperty(
                ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
            if (uuidCf) {
                CFStringGetCString(uuidCf, buf, buf_size, kCFStringEncodingMacRoman);
                CFRelease(uuidCf);
                serialNumber = wxString(buf);
            }
            IOObjectRelease(ioRegistryRoot);
        }
    }
    
    return serialNumber;
}
}
