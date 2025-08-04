#pragma once
#include <stddef.h>
#include <wx/string.h>

namespace Slic3r {
wxString GetMD5HexString(char *buffer, size_t length);
wxString GetPCID(void);

} // namespace Slic3r
