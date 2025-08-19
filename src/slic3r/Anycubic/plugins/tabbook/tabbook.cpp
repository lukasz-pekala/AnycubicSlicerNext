#include "tabbook_plugin.hpp"

#include <boost/preprocessor/stringize.hpp>

PLUGIN_EXPORT_GETINFO(, tabbook) {
    static PluginInfo info{0};
    info.plugin_api = 1;
    info.plugin_type = 0xe;
    info.dependency_count = 0;
    info.dependency = nullptr;
    info.name = BOOST_PP_STRINGIZE(prefix);
    info.description = BOOST_PP_STRINGIZE(prefix);
    info.version = 1;
  return &info;
}
PLUGIN_EXPORT_SETUP(, tabbook) { return new TabBookPlugin(host); }
