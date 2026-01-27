#include "info_manage_plugin.hpp"
#include "slic3r/Anycubic/detail/anonymous.hpp"
#include <libslic3r/AppConfig.hpp>
#include <libslic3r/PrintConfig.hpp>
#include <slic3r/GUI/GUI_App.hpp>
#include <slic3r/GUI/GUI.hpp>
#include <slic3r/GUI/Plater.hpp>
#include <slic3r/GUI/MainFrame.hpp>
#include <slic3r/GUI/Notebook.hpp>
#include <libslic3r/Semver.hpp>
#include <string>

#include <wx/filename.h>
#include <plugins_sdk/event/plugin_event.hxx>
#include <plugins_sdk/event/detail/plugin_custom_event.hxx>
#include <plugins_sdk/event/detail/util_tool.hxx>
#include <slic3r/Utils/PresetUpdater.hpp>

#include <cmath>
#include <nlohmann/json.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define sqr(x) ((x) * (x))

#define AUTO_DEVICEID "test_01"

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
  router->REGISTER_FUNCATION(InfoManage, getAcCfg);
  router->REGISTER_FUNCATION(InfoManage, persetUpdaterOperate);
  router->REGISTER_FUNCATION(InfoManage, IsAutoRunModel);
  router->REGISTER_FUNCATION(InfoManage, IsRunCallTestModel);


  router->REGISTER_FUNCATION(InfoManage, GetModelSlicerInfoMap);
  router->REGISTER_FUNCATION(InfoManage, GetModelSlicerInfo);

  router->REGISTER_FUNCATION(InfoManage, GetFilamentSyncColourList);
  router->REGISTER_FUNCATION(InfoManage, GetFilamentSyncTypeList);

  router->REGISTER_FUNCATION(InfoManage, Download);
  router->REGISTER_FUNCATION(InfoManage, UploadFile);
  router->REGISTER_FUNCATION(InfoManage, GetLastLoadGcode);
  router->REGISTER_FUNCATION(InfoManage, CreateSideToolBtn);

  router->REGISTER_FUNCATION(InfoManage, PushLog);
}

InfoManage::~InfoManage() {}

void InfoManage::PushLog(const std::string& content, int logLevel) 
{
    // logLevel: 0:trace 1:info   2:debug 3:warning 4:error

    switch (logLevel) {
    case 4: {
        BOOST_LOG_TRIVIAL(error) << content;
        break;
    }
    case 3: {
        BOOST_LOG_TRIVIAL(warning) << content;
        break;
    } 
    case 2: {
        BOOST_LOG_TRIVIAL(debug) << content;
        break;
    }
    case 1: {
        BOOST_LOG_TRIVIAL(info) << content;
        break;
    }
    default: {
        BOOST_LOG_TRIVIAL(trace) << content;
        break;
    }
    }
}

wxString InfoManage::GetLastLoadGcode() 
{
    return Slic3r::GUI::wxGetApp().plater()->GetLastGcodeFileName();
}

void InfoManage::Download(const std::string& url, const TransferCallback* cb)
{
    auto http = Slic3r::Http::get(url);

    http.on_progress([cb](Slic3r::Http::Progress p, bool& cancel) {
            if (!cb || !cb->onProgress)
                return;

            TransferProgress tp;
            tp.total = p.ultotal;
            tp.now   = p.ulnow;
            cb->onProgress(tp, cancel);
        })
        .on_complete([cb](std::string body, unsigned status) {
            if (cb && cb->onComplete)
                cb->onComplete(body, status);
        })
        .on_error([cb](std::string body, std::string error, unsigned status) {
            if (cb && cb->onError)
                cb->onError(body, error, status);
        });

    http.perform();
}

void InfoManage::UploadFile(
    const std::string& url, const std::string& filePath, const std::string& fileName, uint64_t fileSize, const TransferCallback* cb)

{
    auto http = Slic3r::Http::post(url);

    http.header("Content-Type", "multipart/form-data");
    http.header("X-File-Length", std::to_string(fileSize));

    http.form_add("filename", fileName).form_add_file("gcode", filePath, fileName).timeout_connect(120);

    http.on_progress([cb](Slic3r::Http::Progress p, bool& cancel) {
            if (!cb || !cb->onProgress)
                return;

            TransferProgress tp;
            tp.total = p.ultotal;
            tp.now   = p.ulnow;
            cb->onProgress(tp, cancel);
        })
        .on_complete([cb](std::string body, unsigned status) {
            if (cb && cb->onComplete)
                cb->onComplete(body, status);
        })
        .on_error([cb](std::string body, std::string error, unsigned status) {
            if (cb && cb->onError)
                cb->onError(body, error, status);
        });

    http.perform();


}


std::string InfoManage::GetFilamentSyncColourList() 
{ 
    std::vector<std::string> _filament_colourList;
    std::string              serialized = json(_filament_colourList).dump();
    return serialized;
}

std::string InfoManage::GetFilamentSyncTypeList() 
{ 
    std::vector<std::string> _filament_typeList;
    std::string              serialized = json(_filament_typeList).dump();
    return serialized;
}



