#include "utility_plugin.hpp"
#include "slic3r/Anycubic/detail/anonymous.hpp"

#include <libslic3r/AppConfig.hpp>
#include <slic3r/GUI/GUI_App.hpp>

#include <plugins_base/funcation.hxx>

UtilityPlugin::UtilityPlugin(Anycubic::Plugins::PluginHost *host)
    : host_(host) {
  assert(host_ != nullptr);
  auto router = host_->Router();
  assert(router != nullptr);
  router->REGISTER_FUNCATION(UtilityPlugin, pcid);
  router->REGISTER_FUNCATION(UtilityPlugin, is_test_env);
  router->REGISTER_FUNCATION(UtilityPlugin, is_china_env);
  router->REGISTER_FUNCATION(UtilityPlugin, language);

  appconf_ = Slic3r ::GUI::wxGetApp().app_config;
}

UtilityPlugin::~UtilityPlugin() {}

std::string UtilityPlugin::pcid(void) const {
  return Slic3r::GetPCID(appconf_).ToStdString();
}
bool UtilityPlugin::is_test_env(void) const {
  return appconf_->get_bool("developer_mode");
}
bool UtilityPlugin::is_china_env(void) const {
  // TODO: 获取真实现的区域信息
  return language() == "zh_CN";
}
std::string UtilityPlugin::language(void) const {
  return appconf_->get("language");
}
bool UtilityPlugin::AttachEvt(wxEvtHandler *) { return false; }

bool UtilityPlugin::DetachEvt(wxEvtHandler *) { return false; }

bool UtilityPlugin::CreateDialog(wxDialog *dlg, wxWindow *parent,
                                 wxString *bmp) {
  return false;
}

bool UtilityPlugin::CreateWebview(wxWebView *view, wxWindow *parent,
                                  wxString *bmp) {
  return false;
}

void UtilityPlugin::Destroy(void) { delete this; }
