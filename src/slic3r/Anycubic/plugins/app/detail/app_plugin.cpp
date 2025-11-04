#include "app_plugin.hpp"

AppPlugin::AppPlugin(Anycubic::Plugins::PluginHost *host) : host_(host) {}

AppPlugin::~AppPlugin() {}

int32_t AppPlugin::import(int32_t import_type, const wxString *paths) {
  return -1;
}