void InfoManage::Auto_BindEvent()
{
    //Bind(EVT_AUTO_FILE_OPEN_EVENT, [this](wxPluginEvent& evt) {
    //    if (wxGetApp().plater()->canvas3D() != nullptr && m_autoStartIndex)
    //        wxGetApp().plater()->canvas3D()->Auto_Importmodel();
    //});
    //Bind(EVT_AUTO_FILE_OPEN_FINISH_EVENT, [this](wxPluginEvent& evt) {
    //    if (m_autoStartIndex)
    //        wxGetApp().mainframe->Auto_SlicerEvent();
    //});
    //Bind(EVT_AUTO_SLICER_FINISH_EVENT, [this](wxPluginEvent& evt) {
    //    if (m_autoStartIndex)
    //        wxGetApp().mainframe->Auto_SlicerEvent(1);
    //});
    //Bind(EVT_AUTO_REMOTEPRINTSHOW_FINISH_EVENT, [this](wxPluginEvent& evt) {
    //    if (m_showWindowType == 1 && m_autoStartIndex) {
    //        wxPluginEvent evt_auto(EVT_AUTO_CLICK_EVENT);
    //        OnPutEvent(evt_auto);
    //    }
    //});
    //Bind(EVT_AUTO_SLOOP_FINISH_EVENT, [this](wxPluginEvent& evt) {
    //    if (!m_autoStartIndex)
    //        return;
    //    wxGetApp().plater()->new_project();
    //    Auto_openFileName();
    //    if (m_auto_nowFileName.size() > 0) {
    //        wxPluginEvent auto_evt(EVT_AUTO_FILE_OPEN_EVENT);
    //        // wxPostEvent(this, auto_evt);
    //        OnPutEvent(auto_evt);
    //    } else {
    //        m_autoStartIndex = false;
    //        wxGetApp().mainframe->AutoSetClickEnable();
    //    }
    //});
    //Bind(EVT_AUTO_SLICER_FAIL_EVENT, [this](wxPluginEvent& evt) {
    //    int i = 0;
    //});
}



bool InfoManage::IsRunCallTestModel(int index)
{
    bool isRunCall = IsAutoRunModel();
    if (!isRunCall)
        return isRunCall;

    switch (index) {
    case RunCallTestModel::r_upload: {
        wxPluginEvent evt_progress(EVT_REMOTE_PRINTER_UPLOAD_PROGRESS);
        evt_progress.SetInt(100);
        OnPutEvent(evt_progress);

        break;
    }
    case RunCallTestModel::r_print: {
        wxPluginEvent   evt(EVT_ACCLOUD_PRINTER_REMOTE);
        RemotePrintObj* obj = new RemotePrintObj();
        obj->succeed        = true;
        obj->result_string  = "";
        obj->printer_id     = Slic3r::GUI::from_u8(getLastRemoteDeviceID());
        obj->task_id        = "";

        updateLastTaskID(Slic3r::GUI::into_u8(obj->task_id));
        evt.SetSharedData(obj, nullptr, [](void*, void* a) { delete (RemotePrintObj*) a; });
        OnPutEvent(evt);

        break;
    }
    case RunCallTestModel::r_calculatePrinter: {
        m_auto_selectDeviceID = AUTO_DEVICEID;

        break;
    }

    default: break;
    }

    return isRunCall;
}

bool InfoManage::IsAutoRunModel()
{
    const std::string value     = Slic3r::GUI::wxGetApp().app_config->get("anycubic_test_mode");
    bool              isAutoRun = (value == "1" || value == "2" || value == "true");
    return isAutoRun;
}

void InfoManage::AutoSetDirPath(const wxString& dir)
{
    m_auto_dirPath = dir;
    Auto_GetFiles();
}

void InfoManage::Auto_GetFiles()
{
    m_autoStartIndex = true;
    m_auto_filesList.clear();

    if (wxDirExists(m_auto_dirPath)) {
        m_auto_filesList = GetDirListFiles(m_auto_dirPath);
        Auto_openFileName();

        wxPluginEvent auto_evt(EVT_AUTO_FILE_OPEN_EVENT);
        OnPutEvent(auto_evt);
    }
}

wxVector<wxString> InfoManage::GetDirListFiles(const wxString& dirPath)
{
    wxVector<wxString> filesList;

    if (!wxDirExists(dirPath)) {
        std::cout << "Directory does not exist." << std::endl;
        return filesList;
    }

    wxDir dir(dirPath);
    if (!dir.IsOpened()) {
        std::cout << "Failed to open directory." << std::endl;
        return filesList;
    }

    wxString filename;
    bool     cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
    while (cont) {
        wxFileName fullPath(dirPath, filename);
        filesList.push_back(fullPath.GetFullPath().wc_str());
        cont = dir.GetNext(&filename);
    }

    return filesList;
}
wxString InfoManage::Auto_GetImportFileName()
{
    return m_auto_nowFileName;
}

