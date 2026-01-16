#include "preset_update.hpp"

bool PresetUpdate::has_package(const wxString &name) { return false; }
bool PresetUpdate::check_update(const wxString &name, bool manual_check) {
  return false;
}
bool PresetUpdate::start_download(const wxString &name) { return false; }

PresetUpdate::PresetUpdate(const wxString &package_dir,
                           const wxString &plugins_dir) {}
