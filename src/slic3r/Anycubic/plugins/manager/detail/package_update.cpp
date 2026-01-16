#include "package_update.hpp"

bool PackageUpdate::has_package(const wxString &name) { return false; }
bool PackageUpdate::check_update(const wxString &name, bool manual_check) {
  return false;
}
bool PackageUpdate::start_download(const wxString &name) { return false; }

PackageUpdate::PackageUpdate(const wxString &package_dir,
                             const wxString &plugins_dir) {}