void InfoManage::Auto_openFileName()
{
    bool firstInit = false;
    if (m_auto_nowFileName.empty() && m_auto_filesList.size() > 0) {
        m_auto_nowFileName = m_auto_filesList[0];
        firstInit          = true;
    }
    if (!firstInit) {
        int index = FindStringInVector(m_auto_nowFileName);
        if (index != -1) {
            m_auto_nowFileName = m_auto_filesList[index + 1];
        } else {
            m_auto_nowFileName = wxEmptyString;
        }
    }
}

int InfoManage::FindStringInVector(const wxString& target)
{
    auto it = std::find(m_auto_filesList.begin(), m_auto_filesList.end(), target);
    if (it != m_auto_filesList.end() && it != m_auto_filesList.end() - 1) {
        return std::distance(m_auto_filesList.begin(), it);
    }
    return -1;
}


Slic3r::Semver get_version_from_json(std::string file_path)
{
    try {
        boost::nowide::ifstream ifs(file_path);
        if (!ifs.is_open()) {
            return Slic3r::Semver();
        }
        json j;
        ifs >> j;
        std::string version_str = j.at(BBL_JSON_KEY_VERSION);

        auto config_version = Slic3r::Semver::parse(version_str);
        if (!config_version) {
            return Slic3r::Semver();
        } else {
            return *config_version;
        }
    } catch (nlohmann::detail::parse_error& err) {
        BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << ": parse " << file_path
                                 << " got a nlohmann::detail::parse_error, reason = " << err.what();
        return Slic3r::Semver();
    }
}

std::string InfoManage::GetModelSlicerInfo(bool isPrint, const wxString& last_load_gcode)
{
    ModelSlicerInfo info;

    Slic3r::GCodeProcessorResult* gcodeResult        = wxGetApp().plater()->get_partplate_list().get_curr_plate()->get_slice_result();
    std::vector<float>            filament_diameters = gcodeResult->filament_diameters;

    std::vector<float>  filament_densities = gcodeResult->filament_densities;
    wxString            modesLayersNum = wxString::Format("%d", gcodeResult->print_statistics.modes[0].layers_times.size()) + _(" Layers");
    wxString            timeStr        = Slic3r::get_time_dhms(gcodeResult->print_statistics.modes[0].time);
    std::vector<double> wipe_tower_used_filaments_m;
    std::vector<double> wipe_tower_used_filaments_g;
    double              total_wipe_tower_used_filament_m = 0, total_wipe_tower_used_filament_g = 0;

    auto get_used_filament_from_volume = [this, &filament_diameters, &filament_densities](double volume, int extruder_id) {
        double koef = 0.001;
        double PI   = 3.141592653589793238;

        std::pair<double, double> ret = {koef * volume /
                                             (PI * ((0.5 * filament_diameters[extruder_id]) * (0.5 * filament_diameters[extruder_id]))),
                                         volume * filament_densities[extruder_id] * 0.001};
        return ret;
    };

    for (auto volume : gcodeResult->print_statistics.total_volumes_per_extruder) {
        auto [model_used_filament_m, model_used_filament_g] = get_used_filament_from_volume(volume.second, volume.first);
        wipe_tower_used_filaments_m.push_back(model_used_filament_m);
        wipe_tower_used_filaments_g.push_back(model_used_filament_g);
        total_wipe_tower_used_filament_m += model_used_filament_m;
        total_wipe_tower_used_filament_g += model_used_filament_g;
    }

    wxString filamentWeight = wxString::Format("%.1fg", total_wipe_tower_used_filament_g);
    wxString filamentLength = wxString::Format("%.2f%s", total_wipe_tower_used_filament_m, "m");

    std::vector<std::string> base64Img = wxGetApp().plater()->get_partplate_list().get_curr_plate()->fff_print()->print_statistics().thumbnails;
    info.imgBase64 = !base64Img.empty() ? base64Img[0] : GetGcodeFileImg(last_load_gcode);

    info.print_time      = timeStr;
    info.filament_length = filamentLength;
    info.used_filament   = filamentWeight;

    info.modleLayers = wxString::Format("%d", gcodeResult->print_statistics.modes[0].layers_times.size()) + _(" Layers");
    std::vector<unsigned int> printing_extruders = wxGetApp().plater()->get_partplate_list().get_curr_plate()->fff_print()->print_statistics().printing_extruders;
    for (int i = 0; i < printing_extruders.size(); i++) {
        info.extruder_idsList.push_back(printing_extruders[i]);
    }

    wxVector<wxString> filament_colourList;
    wxVector<wxString> filament_typeList;
    for (int i = 0; i < wxGetApp().plater()->get_partplate_list().get_curr_plate()->fff_print()->config().filament_colour.values.size();
         i++) {
        filament_colourList.push_back(
            Slic3r::GUI::from_u8(wxGetApp().plater()->get_partplate_list().get_curr_plate()->fff_print()->config().filament_colour.get_at(i)));
    }
    for (int i = 0; i < wxGetApp().plater()->get_partplate_list().get_curr_plate()->fff_print()->config().filament_type.values.size(); i++)
    { filament_typeList.push_back(Slic3r::GUI::from_u8(
            wxGetApp().plater()->get_partplate_list().get_curr_plate()->fff_print()->config().filament_type.get_at(i)));
    }

    info.filamentColors_slicer = filament_colourList;

    info.filamentTypess_slicer = filament_typeList;

    std::string printerName = wxGetApp().plater()->get_partplate_list().get_curr_plate()->fff_print()->print_statistics().printer_model;
    bool        isGcodeIndex = false;
    if (printerName.empty()) {
        std::vector<unsigned int> extruders;// = gcodeResult->print_statistics.getPrinting_extruders();
        std::vector<std::string>  extruder_colors = gcodeResult->extruder_colors;
        std::vector<std::string>  filament_types;//= gcodeResult->filament_types;
        info.extruder_idsList.clear();
        for (int i = 0; i < extruders.size(); i++) {
            info.extruder_idsList.push_back(extruders[i]);
        }

        info.extruder_colors_gcode.clear();
        for (int i = 0; i < extruder_colors.size(); i++) {
            info.extruder_colors_gcode.push_back(extruder_colors[i]);
        }

        info.filament_types_gcode.clear();
        for (int i = 0; i < filament_types.size(); i++) {
            info.filament_types_gcode.push_back(filament_types[i]);
        }

        printerName;// = gcodeResult->printer_model;
        info.isGcode = true;
    }
    info.printerName  = Slic3r::GUI::from_u8(printerName);
    info.machine_type = Anycubic::Plugins::dispatch_call<int>(host_, "remoteManger", "FromStrGetPrinterType", printerName);
    if (!isPrint) {
        /*SetBatchResult({static_cast<int>(gcodeResult->print_statistics.modes[0].time),
                        static_cast<int>(gcodeResult->print_statistics.modes[0].layers_times.size()), total_wipe_tower_used_filament_g,
                        timeStr});*/
    }

    return Anycubic::Plugins::to_json_string(info);
}



