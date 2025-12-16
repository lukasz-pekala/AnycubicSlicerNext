#include "xrc_pluigns.hpp"
#include "handlers/xh_button.hpp"

namespace Slic3r::GUI {
template <typename T, typename... Args> void AddHandler(Args &&...args) {
  if constexpr (std::is_base_of_v<wxXmlResourceHandler, T>) {
    auto p = new T(std::forward<Args>(args)...);
    wxXmlResource::Get()->AddHandler(p);
  } else {
    wxLogError("AddHandler: %s is not a wxXmlResourceHandler",
               typeid(T).name());
  }
}

XrcPlugin::XrcPlugin(Anycubic::Plugins::PluginHost *host) {
  AddHandler<ButtonResourceHandler>();
}
XrcPlugin::~XrcPlugin() {}
} // namespace Slic3r::GUI