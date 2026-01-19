#pragma once
#include "plugins_manager/plugins_manager.hxx"

#include <boost/preprocessor/cat.hpp>
#include <plugins_sdk/event/plugin_event.hxx>
#include <plugins_base/plugins.hxx>
#include <plugins_base/plugins_base.hxx>
#include <plugins_base/funcation.hxx>
#include "slic3r/Anycubic/plugins/xrc/detail/widgets/custom_struct_manger.hxx"
#include "slic3r/Anycubic/plugins/xrc/detail/widgets/ModelSlicerInfoJson.hxx"
#include <plugins_sdk/event/detail/CloudTransferCallback.hxx>

#define PLGUINS_NAME info_manage
#define PLUGIN_NAME_STR BOOST_PP_STRINGIZE(PLGUINS_NAME)

namespace Slic3r {
class AppConfig;
};


enum PrintSelectType {
    eExportGcode         = 0,
    eUploadGcode         = 1,
    eRemotePrinting      = 2,
    eSendTaskList        = 3,
    eSendToPrinters      = 4
};

class InfoManage : public Anycubic::Plugins::Plugin
{
public:
    InfoManage(Anycubic::Plugins::PluginHost* host);
    virtual ~InfoManage();

    void Auto_BindEvent();
    void Auto_openFileName();
    int  FindStringInVector(const wxString& target);
    std::string GetAuto_SelectDeviceID() { return m_auto_selectDeviceID; }
    void        AutoSetDirPath(const wxString& dir);
    bool        IsAutoRunModel();
    void        Auto_GetFiles();
    wxVector<wxString> GetDirListFiles(const wxString& dirPath);
    std::string        getLastRemoteDeviceID() { return m_last_remote_deviceID; }
    void               updateLastTaskID(std::string taskID) { m_last_taskID = taskID; }
    bool               IsRunCallTestModel(int index);
    wxString           Auto_GetImportFileName();

    void CreateSideToolBtn(int flag);

private:

    wxColour HexToWxColour(const wxString& hex);
    void Download(const std::string& url, const TransferCallback* cb);

    void UploadFile(const std::string& url, const std::string& filePath, const std::string& fileName, uint64_t fileSize, const TransferCallback* cb);

    std::string GetFilamentSyncColourList();
    std::string GetFilamentSyncTypeList();

    std::string      LoadGcodeFileInfo(const std::string& filename);
    std::string      GetModelSlicerInfoMap(bool isPrint, const wxString& last_load_gcode);
    std::string      GetModelSlicerInfo(bool isPrint, const wxString& last_load_gcode);
    std::string      GetGcodeFileImg(wxString last_load_gcode);
    bool        persetUpdaterOperate(const std::string& cmd, wxString profiles_new_dir, std::string* error_reason);
    bool        persetUpdaterOperate_(const std::string& cmd, boost::filesystem::path profiles_new_dir, std::string* error_reason);
    std::string getAcCfg(std::string key2);
    wxString get_default_gcode_file_name();
    std::string get_curr_plate_printer_model_name();
    std::string autoExport_gcode3mf(const std::string& fileIndex, bool export_all = false);
    int         get_curr_plate_Index();
    wxString    get_preset_filament(std::string filament_type);
    bool check_is_all_plates_selected();
    void        send_upload_file_cloud_event(wxString constr);
    bool          m_autoStartIndex{false};
    int           m_showWindowType{-1}; // 1. remoteing 2.farm 3.sendPrinter
    wxString      m_auto_nowFileName;
    wxVector<wxString> m_auto_filesList;
    std::string        m_auto_selectDeviceID;
    wxString           m_auto_dirPath;
    std::string        m_last_remote_deviceID = "";
    std::string        m_last_taskID          = "";

    wxString GetLastLoadGcode();

    mutable int m_print_select{eRemotePrinting};
    mutable bool m_print_enable{true};
    bool         m_showRemote{false};
    bool         m_showFarm{false};
    bool         m_showSend{false};
    bool         m_showCloud{false};

  
private:
    bool get_enable_print_status();
    void OnPutEvent(Anycubic::Plugins::SDK::wxPluginEvent& event);

private:
    // Anycubic::Plugins::Plugin
    const char *Name(void) override { return PLUGIN_NAME_STR; };
    bool Start(void) override { return true; };
    void Stop(void) override {}
    bool AttachEvt(class wxEvtHandler *) override;
    bool DetachEvt(class wxEvtHandler *) override;
    bool BindEvt(class wxPanel* panel, class wxWindow* parent = nullptr, class wxString *bmp = nullptr) override;
    bool CreateWebview(class wxWebView *view, class wxWindow *parent = nullptr,class wxString *bmp = nullptr) override;
    void Destroy(void) override;

private:
    Anycubic::Plugins::PluginHost *host_;
    std::string download_path_;
    std::string access_token_;

    std::vector<wxEvtHandler*> m_evt_list;
    std::mutex                 mtx_;
};