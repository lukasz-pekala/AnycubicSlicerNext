#include <wx/file.h>
#include <wx/process.h>
#include <wx/string.h>
#include <wx/utils.h>
namespace Slic3r {
#ifdef __WXMSW__
wxString getSerialNumber(void) {
  wxString serialNumber = "";

  // Windows implementation using WMI query
  wxString cmd = "wmic bios get serialnumber /format:list";
  wxArrayString output, errors;
  long result = wxExecute(cmd, output, errors, wxEXEC_NODISABLE);

  if (result == 0) {
    for (const auto &line : output) {
      if (line.StartsWith("SerialNumber=")) {
        serialNumber = line.AfterFirst('=');
        serialNumber.Trim();
        break;
      }
    }
  }

  // If WMI query failed, try PowerShell as fallback
  if (serialNumber.IsEmpty()) {
    cmd = "powershell.exe -Command "(Get - WmiObject - Class Win32_BIOS)
              .SerialNumber "";
    result = wxExecute(cmd, output, errors, wxEXEC_NODISABLE);

    if (result == 0 && !output.IsEmpty()) {
      serialNumber = output[0];
      serialNumber.Trim();
    }
  }

  return serialNumber;
}
#elif __WXGTK__
wxString getSerialNumber(void) {
  wxString serialNumber = "";

  // Linux implementation by reading sysfs files (no root required)
  // Try product serial first
  wxFile file("/sys/class/dmi/id/product_serial");
  if (file.IsOpened()) {
    file.ReadAll(&serialNumber);
    serialNumber.Trim();
    file.Close();
  }

  // If product serial is empty or unavailable, try board serial
  if (serialNumber.IsEmpty()) {
    wxFile file2("/sys/class/dmi/id/board_serial");
    if (file2.IsOpened()) {
      file2.ReadAll(&serialNumber);
      serialNumber.Trim();
      file2.Close();
    }
  }

  // If still empty, try system UUID as alternative
  if (serialNumber.IsEmpty()) {
    wxFile file3("/sys/class/dmi/id/product_uuid");
    if (file3.IsOpened()) {
      file3.ReadAll(&serialNumber);
      serialNumber.Trim();
      file3.Close();
    }
  }

  return serialNumber;
}
#endif
} // namespace Slic3r
