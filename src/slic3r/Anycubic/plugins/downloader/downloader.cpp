
#include "detail/downloader_plugin.hpp"

PLUGIN_EXPORT_GETINFO(, PLGUINS_NAME) {
  const char *deps[] = {"utility"};
  static PluginInfo info{0};
  info.plugin_api = 1;
  info.plugin_type = 0xe;
  info.dependency_count = sizeof(deps) / sizeof(deps[0]);
  info.dependency = deps;
  info.name = PLUGIN_NAME_STR;
  info.description = PLUGIN_NAME_STR;
  info.version = 1;
  return &info;
}
PLUGIN_EXPORT_SETUP(, PLGUINS_NAME) { return new DownloaderPlugin(host); }