std::string InfoManage::GetModelSlicerInfoMap(bool isPrint, const wxString& last_load_gcode)
{
    std::map<int, ModelSlicerInfo> modelSelcicerInfoMap;
    for (const auto& partObj : wxGetApp().plater()->get_partplate_list().get_plate_list()) {
        partObj->get_index();
        Slic3r::GCodeProcessorResult* gcodeResult = partObj->get_slice_result();

        ModelSlicerInfo info;

        std::vector<float> filament_diameters = gcodeResult->filament_diameters;

        std::vector<float> filament_densities = gcodeResult->filament_densities;
        wxString modesLayersNum = wxString::Format("%d", gcodeResult->print_statistics.modes[0].layers_times.size()) + _(" Layers");
        wxString timeStr        = Slic3r::get_time_dhms(gcodeResult->print_statistics.modes[0].time);
        std::vector<double> wipe_tower_used_filaments_m;
        std::vector<double> wipe_tower_used_filaments_g;
        double              total_wipe_tower_used_filament_m = 0, total_wipe_tower_used_filament_g = 0;

        auto get_used_filament_from_volume = [this, &filament_diameters, &filament_densities](double volume, int extruder_id) {
            double                    koef = 0.001;
            std::pair<double, double> ret  = {koef * volume / (M_PI * sqr(0.5 * filament_diameters[extruder_id])),
                                             volume * filament_densities[extruder_id] * 0.001};
            return ret;
        };
        std::map<int, wxString> extruderWeightMap;
        for (auto volume : gcodeResult->print_statistics.total_volumes_per_extruder) {
            auto [model_used_filament_m, model_used_filament_g] = get_used_filament_from_volume(volume.second, volume.first);
            wipe_tower_used_filaments_m.push_back(model_used_filament_m);
            wipe_tower_used_filaments_g.push_back(model_used_filament_g);
            total_wipe_tower_used_filament_m += model_used_filament_m;
            total_wipe_tower_used_filament_g += model_used_filament_g;
            extruderWeightMap[volume.first] = wxString::Format("%.2fg", model_used_filament_g);
        }

        wxString filamentWeight = wxString::Format("%.1fg", total_wipe_tower_used_filament_g);
        wxString filamentLength = wxString::Format("%.2f%s", total_wipe_tower_used_filament_m, "m");

        std::vector<std::string> base64Img;//= partObj->fff_print()->print_statistics().thumbnails;
        info.imgBase64 = !base64Img.empty() ? base64Img[0] : GetGcodeFileImg(last_load_gcode);

        info.print_time       = timeStr;
        info.filament_length  = filamentLength;
        info.used_filament    = filamentWeight;
        info.filamentWeight_d = total_wipe_tower_used_filament_g;
        info.print_time_f     = gcodeResult->print_statistics.modes[0].time;

        info.modleLayers = wxString::Format("%d", gcodeResult->print_statistics.modes[0].layers_times.size()) + _(" Layers");

        std::vector<unsigned int> printing_extruders;// = partObj->fff_print()->print_statistics().printing_extruders;
        for (int i = 0; i < printing_extruders.size(); i++) {
            info.extruder_idsList.push_back(printing_extruders[i]);
        }

        wxVector<wxString> filament_colourList;
        wxVector<wxString> filament_typeList;
        for (int i = 0; i < partObj->fff_print()->config().filament_colour.values.size(); i++) {
            filament_colourList.push_back(Slic3r::GUI::from_u8(partObj->fff_print()->config().filament_colour.get_at(i)));
        }
        for (int i = 0; i < partObj->fff_print()->config().filament_type.values.size(); i++) {
            filament_typeList.push_back(Slic3r::GUI::from_u8(partObj->fff_print()->config().filament_type.get_at(i)));
        }

        info.filamentColors_slicer = filament_colourList;

        info.filamentTypess_slicer = filament_typeList;

        std::string printerName;// = partObj->fff_print()->print_statistics().printer_model;
        bool                     isGcodeIndex = false;
        std::vector<std::string> filament_types;
        std::vector<std::string> extruder_colors;
        if (printerName.empty()) {
            std::vector<unsigned int> extruders;//= gcodeResult->print_statistics.getPrinting_extruders();
            std::vector<std::string>  extruder_colors = gcodeResult->extruder_colors;
            std::vector<std::string>  filament_types;// = gcodeResult->filament_types;
            info.extruder_idsList.clear();
            for (int i = 0; i < extruders.size(); i++) {
                info.extruder_idsList.push_back(extruders[i]);
            }

            info.extruder_colors_gcode.clear();
            for (int i = 0; i < extruder_colors.size(); i++) {
                info.extruder_colors_gcode.push_back(extruder_colors[i]);
            }

            info.filament_types_gcode.clear();
            for (int i = 0; i < filament_types.size(); i++) {
                info.filament_types_gcode.push_back(filament_types[i]);
            }

            printerName;// = gcodeResult->printer_model;
            info.isGcode = true;
        }

        wxVector<FilamentInfoObj>    colorFilamentobjsList;
        wxVector<GcodeFilamentColor> gcodeFilamentColorList;

        if (info.isGcode) {
            for (int i = 0; i < extruder_colors.size(); i++) {
                wxColour    numColor           = HexToWxColour(extruder_colors[i]);
                std::string select_preset_type = "-";

                if (filament_types.size() > i) {
                    select_preset_type = filament_types[i];
                }

                colorFilamentobjsList.push_back({numColor, getTextFitColor(numColor), select_preset_type});
            }

        } else {
            for (int i = 0; i < info.filamentColors_slicer.size(); i++) {
                wxColour    numColor           = HexToWxColour(info.filamentColors_slicer.at(i));
                std::string select_preset_type = "-";
                if (info.filamentTypess_slicer.size() > i) {
                    select_preset_type = Slic3r::GUI::into_u8(info.filamentTypess_slicer.at(i));
                }
                colorFilamentobjsList.push_back({numColor, getTextFitColor(numColor), select_preset_type});
            }
        }
        int sloopIndex = 0;
        for (size_t extruder_id : info.extruder_idsList) {
            if (colorFilamentobjsList.size() > extruder_id) {
                GcodeFilamentColor colorFilamentobjs;
                colorFilamentobjs.num   = extruder_id;
                colorFilamentobjs.sloop = sloopIndex;
                if (extruderWeightMap.find(extruder_id) != extruderWeightMap.end()) {
                    colorFilamentobjs.weight = extruderWeightMap[extruder_id];
                }
                if (isGcodeIndex) {
                    if (extruder_colors.size() > extruder_id) {
                        colorFilamentobjs.filamentColor = HexToWxColour(extruder_colors[extruder_id]);
                    }
                } else {
                    if (info.filamentColors_slicer.size() > extruder_id) {
                        colorFilamentobjs.filamentColor = HexToWxColour(info.filamentColors_slicer.at(extruder_id));
                    }
                }
                if ((isGcodeIndex ? filament_types.size() : info.filamentTypess_slicer.size()) > extruder_id) {
                    colorFilamentobjs.filament = isGcodeIndex ? filament_types[extruder_id] : info.filamentTypess_slicer.at(extruder_id);
                } else {
                    colorFilamentobjs.filament = get_preset_filament(Slic3r::GUI::into_u8(colorFilamentobjsList[extruder_id].filament_type));
                }
                gcodeFilamentColorList.push_back(colorFilamentobjs);
                sloopIndex++;
            }
        }

        info.gcodeFilamentColorList = gcodeFilamentColorList;

        info.printerName                           = printerName;
        info.machine_type;// = FromStrGetPrinterType(printerName);
        modelSelcicerInfoMap[partObj->get_index()] = info;
    }
    
    
    json jmap = json::object();

    for (const auto& [key, info] : modelSelcicerInfoMap) {
        json jinfo;
        Anycubic::Plugins::to_json(jinfo, info);
        jmap[std::to_string(key)] = jinfo;
    }

    std::string jsonStr = jmap.dump(2); 
    
    
    return jsonStr;


}


