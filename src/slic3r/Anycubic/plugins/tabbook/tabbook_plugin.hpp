#pragma once
#include "plugins_manager/plugins_manager.hxx"

#include <boost/preprocessor/cat.hpp>

#include "plugins_base/plugins.hxx"
#include "plugins_base/plugins_base.hxx"

#define PLGUINS_NAME tabbook
#define PLUGIN_NAME_STR BOOST_PP_STRINGIZE(PLGUINS_NAME)

class TabBookPlugin : public Anycubic::Plugins::Plugin {
public:
  TabBookPlugin(Anycubic::Plugins::PluginHost *host);
  virtual ~TabBookPlugin();

private:
  int32_t CreateTab(int idx, const std::string &title, const std::string &icon,
                    wxWindow *panel);
  int32_t RemoveTab(int idx);
  int32_t GetTabCount(void) const;
  std::string GetTabTitle(int idx) const;

private:
  const char *Name(void) override { return PLUGIN_NAME_STR; };
  bool Start(void) override { return true; };
  void Stop(void) override {}
  bool AttachEvt(class wxEvtHandler *) override;
  bool DetachEvt(class wxEvtHandler *) override;
  bool BindEvt(class wxPanel* panel, class wxWindow* parent = nullptr,
                   class wxString *bmp = nullptr) override;
  bool CreateWebview(class wxWebView *view, class wxWindow *parent = nullptr,
                     class wxString *bmp = nullptr) override;
  void Destroy(void) override;

private:
  Anycubic::Plugins::PluginHost *host_;
};