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
  std::string language(void) const;
  std::string get_temp_path(void) const;
  std::string get_download_path() const;
  void set_download_path(const std::string &path);

  std::string get_access_token(void) const;
  void set_access_token(const std::string &token);

  bool is_auto_login(void) const;
  void set_auto_login(bool auto_login);
  bool get_login_token(wxString *token);
  void set_login_token(const wxString *token);
  void get_user_info(wxString *username, wxString *password);
  void set_user_info(const wxString *username, const wxString *password);

private:
  // Anycubic::Plugins::Plugin
  const char *Name(void) override { return PLUGIN_NAME_STR; };
  bool Start(void) override { return true; };
  bool AttachEvt(class wxEvtHandler *) override;
  bool DetachEvt(class wxEvtHandler *) override;
  bool CreatePanel(class wxPanel *panel, class wxWindow *parent = nullptr,
                   class wxString *bmp = nullptr) override;
  bool CreateWebview(class wxWebView *view, class wxWindow *parent = nullptr,
                     class wxString *bmp = nullptr) override;
  void Destroy(void) override;

private:
  Anycubic::Plugins::PluginHost *host_;
  std::string download_path_;
  std::string access_token_;
};