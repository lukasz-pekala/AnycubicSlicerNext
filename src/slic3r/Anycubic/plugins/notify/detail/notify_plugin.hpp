#pragma once
#include "plugins_manager/plugins_manager.hxx"

#include <boost/preprocessor/cat.hpp>

#include "plugins_base/plugins.hxx"
#include "plugins_base/plugins_base.hxx"

#define PLGUINS_NAME notify
#define PLUGIN_NAME_STR BOOST_PP_STRINGIZE(PLGUINS_NAME)

namespace Slic3r::GUI {
class NotificationManager;
} // namespace Slic3r::GUI
using Slic3r::GUI::NotificationManager;


class NotifyPlugin : public Anycubic::Plugins::Plugin {
public:
  NotifyPlugin(Anycubic::Plugins::PluginHost *host);
  virtual ~NotifyPlugin();

private:
/**
 * @brief 气泡提示
 * 
 * @param success 是否成功
 * @param title 标题
 * @param msg 消息
 */
void popup_notification(bool success,const wxString&title, const wxString& msg);

private:
  // Anycubic::Plugins::Plugin
  const char *Name(void) override { return PLUGIN_NAME_STR; };
  bool Start(void) override;
  void Stop(void) override;
  bool CreateWebview(class wxWebView *view, class wxWindow *parent = nullptr,
                     class wxString *bmp = nullptr) override;
  bool AttachEvt(class wxEvtHandler*) override { return false; };
  bool DetachEvt(class wxEvtHandler *) override { return false; };
  bool BindEvt(class wxPanel* panel, class wxWindow* parent = nullptr,
                   class wxString *bmp = nullptr) override {
    return false;
  }

  void Destroy(void) override { delete this; };
private:
  Anycubic::Plugins::PluginHost *host_;
   NotificationManager *ntf_mngr_;
};