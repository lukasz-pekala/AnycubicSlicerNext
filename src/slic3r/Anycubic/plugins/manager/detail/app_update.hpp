#pragma once

#include <wx/string.h>

class AppUpdate {
public:
  bool has_package(const wxString &name);
  bool check_update(const wxString &name, bool manual_check);
  bool start_download(const wxString &name);

public:
  AppUpdate(const wxString &package_dir, const wxString &plugins_dir);
};