std::string InfoManage::LoadGcodeFileInfo(const std::string& filename)
{
    std::string   line;
    std::ifstream file;

#ifdef _WIN32
    std::filesystem::path p = wxString::FromUTF8(filename).wc_str();
#else
    std::filesystem::path p = filename;
#endif
    bool        capture     = false;
    bool        infoCapture = false;
    std::string capturedLines;

    if (file.is_open()) {
        while (getline(file, line)) {
            if (line.find("thumbnail begin") != std::string::npos) {
                capture = true;
                continue;
            }
            if (line.find("thumbnail end") != std::string::npos) {
                capture = false;
                break;
            }
            if (capture) {
                std::stringstream ss(line);
                char              semicolon;
                std::string       content;
                ss >> semicolon;
                getline(ss, content);
                auto it = std::find_if(content.begin(), content.end(), [](unsigned char ch) { return !std::isspace(ch); });
                content.erase(content.begin(), it);
                capturedLines += content;
            }
        }
        file.close();
    } else {
        std::cerr << "Unable to open file";
        return capturedLines;
    }

    return capturedLines;
}



std::string InfoManage::GetGcodeFileImg(wxString last_load_gcode)
{
    std::string localFielName;
    wxString    nowFileName = !last_load_gcode.empty() ? last_load_gcode : wxString();
    if (nowFileName.length() == 0) {
        nowFileName = get_default_gcode_file_name();
        if (nowFileName == "emptyFile") {
            //showACCloudOpterStaticDialog(false, _("Upload to cloud failed"),_("G-code export failed. Please check your model and print settings"));

            //SetUpLoadFileIndexEvent(false);
            return localFielName;
        }
        localFielName = Slic3r::GUI::into_u8(nowFileName);
    } else {
        localFielName = Slic3r::GUI::into_u8(nowFileName);
    }
    std::string imgStr = LoadGcodeFileInfo(localFielName);
    return imgStr;
}



