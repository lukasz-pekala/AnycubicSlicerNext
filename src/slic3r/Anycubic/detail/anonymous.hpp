#pragma once
#include <stddef.h>
#include <wx/string.h>

namespace Slic3r {
wxString GetMD5HexString(char *buffer, size_t length);
wxString GetPCID(class AppConfig *app_config);
wxString getSerialNumber(void);

} // namespace Slic3r
