#pragma once
#include "plugins_manager/plugins_manager.hxx"

#include <boost/preprocessor/cat.hpp>

#include "plugins_base/plugins.hxx"
#include "plugins_base/plugins_base.hxx"

#define PLGUINS_NAME xrc
#define PLUGIN_NAME_STR BOOST_PP_STRINGIZE(PLGUINS_NAME)

namespace Slic3r::GUI {

class XrcPlugin : public Anycubic::Plugins::Plugin {
public:
  XrcPlugin(Anycubic::Plugins::PluginHost *host);
  virtual ~XrcPlugin();

private:
  // Anycubic::Plugins::Plugin
  const char *Name(void) override { return PLUGIN_NAME_STR; };
  bool Start(void) override { return true; };
  bool AttachEvt(class wxEvtHandler *) override { return false; }
  bool DetachEvt(class wxEvtHandler *) override { return false; }
  bool CreateDialog(class wxDialog *dlg, class wxWindow *parent = nullptr,
                    class wxString *bmp = nullptr) override {
    return false;
  }
  bool CreateWebview(class wxWebView *view, class wxWindow *parent = nullptr,
                     class wxString *bmp = nullptr) override {
    return false;
  }
  void Destroy(void) override { delete this; }

private:
};
} // namespace Slic3r::GUI
