#pragma once
#include "plugins_manager/plugins_manager.hxx"

#include <boost/preprocessor/cat.hpp>

#include "plugins_base/plugins.hxx"
#include "plugins_base/plugins_base.hxx"

#define PLGUINS_NAME app
#define PLUGIN_NAME_STR BOOST_PP_STRINGIZE(PLGUINS_NAME)

namespace Slic3r {
class AppConfig;
};
class AppPlugin : public Anycubic::Plugins::Plugin {
public:
  AppPlugin(Anycubic::Plugins::PluginHost *host);
  virtual ~AppPlugin();

private:
  int32_t import(const wxString *paths);
  bool current_gcode_file(wxString *path);
  wxString recent_projects(void);

private:
  // Anycubic::Plugins::Plugin
  const char *Name(void) override { return PLUGIN_NAME_STR; };
  bool Start(void) override { return true; };
  void Stop(void) override {}
  bool AttachEvt(class wxEvtHandler *) override { return false; };
  bool DetachEvt(class wxEvtHandler *) override { return false; };
  bool BindEvt(class wxPanel* panel, class wxWindow* parent = nullptr,
                   class wxString *bmp = nullptr) override {
    return false;
  }
  bool CreateWebview(class wxWebView *view, class wxWindow *parent = nullptr,
                     class wxString *bmp = nullptr) override {
    return false;
  }
  void Destroy(void) override { delete this; };

private:
  Anycubic::Plugins::PluginHost *host_;
};