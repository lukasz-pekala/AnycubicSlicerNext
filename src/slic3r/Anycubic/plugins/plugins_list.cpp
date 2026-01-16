#include "plugins_list.hpp"

#include <plugins_list.h>

#include <boost/preprocessor.hpp>

#include <plugins_base/plugins_base.hxx>
#include <plugins_manager/detail/LibraryBase.hxx>

#define PLUGINS_LIST_SEQ                                                       \
  BOOST_PP_SEQ_POP_BACK(BOOST_PP_TUPLE_TO_SEQ(PLUGINS_LIST))

#define DECLARE_PLUGIN_FUNCTION(r, nil, elem)                                  \
  struct PluginInfo *BOOST_PP_CAT(elem, _getInfo)(void);                       \
  struct Anycubic::Plugins::Plugin *BOOST_PP_CAT(elem, _setup)(                \
      struct Anycubic::Plugins::PluginHost * host);

BOOST_PP_SEQ_FOR_EACH(DECLARE_PLUGIN_FUNCTION, nil, PLUGINS_LIST_SEQ)

#define DECLARE_FUNCTION(r, suffix, elem) BOOST_PP_CAT(elem, suffix),

#define DEFINED_FUNCTION(r, suffix, elem)                                      \
  std::shared_ptr<LibraryBase> BOOST_PP_CAT(elem, suffix)() {                  \
    return std::shared_ptr<LibraryBase>(new LibraryStatic(                     \
        BOOST_PP_CAT(elem, _getInfo), BOOST_PP_CAT(elem, _setup)));            \
  }

BOOST_PP_SEQ_FOR_EACH(DEFINED_FUNCTION, _create, PLUGINS_LIST_SEQ)
namespace Anycubic::Plugins {
std::vector<create_library_t> &GetPluginsList() {
  static std::vector<create_library_t> plugins_list{BOOST_PP_SEQ_FOR_EACH(
      DECLARE_FUNCTION, _create, PLUGINS_LIST_SEQ) nullptr};
  return plugins_list;
}
} // namespace Anycubic::Plugins
