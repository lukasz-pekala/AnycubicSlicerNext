#include "utility_plugin.hpp"

#include "slic3r/Anycubic/detail/anonymous.hpp"

#include <slic3r/GUI/GUI_App.hpp>
#include <libslic3r/AppConfig.hpp>

#include <plugins_base/funcation.hxx>
#include <string>

UtilityPlugin::UtilityPlugin(Anycubic::Plugins::PluginHost *host):host_(host) {
  assert(host_ != nullptr);
  auto router = host_->Router();
  assert(router != nullptr);
  router->REGISTER_FUNCATION(UtilityPlugin, pcid);
  router->REGISTER_FUNCATION(UtilityPlugin, is_test_env);
  router->REGISTER_FUNCATION(UtilityPlugin, is_china_env);
  router->REGISTER_FUNCATION(UtilityPlugin, dark_mode);
  router->REGISTER_FUNCATION(UtilityPlugin, region);
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
  router->REGISTER_FUNCATION(UtilityPlugin, clear_login_info);
  router->REGISTER_FUNCATION(UtilityPlugin, get_slic3r_version);
  router->REGISTER_FUNCATION(UtilityPlugin, get_app_version);
  router->REGISTER_FUNCATION(UtilityPlugin, get_app_version_code);


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
  // 获取真实现的区域信息
  return region() != "Global"; 
}
bool UtilityPlugin::dark_mode(void) const {
  return Slic3r::GUI::wxGetApp().dark_mode();
}

std::string UtilityPlugin::region(void) const {
  wxString region;
  host_->GetValue("region", region);
  return region.utf8_string();
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
  if(host_->GetValue("user/auto_login", auto_login)){
    auto_login.Lower();
    return auto_login == "true" || auto_login == "1" || auto_login == "on" ||
          auto_login == "yes" || auto_login == "y" || auto_login == "t";
  }
  return false;
}
void UtilityPlugin::set_auto_login(bool auto_login) {
  host_->SetValue("user/auto_login", auto_login ? "true" : "false");
}
bool UtilityPlugin::get_login_token(wxString *token) {
  return host_->GetEncryptValue("user/login_token", *token);
}
void UtilityPlugin::set_login_token(const wxString *token) {
  host_->SetEncryptValue("user/login_token", *token);
  set_auto_login(!token->IsEmpty());
}
void UtilityPlugin::get_user_info(wxString *username, wxString *password) {
  host_->GetEncryptValue("user/username", *username);
  host_->GetEncryptValue("user/password", *password);
}
void UtilityPlugin::set_user_info(const wxString *username,
                                  const wxString *password) {
  host_->SetEncryptValue("user/username", *username);
  host_->SetEncryptValue("user/password", *password);
  set_auto_login(!username->IsEmpty());
}

void UtilityPlugin::clear_login_info(void) {
  wxString empty_string;
  if(is_china_env()){
    set_user_info(&empty_string, &empty_string);
  }else{
    set_login_token(&empty_string);
  }
}

std::string UtilityPlugin::get_slic3r_version(void) const { return SLIC3R_VERSION; }
std::string UtilityPlugin::get_app_version(void) const { return SoftFever_VERSION; }
int32_t UtilityPlugin::get_app_version_code(void) const { return VERSION_CODE; }

bool UtilityPlugin::BindEvt(wxPanel* panel, wxWindow* parent,
                                wxString *bmp) {
  return false;
}

bool UtilityPlugin::CreateWebview(wxWebView *view, wxWindow *parent,
                                  wxString *bmp) {
  return false;
}

void UtilityPlugin::Destroy(void) { delete this; }
