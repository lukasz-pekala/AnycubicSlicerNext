#pragma once
#include "plugins_manager/plugins_manager.hxx"

#include <boost/preprocessor/cat.hpp>
#include "plugins_base/plugins.hxx"
#include "plugins_base/plugins_base.hxx"

#define PLGUINS_NAME utility
#define PLUGIN_NAME_STR BOOST_PP_STRINGIZE(PLGUINS_NAME)

namespace Slic3r {
class AppConfig;
};
class UtilityPlugin : public Anycubic::Plugins::Plugin {
public:
  UtilityPlugin(Anycubic::Plugins::PluginHost *host);
  virtual ~UtilityPlugin();

private:

  std::string pcid(void) const;
  bool is_test_env(void) const;
  bool is_china_env(void) const;
  bool dark_mode(void) const;
  std::string region(void) const;
  std::string language(void) const;
  std::string get_temp_path(void) const;
  std::string get_download_path() const;
  void set_download_path(const std::string &path);


  bool is_auto_login(void) const;
  void set_auto_login(bool auto_login);
  bool get_access_token(wxString *token);
  void set_access_token(const wxString *token);
  void get_user_info(wxString *username, wxString *password);
  void set_user_info(const wxString *username, const wxString *password);
  void clear_login_info(void);

  std::string get_slic3r_version(void) const;
  std::string get_app_version(void) const;
  int32_t get_app_version_code(void)const;
private:
private:
  // Anycubic::Plugins::Plugin
  const char *Name(void) override { return PLUGIN_NAME_STR; };
  bool Start(void) override { return true; };
  void Stop(void) override {}
  bool        AttachEvt(class wxEvtHandler*) override { return false; }
  bool        DetachEvt(class wxEvtHandler*) override { return false; }
  bool BindEvt(class wxPanel* panel, class wxWindow* parent = nullptr,
                   class wxString *bmp = nullptr) override;
  bool CreateWebview(class wxWebView *view, class wxWindow *parent = nullptr,
                     class wxString *bmp = nullptr) override;
  void Destroy(void) override;

private:
  Anycubic::Plugins::PluginHost *host_;
  std::string download_path_;

  std::vector<wxEvtHandler*> m_evt_list;
  std::mutex                 mtx_;
};