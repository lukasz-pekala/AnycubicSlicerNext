#include "info_manage_plugin.hpp"
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

InfoManage::InfoManage(Anycubic::Plugins::PluginHost *host)
    : host_(host) {
  assert(host_ != nullptr);
  auto router = host_->Router();
  assert(router != nullptr);
  

  router->REGISTER_FUNCATION(InfoManage, get_default_gcode_file_name);
  router->REGISTER_FUNCATION(InfoManage, get_curr_plate_printer_model_name);
  router->REGISTER_FUNCATION(InfoManage, autoExport_gcode3mf);
  router->REGISTER_FUNCATION(InfoManage, get_curr_plate_Index);
  router->REGISTER_FUNCATION(InfoManage, get_preset_filament);
  router->REGISTER_FUNCATION(InfoManage, check_is_all_plates_selected);
  router->REGISTER_FUNCATION(InfoManage, send_upload_file_cloud_event);
}

InfoManage::~InfoManage() {}


wxString GetFileNameFromUrl(const wxString& url)
{
    size_t pos = url.find_last_of('/');
    if (pos != wxString::npos) {
        return url.Mid(pos + 1);
    }
    return wxEmptyString;
}

void InfoManage::send_upload_file_cloud_event(wxString constr) 
{
    wxBookCtrlEvent evt_select(wxEVT_BOOKCTRL_PAGE_CHANGED, Slic3r::GUI::wxGetApp().mainframe->m_tabpanel->GetId());
    Slic3r::GUI::wxGetApp().mainframe->m_tabpanel->SetSelection(3);
    evt_select.SetSelection(3);
    evt_select.SetString(constr);
    evt_select.SetInt(3);
    wxPostEvent(Slic3r::GUI::wxGetApp().mainframe->m_tabpanel, evt_select);
}

bool InfoManage::check_is_all_plates_selected() 
{

    return Slic3r::GUI::wxGetApp().plater()->get_preview_canvas3D()->is_all_plates_selected();

}

wxString InfoManage::get_preset_filament(std::string filament_type) 
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

int InfoManage::get_curr_plate_Index() 
{ 
    return Slic3r::GUI::wxGetApp().plater()->get_partplate_list().get_curr_plate()->get_index();

}

std::string InfoManage::autoExport_gcode3mf(const std::string& fileIndex, bool export_all) 
{

    return Slic3r::GUI::wxGetApp().plater()->autoExport_gcode3mf(fileIndex, export_all);

}

std::string InfoManage::get_curr_plate_printer_model_name() 
{
    const Slic3r::PrintConfig& m_config = Slic3r::GUI::wxGetApp().plater()->get_partplate_list().get_curr_plate()->fff_print()->config();
    std::string printer_model = m_config.printer_model.value;
    return printer_model;
}

wxString InfoManage::get_default_gcode_file_name() 
{

    return Slic3r::GUI::wxGetApp().plater()->GetdefaultGcodeFileNmae();

}


void InfoManage::OnPutEvent(Anycubic::Plugins::SDK::wxPluginEvent& event)
{
    wxPropagationDisabler       disablePropagation(event);
    std::lock_guard<std::mutex> lock(mtx_);
    for (auto& e : m_evt_list) {
        e->ProcessEvent(event);
    }
}

bool InfoManage::AttachEvt(wxEvtHandler* evt)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (std::find(m_evt_list.begin(), m_evt_list.end(), evt) != m_evt_list.end())
        return false;
    m_evt_list.push_back(evt);
    return true;
}

bool InfoManage::DetachEvt(wxEvtHandler* evt)
{
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = std::find(m_evt_list.begin(), m_evt_list.end(), evt);
    if (it == m_evt_list.end())
        return false;
    m_evt_list.erase(it, m_evt_list.end());

    return true;
}

bool InfoManage::BindEvt(wxPanel* panel, wxWindow* parent,
                                wxString *bmp) {
  return false;
}

bool InfoManage::CreateWebview(wxWebView *view, wxWindow *parent,
                                  wxString *bmp) {
  return false;
}

void InfoManage::Destroy(void) { delete this; }