bool InfoManage::persetUpdaterOperate(const std::string& cmd, wxString profiles_new_dir, std::string* error_reason) 
{
    boost::filesystem::path paramDir(profiles_new_dir.ToUTF8().data());
    return persetUpdaterOperate_(cmd, paramDir, error_reason);

}
bool InfoManage::persetUpdaterOperate_(const std::string& cmd, boost::filesystem::path profiles_new_dir, std::string* error_reason)
{
    static boost::filesystem::path pereset_dir_sys = boost::filesystem::path(Slic3r::resources_dir()) / "profiles";    
    static boost::filesystem::path pereset_dir_run = boost::filesystem::path(Slic3r::data_dir()) / "system";         
    static boost::filesystem::path pereset_dir_ota = boost::filesystem::path(Slic3r::data_dir()) / "ota" / "profiles"; 
    Slic3r::Semver version_of_sys = get_version_from_json((pereset_dir_sys / "Anycubic.json").make_preferred().string());
    Slic3r::Semver version_of_run = get_version_from_json((pereset_dir_run / "Anycubic.json").make_preferred().string());
    Slic3r::Semver version_of_ota = get_version_from_json((pereset_dir_ota / "Anycubic.json").make_preferred().string());

    static auto testPersetFolder = [](boost::filesystem::path profiles_dir) -> bool {
        if (!get_version_from_json((profiles_dir / "Anycubic.json").make_preferred().string()).valid()) {
            return false;
        }
        return true;
    };
    static auto doPersetFolderCut = [](boost::filesystem::path source, boost::filesystem::path target) -> bool {
        boost::system::error_code ec;
        if (source == target) {
            return true;
        }
        if (boost::filesystem::exists(target)) {
            boost::filesystem::remove_all(target, ec);
            if (ec) {
                BOOST_LOG_TRIVIAL(error) << Slic3r::format("Error removing %1%  %2% ", target.string(), ec.message());
                return false;
            }
        }
        if (boost::filesystem::exists(source)) {
            boost::filesystem::rename(source, target, ec);
            if (ec) {
                BOOST_LOG_TRIVIAL(error) << Slic3r::format("Error renaming %1% ---> %2%  %3% ", source.string(), target.string(),
                                                          ec.message());
                return false;
            }
            return true;
        }
        return false;
    };
    static auto doPersetUpdateNow = []() -> bool {
        bool res = true;
        Slic3r::set_profiles_dir(pereset_dir_ota.string());
#ifdef ENABLE_OLD_VERSION_UPDATE
        Slic3r::GUI::wxGetApp().check_config_updates_from_updater();
#endif
        return res;
    };

    bool result = true;

    if (cmd == "verify") {
        wxString status;
        host_->GetEncryptValue("app/profiles_ota", status);
        if (status.empty()) {
            Slic3r::set_profiles_dir(pereset_dir_sys.string());
            BOOST_LOG_TRIVIAL(info) << "use system profiles : " << pereset_dir_sys.string();
            return true;
        }
        if (testPersetFolder(pereset_dir_ota) && version_of_ota > version_of_sys) {
            Slic3r::set_profiles_dir(pereset_dir_ota.string());

            wxString vale_ota = "ota";
            host_->SetEncryptValue("app/profiles_ota", vale_ota);
            BOOST_LOG_TRIVIAL(info) << "use ota profiles : " << pereset_dir_ota.string();
        } else {
            Slic3r::set_profiles_dir(pereset_dir_sys.string());
            wxString vale_0 = "0";
            host_->SetEncryptValue("app/profiles_ota", vale_0);
            BOOST_LOG_TRIVIAL(info) << "use system profiles : " << pereset_dir_sys.string();
        }
    } else if (cmd == "ready" || cmd == "install") {
        result = result && testPersetFolder(profiles_new_dir);
        result = result && doPersetFolderCut(profiles_new_dir, pereset_dir_ota);
        result = result && testPersetFolder(pereset_dir_ota);

        if (!result) {
            BOOST_LOG_TRIVIAL(error) << "Error in persetUpdaterOperate: " << cmd;
            return false; 
        }
        wxString vale_ready = "ready";
        host_->SetEncryptValue("app/profiles_ota", vale_ready);
        if (cmd == "install") {
            result = result && doPersetUpdateNow();
            if (!result) {
                Slic3r::set_profiles_dir(pereset_dir_sys.string());
                wxString vale_system = "system";
                host_->SetEncryptValue("app/profiles_ota", vale_system);
                BOOST_LOG_TRIVIAL(error) << "Error in install: " << cmd;
                return false;
            }
            BOOST_LOG_TRIVIAL(info) << "Successfully installed pereset_dir_ota.";
            wxString vale_done = "done";
            host_->SetEncryptValue("app/profiles_ota", vale_done);
        }
    }

    return result;
}

