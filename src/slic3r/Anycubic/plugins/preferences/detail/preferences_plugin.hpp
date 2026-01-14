#pragma once

#include "plugins_manager/plugins_manager.hxx"

#include <boost/preprocessor/cat.hpp>

#include "plugins_base/plugins.hxx"
#include "plugins_base/plugins_base.hxx"

#include <vector>

#define PLGUINS_NAME preferences
#define PLUGIN_NAME_STR BOOST_PP_STRINGIZE(PLGUINS_NAME)

class PreferencesPlugin : public Anycubic::Plugins::Plugin {
public:
  PreferencesPlugin(Anycubic::Plugins::PluginHost *host);
  virtual ~PreferencesPlugin() {}

public:
  void CreateTabs(wxWindow *parent);

private:
  bool register_tab(const wxString &plugin_name, const wxString &title,
                    const wxString &icon, const wxString &xrcName,
                    const wxString *xrc);
  void open(void) const;

private:
  // Anycubic::Plugins::Plugin interface
  const char *Name(void) override { return PLUGIN_NAME_STR; }
  bool Start(void) override { return true; }
  void Stop(void) override {}
  bool AttachEvt(wxEvtHandler *evt) override { return false; }
  bool DetachEvt(wxEvtHandler *evt) override { return false; }
  bool BindEvt(class wxPanel* panel, wxWindow* parent = nullptr,
                   wxString *bmp = nullptr) override {
    return false;
  }
  bool CreateWebview(wxWebView *view, wxWindow *parent = nullptr,
                     wxString *bmp = nullptr) override {
    return false;
  }
  void Destroy(void) override { delete this; }

private:
  Anycubic::Plugins::PluginHost *host_;
  struct Tab {
    wxString pluginName;
    wxString title;
    wxString iconName;
    wxString xrcName;
    wxString xrc;
  };
  std::vector<Tab> tabs_;
};
