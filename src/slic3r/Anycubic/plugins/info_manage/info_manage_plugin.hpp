#pragma once
#include "plugins_manager/plugins_manager.hxx"

#include <boost/preprocessor/cat.hpp>
#include <plugins_sdk/event/plugin_event.hxx>
#include "plugins_base/plugins.hxx"
#include "plugins_base/plugins_base.hxx"

#define PLGUINS_NAME info_manage
#define PLUGIN_NAME_STR BOOST_PP_STRINGIZE(PLGUINS_NAME)

namespace Slic3r {
class AppConfig;
};
class InfoManage : public Anycubic::Plugins::Plugin
{
public:
    InfoManage(Anycubic::Plugins::PluginHost* host);
    virtual ~InfoManage();

private:
  wxString get_default_gcode_file_name();
  std::string get_curr_plate_printer_model_name();
  std::string autoExport_gcode3mf(const std::string& fileIndex, bool export_all = false);
  int         get_curr_plate_Index();
  wxString    get_preset_filament(std::string filament_type);
  bool check_is_all_plates_selected();
  void        send_upload_file_cloud_event(wxString constr);

  
private:
  void OnPutEvent(Anycubic::Plugins::SDK::wxPluginEvent& event);

  private:
  // Anycubic::Plugins::Plugin
  const char *Name(void) override { return PLUGIN_NAME_STR; };
  bool Start(void) override { return true; };
  bool AttachEvt(class wxEvtHandler *) override;
  bool DetachEvt(class wxEvtHandler *) override;
  bool BindEvt(class wxPanel* panel, class wxWindow* parent = nullptr,
                   class wxString *bmp = nullptr) override;
  bool CreateWebview(class wxWebView *view, class wxWindow *parent = nullptr,
                     class wxString *bmp = nullptr) override;
  void Destroy(void) override;

private:
  Anycubic::Plugins::PluginHost *host_;
  std::string download_path_;
  std::string access_token_;

  std::vector<wxEvtHandler*> m_evt_list;
  std::mutex                 mtx_;
};