std::string InfoManage::getAcCfg(std::string key2)
{
    if ("preset_version_sys" == key2) {
        boost::filesystem::path p(Slic3r::resources_dir());
        p = p / "profiles" / "Anycubic.json";
        return get_version_from_json(p.string()).to_string();
    } else if ("preset_version_runtime" == key2) {
        return Slic3r::GUI::wxGetApp().preset_bundle->get_vendor_profile_version("Anycubic").to_string();
    }
    return std::string();
}

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

wxColour InfoManage::HexToWxColour(const wxString& hex)
{
    long red, green, blue;

    wxString colorStr = hex;
    if (colorStr.StartsWith("#")) {
        colorStr = colorStr.Mid(1);
    }
    colorStr.Mid(0, 2).ToLong(&red, 16);
    colorStr.Mid(2, 2).ToLong(&green, 16);
    colorStr.Mid(4, 2).ToLong(&blue, 16);

    return wxColour(red, green, blue);
}

wxString InfoManage::get_default_gcode_file_name() 
{

    return Slic3r::GUI::wxGetApp().plater()->GetdefaultGcodeFileNmae();

}

void InfoManage::CreateSideToolBtn(int flag) 
{
    // flag:1.remotePrint 2.farmPrinter 3.sendPrinter 4.coudeUpload
    if (!m_showRemote)
        m_showRemote = flag == 1;

    if (!m_showFarm)
        m_showFarm = flag == 2;

    if (!m_showSend)
        m_showSend = flag == 3;

    if (!m_showCloud)
        m_showCloud = flag == 4;


    SideButton* print_option_btn = Slic3r::GUI::wxGetApp().mainframe->m_print_option_btn;
    if (!print_option_btn)
        return;
    SideButton* print_btn = Slic3r::GUI::wxGetApp().mainframe->m_print_btn;


    print_option_btn->Bind(wxEVT_BUTTON, [this, print_btn](wxCommandEvent& event) {
        SidePopup* p = new SidePopup(Slic3r::GUI::wxGetApp().mainframe);
        if (Slic3r::GUI::wxGetApp().preset_bundle) {

            if (m_showRemote)
            {
                SideButton* remotePrint_btn = new SideButton(p, _L("Remote Print"), "");
                remotePrint_btn->SetCornerRadius(0);
                remotePrint_btn->Bind(wxEVT_BUTTON, [this, p, print_btn](wxCommandEvent&) {
                    print_btn->SetLabel(_L("Remote Print"));
                    m_print_select = eRemotePrinting;
                    m_print_enable = get_enable_print_status();
                    print_btn->Enable(m_print_enable);
                    Slic3r::GUI::wxGetApp().mainframe->Layout();
                    p->Dismiss();
                });
                p->append_button(remotePrint_btn);
            }

            if (m_showSend) {
                SideButton* sendToPrinters_btn = new SideButton(p, _L("Send To Printers"), "");
                sendToPrinters_btn->SetCornerRadius(0);
                sendToPrinters_btn->Bind(wxEVT_BUTTON, [this, p, print_btn](wxCommandEvent&) {
                    print_btn->SetLabel(_L("Send To Printers"));
                    m_print_select = eSendToPrinters;
                    m_print_enable = get_enable_print_status();
                    print_btn->Enable(m_print_enable);
                    Slic3r::GUI::wxGetApp().mainframe->Layout();
                    p->Dismiss();
                });
                p->append_button(sendToPrinters_btn);
            }

            if (m_showFarm) {
                SideButton* sendTaskList_btn = new SideButton(p, _L("Send Task List"), "");
                sendTaskList_btn->SetCornerRadius(0);
                sendTaskList_btn->Bind(wxEVT_BUTTON, [this, p, print_btn](wxCommandEvent&) {
                    print_btn->SetLabel(_L("Send Task List"));
                    m_print_select = eSendTaskList;
                    m_print_enable = get_enable_print_status();
                    print_btn->Enable(m_print_enable);
                    Slic3r::GUI::wxGetApp().mainframe->Layout();
                    p->Dismiss();
                });
                p->append_button(sendTaskList_btn);
            }

            {
                SideButton* export_gcode_btn = new SideButton(p, _L("Export G-code file"), "");
                export_gcode_btn->SetCornerRadius(0);
                export_gcode_btn->Bind(wxEVT_BUTTON, [this, p, print_btn](wxCommandEvent&) {
                    print_btn->SetLabel(_L("Export G-code file"));
                    m_print_select = eExportGcode;
                    m_print_enable = get_enable_print_status();
                    print_btn->Enable(m_print_enable);
                    Slic3r::GUI::wxGetApp().mainframe->Layout();
                    p->Dismiss();
                });
                p->append_button(export_gcode_btn);
            }

            if (m_showCloud)
            {
                SideButton* upload = new SideButton(p, _L("Send To Cloud File"), "");
                upload->SetCornerRadius(0);
                upload->Bind(wxEVT_BUTTON, [this, p, print_btn](wxCommandEvent&) {
                    print_btn->SetLabel(_L("Send To Cloud File"));
                    m_print_select = eUploadGcode;
                    m_print_enable = get_enable_print_status();
                    print_btn->Enable(m_print_enable);
                    Slic3r::GUI::wxGetApp().mainframe->Layout();
                    p->Dismiss();
                });
                p->append_button(upload);
            }


            p->Popup(print_btn);
        }
        
    });

    print_btn->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
        bool fromCloud = false;
        int  type      = -1;
        if (m_print_select == int(eExportGcode)) {
            wxPostEvent(Slic3r::GUI::wxGetApp().mainframe->m_plater, SimpleEvent(EVT_GLTOOLBAR_EXPORT_GCODE));
            return;
        }else if (m_print_select == int(eUploadGcode)) {
            Anycubic::Plugins::dispatch_call<void>(host_, "remoteManger", "doGcodeUploadEvent");
            return;
        } else if (m_print_select == int(eRemotePrinting)) {
            type = 0;
        } else if (m_print_select == int(eSendTaskList)) {
            type = 1;
        } else if (m_print_select == int(eSendToPrinters)) {
            type = 2;
        }
        Anycubic::Plugins::dispatch_call<void>(host_, "remoteManger", "doRemotePrintEvent", type);
    });
}
bool InfoManage::get_enable_print_status() 
{
    bool enable = true;

    PartPlateList& part_plate_list = Slic3r::GUI::wxGetApp().mainframe->m_plater->get_partplate_list();
    PartPlate*     current_plate   = part_plate_list.get_curr_plate();
    bool           is_all_plates   = wxGetApp().plater()->get_preview_canvas3D()->is_all_plates_selected();
    bool isLogin                   = Anycubic::Plugins::dispatch_call<bool>(host_, "cloud_client", "is_login");
    if (m_print_select == int(eExportGcode)) {
        if (!current_plate->is_slice_result_valid()) {
            enable = false;
        }
        enable = enable && !is_all_plates;
    } else if (m_print_select == int(eUploadGcode)) {
        wxString lastGcodeFile = GetLastLoadGcode();

        if (!current_plate->is_slice_result_valid()) {
            enable = false;
        }
        if (!lastGcodeFile.empty())
            enable = true;

        enable = enable && !is_all_plates && isLogin;

    } else if (m_print_select == int(eSendTaskList)) {
        wxString lastGcodeFile = GetLastLoadGcode();

        if (!current_plate->is_slice_result_ready_for_print()) {
            enable = false;
        }
        if (!lastGcodeFile.empty())
            enable = true;

        enable = isLogin;

        enable = enable && !is_all_plates;

    } else if (m_print_select == int(eSendToPrinters)) {
        wxString lastGcodeFile = GetLastLoadGcode();

        if (!current_plate->is_slice_result_ready_for_print()) {
            enable = false;
        }
        if (!lastGcodeFile.empty())
            enable = true;

        enable = enable && !is_all_plates;

    } else if (m_print_select == int(eRemotePrinting)) {
        wxString lastGcodeFile = GetLastLoadGcode();

        if (!current_plate->is_slice_result_ready_for_print()) {
            enable = false;
        }
        if (!lastGcodeFile.empty())
            enable = true;
        

        enable = isLogin;

        enable = enable && !is_all_plates;
    }

    BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << boost::format(": m_print_select %1%, enable= %2% ") % m_print_select % enable;

    return enable;

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
