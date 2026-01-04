#include "notify_plugin.hpp"


#include <slic3r/GUI/GUI_App.hpp>
#include <slic3r/GUI/NotificationManager.hpp>


#include <plugins_base/funcation.hxx>


NotifyPlugin::NotifyPlugin(Anycubic::Plugins::PluginHost* host) : host_(host),ntf_mngr_ (Slic3r::GUI::wxGetApp().notification_manager()) { 
    assert(ntf_mngr_ != nullptr);
    auto router = host_->Router(); 
    assert(router != nullptr);
    router->REGISTER_FUNCATION(NotifyPlugin, popup_notification);
}

NotifyPlugin::~NotifyPlugin() {}

void NotifyPlugin::popup_notification(bool success, const wxString& title, const wxString& msg) {

    auto type = success? Slic3r::GUI::NotificationType::PlaterError: Slic3r::GUI::NotificationType::PlaterWarning;
    ntf_mngr_->push_notification(type, NotificationManager::NotificationLevel::ErrorNotificationLevel,
                                 msg.utf8_string(), title.utf8_string(), nullptr, 10);
}

bool NotifyPlugin::Start(void) { return true; }
void NotifyPlugin::Stop(void) {}
bool NotifyPlugin::CreateWebview(class wxWebView* view, class wxWindow* parent, class wxString* bmp) { return false; }