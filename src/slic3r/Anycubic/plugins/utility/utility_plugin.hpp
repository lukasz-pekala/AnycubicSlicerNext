#pragma once
#include "plugins_manager/plugins_manager.hxx"

#include <boost/preprocessor/cat.hpp>

#include "plugins_base/plugins.hxx"
#include "plugins_base/plugins_base.hxx"

class UtilityPlugin : public Anycubic::Plugins::Plugin {
public:
  UtilityPlugin(Anycubic::Plugins::PluginHost *host);
  virtual ~UtilityPlugin();

private:
  std::string pcid(void) const;

private:
  // Anycubic::Plugins::Plugin
  bool AttachEvt(class wxEvtHandler *) override;
  bool DetachEvt(class wxEvtHandler *) override;
  bool CreateDialog(class wxDialog *dlg, class wxWindow *parent = nullptr,
                    class wxString *bmp = nullptr) override;
  bool CreateWebview(class wxWebView *view, class wxWindow *parent = nullptr,
                     class wxString *bmp = nullptr) override;
  void Destroy(void) override;

private:
  Anycubic::Plugins::PluginHost *host_;
};