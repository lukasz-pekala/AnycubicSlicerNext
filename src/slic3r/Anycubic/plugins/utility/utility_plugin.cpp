#include "utility_plugin.hpp"
#include "slic3r/Anycubic/detail/anonymous.hpp"

#include <libslic3r/AppConfig.hpp>
#include <slic3r/GUI/GUI_App.hpp>

#include <plugins_base/funcation.hxx>

#include <wx/filename.h>

UtilityPlugin::UtilityPlugin(Anycubic::Plugins::PluginHost *host)
    : host_(host) {
  assert(host_ != nullptr);
  auto router = host_->Router();
  assert(router != nullptr);
  router->REGISTER_FUNCATION(UtilityPlugin, pcid);
  router->REGISTER_FUNCATION(UtilityPlugin, is_test_env);
  router->REGISTER_FUNCATION(UtilityPlugin, is_china_env);
  router->REGISTER_FUNCATION(UtilityPlugin, language);
  router->REGISTER_FUNCATION(UtilityPlugin, get_temp_path);
  router->REGISTER_FUNCATION(UtilityPlugin, get_download_path);
  router->REGISTER_FUNCATION(UtilityPlugin, set_download_path);
  router->REGISTER_FUNCATION(UtilityPlugin, set_access_token);
  router->REGISTER_FUNCATION(UtilityPlugin, get_access_token);
  router->REGISTER_FUNCATION(UtilityPlugin, is_auto_login);
  router->REGISTER_FUNCATION(UtilityPlugin, set_auto_login);
  router->REGISTER_FUNCATION(UtilityPlugin, get_login_token);
  router->REGISTER_FUNCATION(UtilityPlugin, set_login_token);
  router->REGISTER_FUNCATION(UtilityPlugin, get_user_info);
  router->REGISTER_FUNCATION(UtilityPlugin, set_user_info);
}

UtilityPlugin::~UtilityPlugin() {}

std::string UtilityPlugin::pcid(void) const {
  return Slic3r::GetPCID(nullptr).ToStdString();
}
bool UtilityPlugin::is_test_env(void) const {
  wxString developer_mode;
  host_->GetValue("developer_mode", developer_mode);
  return developer_mode == "1";
}
bool UtilityPlugin::is_china_env(void) const {
  // TODO: 获取真实现的区域信息
  return language() == "zh_CN";
}
std::string UtilityPlugin::language(void) const {
  wxString developer_mode;
  host_->GetValue("language", developer_mode);
  return developer_mode.utf8_string();
}
std::string UtilityPlugin::get_temp_path(void) const {
  wxStandardPaths &standardPaths = wxStandardPaths::Get();
  wxFileName tmpDir = wxFileName::DirName(standardPaths.GetTempDir());
  tmpDir.AppendDir(wxASCII_STR(SLIC3R_APP_NAME));
  if (!tmpDir.DirExists()) {
    tmpDir.Mkdir();
  }
  return tmpDir.GetFullPath().utf8_string();
}

std::string UtilityPlugin::get_download_path() const { return download_path_; }

void UtilityPlugin::set_download_path(const std::string &path) {
  download_path_ = path;
}
std::string UtilityPlugin::get_access_token(void) const {
  return access_token_;
}
void UtilityPlugin::set_access_token(const std::string &token) {
  access_token_ = token;
}
bool UtilityPlugin::is_auto_login(void) const {
  wxString auto_login;
  host_->GetValue("user/auto_login", auto_login);
  auto_login.Lower();
  return auto_login == "true" || auto_login == "1" || auto_login == "on" ||
         auto_login == "yes" || auto_login == "y" || auto_login == "t";
}
void UtilityPlugin::set_auto_login(bool auto_login) {
  host_->SetValue("user/auto_login", auto_login ? "true" : "false");
}
void UtilityPlugin::get_login_token(wxString *token) {
  host_->GetEncryptValue("user/login_token", *token);
}
void UtilityPlugin::set_login_token(const wxString *token) {
  host_->SetEncryptValue("user/login_token", *token);
}
void UtilityPlugin::get_user_info(wxString *username, wxString *password) {
  host_->GetEncryptValue("user/username", *username);
  host_->GetEncryptValue("user/password", *password);
}
void UtilityPlugin::set_user_info(const wxString *username,
                                  const wxString *password) {
  host_->SetEncryptValue("user/username", *username);
  host_->SetEncryptValue("user/password", *password);
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
