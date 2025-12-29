#include "utility_plugin.hpp"
#include "slic3r/Anycubic/detail/anonymous.hpp"

#include <libslic3r/AppConfig.hpp>
#include <libslic3r/PrintConfig.hpp>
#include <slic3r/GUI/GUI_App.hpp>
#include <slic3r/GUI/Plater.hpp>
#include <slic3r/GUI/MainFrame.hpp>
#include <slic3r/GUI/Notebook.hpp>
#include <plugins_base/funcation.hxx>
#include <string>

#include <wx/filename.h>
#include <plugins_sdk/event/plugin_event.hxx>
#include <plugins_sdk/event/detail/plugin_custom_event.hxx>

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
  router->REGISTER_FUNCATION(UtilityPlugin, get_slic3r_version);
  router->REGISTER_FUNCATION(UtilityPlugin, get_app_version);
  router->REGISTER_FUNCATION(UtilityPlugin, get_app_version_code);


  router->REGISTER_FUNCATION(UtilityPlugin, get_default_gcode_file_name);
  router->REGISTER_FUNCATION(UtilityPlugin, get_curr_plate_printer_model_name);
  router->REGISTER_FUNCATION(UtilityPlugin, autoExport_gcode3mf);
  router->REGISTER_FUNCATION(UtilityPlugin, get_curr_plate_Index);
  router->REGISTER_FUNCATION(UtilityPlugin, get_preset_filament);
  router->REGISTER_FUNCATION(UtilityPlugin, check_is_all_plates_selected);
  router->REGISTER_FUNCATION(UtilityPlugin, send_upload_file_cloud_event);
  router->REGISTER_FUNCATION(UtilityPlugin, http_get);
}

UtilityPlugin::~UtilityPlugin() {}


wxString GetFileNameFromUrl(const wxString& url)
{
    size_t pos = url.find_last_of('/');
    if (pos != wxString::npos) {
        return url.Mid(pos + 1);
    }
    return wxEmptyString;
}

void UtilityPlugin::http_get(wxString url) 
{

    Slic3r::Http http = Slic3r::Http::get(url.ToStdString());
    http.timeout_max(60 * 5);
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << ", DownParamEvent = " + url;

    http.on_progress([this](Slic3r::Http::Progress p, bool& cancel_http) {

        })
        .on_complete([this, url](std::string body, unsigned size) {
            BOOST_LOG_TRIVIAL(warning)<<__FUNCTION__ << "http.on_complete";

            try {
                auto savePath = wxStandardPaths::Get().GetTempDir();

                std::string fileName = Slic3r::GUI::format(GetFileNameFromUrl(url));
                if (fileName.empty())
                    fileName = "paramInfo.zip";

                std::string m_downloadFileName = Slic3r::GUI::format(wxString::Format("%s%s%s", savePath, wxString(wxFileName::GetPathSeparator()),
                                                                          Slic3r::GUI::format_wxstr(fileName.data())));

                fs::fstream file(m_downloadFileName, std::ios::out | std::ios::binary | std::ios::trunc);
                if (!file.is_open()) {
                    throw std::runtime_error("Failed to open file for writing: " + m_downloadFileName);
                }
                file.write(body.c_str(), body.size());
                if (!file) {
                    throw std::runtime_error("Failed to write to file: " + m_downloadFileName);
                }
                BOOST_LOG_TRIVIAL(warning)<<__FUNCTION__ << " Downloaded file: " << m_downloadFileName;

                Anycubic::Plugins::SDK::wxPluginEvent evt_1(EVT_POST_SHOW_NEW_VERSIONPAR_EVENT);
                evt_1.SetString(Slic3r::GUI::format_wxstr(m_downloadFileName));
                OnCloudMqttEvent(evt_1);
            } catch (const std::exception& e) {
                BOOST_LOG_TRIVIAL(error)<<__FUNCTION__ << ", Exception: " << e.what();
            }
        })
        .on_error([&](std::string body, std::string error, unsigned status) {
            BOOST_LOG_TRIVIAL(error)<<__FUNCTION__ << boost::format(", status=%1%, error=%2%, body=%3%") % status % error % body;
        })
        .perform();

}

void UtilityPlugin::send_upload_file_cloud_event(wxString constr) 
{
    wxBookCtrlEvent evt_select(wxEVT_BOOKCTRL_PAGE_CHANGED, Slic3r::GUI::wxGetApp().mainframe->m_tabpanel->GetId());
    Slic3r::GUI::wxGetApp().mainframe->m_tabpanel->SetSelection(3);
    evt_select.SetSelection(3);
    evt_select.SetString(constr);
    evt_select.SetInt(3);
    wxPostEvent(Slic3r::GUI::wxGetApp().mainframe->m_tabpanel, evt_select);
}

