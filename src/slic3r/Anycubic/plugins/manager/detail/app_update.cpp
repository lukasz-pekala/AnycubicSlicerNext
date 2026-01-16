#include "app_update.hpp"

bool AppUpdate::has_package(const wxString &name) { return false; }
bool AppUpdate::check_update(const wxString &name, bool manual_check) {
  return false;
}
bool AppUpdate::start_download(const wxString &name) { return false; }

AppUpdate::AppUpdate(const wxString &package_dir, const wxString &plugins_dir) {
}