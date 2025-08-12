#include "tabbook_plugin.hpp"

#include <boost/preprocessor/stringize.hpp>

PLUGIN_EXPORT_GETINFO(, tabbok) {
  static PluginInfo info{
      .plugin_api = 1,
      .plugin_type = 0xe,
      .dependency_count = 0,
      .dependency = nullptr,
      .name = BOOST_PP_STRINGIZE(prefix), .description = BOOST_PP_STRINGIZE(prefix), .version = 1,
                                          };
  return &info;
}
PLUGIN_EXPORT_SETUP(, tabbook) { return new TabBookPlugin(host); }
