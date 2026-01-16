#pragma once
#include <boost/preprocessor/cat.hpp>

#include "plugins_base/plugins.hxx"
#include "plugins_base/plugins_base.hxx"

#include <memory>
#include <vector>

#define PLGUINS_NAME manager
#define PLUGIN_NAME_STR BOOST_PP_STRINGIZE(PLGUINS_NAME)

struct Instace;

class ManagerPlugin : public Anycubic::Plugins::Plugin {
public:
  ManagerPlugin(Anycubic::Plugins::PluginHost *host);

private:
  // Anycubic::Plugins::Plugin
  const char *Name(void) override { return PLUGIN_NAME_STR; };
  bool Start(void) override;
  void Stop(void) override;
  bool AttachEvt(class wxEvtHandler *) override { return false; };
  bool DetachEvt(class wxEvtHandler *) override { return false; };
  bool BindEvt(class wxPanel *panel, class wxWindow *parent = nullptr,
               class wxString *bmp = nullptr) override {
    return false;
  }
  bool CreateWebview(class wxWebView *view, class wxWindow *parent = nullptr,
                     class wxString *bmp = nullptr) override {
    return false;
  }
  void Destroy(void) override { delete this; };

private:
  bool has_package(const wxString &name);
  bool start_download_plugin(const wxString &name);
  bool check_update(const wxString &name, int manual_check);

private:
  Anycubic::Plugins::PluginHost *host_;
  std::vector<std::shared_ptr<Instace>> instances_;
};