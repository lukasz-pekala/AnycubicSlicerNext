#include "detail/home_dialog.hpp"

PLUGIN_EXPORT_GETINFO(, PLGUINS_NAME) {
  static PluginInfo info{0};
  info.plugin_api = 1;
  info.plugin_type = 0xe;
  info.dependency_count = 0;
  info.dependency = nullptr;
  info.name = PLUGIN_NAME_STR;
  info.description = PLUGIN_NAME_STR;
  info.version = 1;
  return &info;
}
PLUGIN_EXPORT_SETUP(, PLGUINS_NAME) { return new HomeDialog(host); }