bool UtilityPlugin::check_is_all_plates_selected() 
{

    return Slic3r::GUI::wxGetApp().plater()->get_preview_canvas3D()->is_all_plates_selected();

}

wxString UtilityPlugin::get_preset_filament(std::string filament_type) 
{
    std::string itemName = Slic3r::Preset::remove_suffix_modified(filament_type);

    Slic3r::PresetCollection& filaments = Slic3r::GUI::wxGetApp().preset_bundle->filaments;
    const Slic3r::Preset*       preset    = filaments.find_preset(itemName, false);
    if (preset == nullptr) {
        return Slic3r::GUI::from_u8(itemName);
    } else {
        preset->config.opt_string("filament_type");
        const Slic3r::ConfigOptionStrings* filament_type = preset->config.option<Slic3r::ConfigOptionStrings>("filament_type");
        if (filament_type != nullptr) {
            std::string filament_types = filament_type->values[0];
            if (filament_types.length() > 0) {
                return Slic3r::GUI::from_u8(filament_types);
            } else {
                return Slic3r::GUI::from_u8(itemName);
            }
        } else {
            return Slic3r::GUI::from_u8(itemName);
        }
    }

    return Slic3r::GUI::from_u8(itemName);


}

int UtilityPlugin::get_curr_plate_Index() 
{ 
    return Slic3r::GUI::wxGetApp().plater()->get_partplate_list().get_curr_plate()->get_index();

}

std::string UtilityPlugin::autoExport_gcode3mf(const std::string& fileIndex, bool export_all) 
{

    return Slic3r::GUI::wxGetApp().plater()->autoExport_gcode3mf(fileIndex, export_all);

}

std::string UtilityPlugin::get_curr_plate_printer_model_name() 
{
    const Slic3r::PrintConfig& m_config = Slic3r::GUI::wxGetApp().plater()->get_partplate_list().get_curr_plate()->fff_print()->config();
    std::string printer_model = m_config.printer_model.value;
    return printer_model;
}

wxString UtilityPlugin::get_default_gcode_file_name() 
{

    return Slic3r::GUI::wxGetApp().plater()->GetdefaultGcodeFileNmae();

}

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
  return region() == "china";
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
  host_->GetValue("user/auto_login", auto_login);
  auto_login.Lower();
  return auto_login == "true" || auto_login == "1" || auto_login == "on" ||
         auto_login == "yes" || auto_login == "y" || auto_login == "t";
}
void UtilityPlugin::set_auto_login(bool auto_login) {
  host_->SetValue("user/auto_login", auto_login ? "true" : "false");
}
bool UtilityPlugin::get_login_token(wxString *token) {
  return host_->GetEncryptValue("user/login_token", *token);
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

std::string UtilityPlugin::get_slic3r_version(void) const { return SLIC3R_VERSION; }
std::string UtilityPlugin::get_app_version(void) const { return SoftFever_VERSION; }
int32_t UtilityPlugin::get_app_version_code(void) const { return VERSION_CODE; }

bool UtilityPlugin::AttachEvt(wxEvtHandler *) { return false; }
void UtilityPlugin::OnCloudMqttEvent(Anycubic::Plugins::SDK::wxPluginEvent& event)
{
    wxPropagationDisabler       disablePropagation(event);
    std::lock_guard<std::mutex> lock(mtx_);
    for (auto& e : m_evt_list) {
        e->ProcessEvent(event);
    }
}

bool UtilityPlugin::AttachEvt(wxEvtHandler* evt)
{
    std::lock_guard<std::mutex> lock(mtx_);
    assert(wxIsMainThread() && std::ranges::none_of(m_evt_list, [evt](auto& e) { return e == evt; }));
    m_evt_list.push_back(evt);
    return true;
}

bool UtilityPlugin::DetachEvt(wxEvtHandler* evt)
{
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = std::remove(m_evt_list.begin(), m_evt_list.end(), evt);
    if (it == m_evt_list.end())
        return false;
    m_evt_list.erase(it, m_evt_list.end());

    return true;
}

bool UtilityPlugin::BindEvt(wxPanel* panel, wxWindow* parent,
                                wxString *bmp) {
  return false;
}

bool UtilityPlugin::CreateWebview(wxWebView *view, wxWindow *parent,
                                  wxString *bmp) {
  return false;
}

void UtilityPlugin::Destroy(void) { delete this; }
