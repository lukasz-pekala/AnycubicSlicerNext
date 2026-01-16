#include "detail/manager_plugin.hpp"
PLUGIN_EXPORT_GETINFO(, PLGUINS_NAME) {
  static const char *dependency[] = {"app", "home_dialog"};
  static PluginInfo info{0};
  info.plugin_api = 1;
  info.plugin_type = 0xe;
  info.dependency_count = sizeof(dependency) / sizeof(dependency[0]);
  info.dependency = dependency;
  info.name = PLUGIN_NAME_STR;
  info.description = PLUGIN_NAME_STR;
  info.version = 1;
  return &info;
}
PLUGIN_EXPORT_SETUP(, PLGUINS_NAME) { return new ManagerPlugin(host); }
