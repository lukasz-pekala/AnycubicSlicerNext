#pragma once

#include <wx/wx.h>

wxString from_u8(const std::string& str);

std::string into_u8(const wxString& str);

enum RunCallTestModel {
    r_upload           = 0,
    r_print            = 1,
    r_calculatePrinter = 2,
};
struct FileUploadObj
{
    bool            isCloud{true};
    wxVector<int> processList;
    wxVector<wxString> errList;

};

struct RemotePrintObj
{
    bool        succeed       = false;
    wxString printer_id    = "";
    wxString result_string = "";
    wxString device_id     = "";
    wxString task_id       = "";
    bool        isLANPrint    = false;
};


struct OpAmsChange
{
    wxString deviceID;
    int         countIndex;
    int         slotNum;
    wxString    slotFilament;
};

struct AmsSlotObj
{
    int      count;
    int      slotName;
    wxString filament;
    wxColour filamentColor;
    int      sourceBoxId{-2};
};

struct intDoubleMap
{
    int    slotInt;
    double slotDouble;
};

struct AmsLinkMappingObj
{
    int                   num;
    wxString              filamentName;
    wxColour              filamentColor;
    wxVector<intDoubleMap> slotInfoMap;
};

struct GcodeFilamentColor
{
    int      num;
    wxString filament;
    wxColour filamentColor;
    int      sloop;
    wxString weight;
};


struct ValueComparator
{
    bool                   operator()(const int& a, const int& b) const { return mapRef->at(a).slotInt < mapRef->at(b).slotInt; }
    wxVector<intDoubleMap>* mapRef;
};

class ACScrolledNoFocusWindow : public wxScrolledWindow
{
public:
    ACScrolledNoFocusWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
        : wxScrolledWindow(parent, id, pos, size, style)
    {}
    ~ACScrolledNoFocusWindow() = default;

protected:
    virtual bool ShouldScrollToChildOnFocus(wxWindow* child) override { return false; }
};

class ACZipThread : public wxThread
{
public:
    ACZipThread(wxEvtHandler* handler, wxString fileIndex, bool isLan, wxString infoStr = "");

protected:
    virtual ExitCode Entry() override;

private:
    wxEvtHandler* m_handler;
    wxString      m_fileIndex;
    bool          m_isLan;
    wxString      m_infoStr;
};

struct CloudSlicerInfoObj_FilamentColor
{
    wxString      material_type;
    wxVector<int> color;
};

struct ModelSlicerInfo
{
    int                             fileID{-1};
    bool                            isCloudStart{false};
    wxString                        gcodeName;
    wxString                     imgBase64;
    wxString                     print_time;
    wxString                     used_filament;
    double                          filamentWeight_d;
    float                           print_time_f;
    wxString                     modleLayers;
    wxString                     filament_length;
    wxString                     filament_type;
    wxVector<int>                filament_color;
    int                             machine_type;
    wxString                     printerName;
    bool                            isGcode{false};
    wxVector<unsigned int>       extruder_idsList;
    wxVector<wxString>        filamentColors_slicer;
    wxVector<wxString>        filamentTypess_slicer;
    wxVector<wxString>        filament_types_gcode;
    wxVector<wxString>        extruder_colors_gcode;
    wxVector<GcodeFilamentColor> gcodeFilamentColorList;

    wxVector<CloudSlicerInfoObj_FilamentColor> filamentSlicerList;
};

struct CloudSlicerInfoObj
{
    int                                           openType{0}; // 0: print 1:framtask
    int                                           machine_type;
    wxString                                   machine_name;
    wxString                                   thumbnail;
    wxString                                   name;
    wxString                                   ams_info;
    wxVector<CloudSlicerInfoObj_FilamentColor> filament_color;
    wxString                                   print_time;
    int                                           estimate;
    int                                           total_layers;
    double                                        used_filament;
    double                                        length_filament;
};

struct R_BatchResult
{
    int32_t     estimate;
    int32_t     total_layers;
    double      supplies_usage;
    wxString print_time;
};

struct ZipFinishObj
{
    bool     isLan{false};
    wxString gcodeName;
    wxString filePath;
    wxString baseUrl;
};

struct FramGroupSendTaskObj
{
    wxVector<int> printer_ids;
    bool             is_start_print{false};
    int              print_number{1};
    int              work_project_group_id{0};
    int              bulk_id{0};
    int              empty_group_id{0};
};

struct FramGroupCloudObj
{
    int      id;
    int      user_id;
    wxString name;
    int      print_number;
    int      status;
};

struct FramGroupCloudList
{
    wxVector<FramGroupCloudObj> g_framGroupCloudList;
};



struct WebWakeUpObj
{
    wxString accessToken;
    wxString hash;
    wxString fileName;
    wxString userId;
    int         fileId{-1};
    int         fileType{-1};
    bool        regionCn{false};
    bool        prod{false};
};

struct LANSendFileResultObj
{
    bool     succeed{false};
    wxString resultStr;
    wxString deviceID;
};

struct LANInfoObj
{
    wxString gcodeName;
    wxString filePath;
    wxString baseUrl;
    wxString deviceID;
    bool     auto_leveling_support{false};
    bool     vibration_compensation_support{false};
    bool     flow_calibration_support{false};
    bool     drying_first_support{false};
    bool     camera_timelapse{false};
    bool     gcode_3mf_support{false};

public:
    bool isQualInfo(LANInfoObj& obj);
};

struct LANInfoObjList
{
    wxVector<LANInfoObj> lanInfoObjList;
};

struct Peripherie
{
    int camera{-1};
    int multiColorBox{-1};
    int udisk{-1};

public:
    bool isQualInfo(Peripherie& obj);
};

struct CPrintOptions
{
    wxString function_name;
    wxString function_des;
    wxString name;
};

struct DryingStateObj
{
    wxString deviceID;
    int         selectIndex{-1};
    wxString    dryTemp;
    wxString    drtTime;
    bool        isEnable{false};

public:
    void Clear()
    {
        deviceID = "";
        selectIndex = -1;
        dryTemp     = "";
        drtTime     = "";
        isEnable    = false;
    }
};

struct CPrinterFunctions
{
    bool ai{false};
    bool photography{false};
    bool leveling{true};
    bool resonance{false};
    bool dryEnable{false};
    bool flowCalibration{false};
};

struct CPrintOptionsResponse
{
    int32_t                    machine_type;
    int32_t                    padding;
    wxString                   name;
    wxVector<CPrintOptions> optionList;
};

struct CPrintTaskResponse
{
    uint32_t    id;
    uint32_t    taskid;
    uint32_t    user_id;
    uint32_t    printer_id;
    uint32_t    gcode_id;
    uint32_t    model;
    uint32_t    estimate;
    uint32_t    pause;
    uint32_t    progress;
    uint32_t    print_status;
    uint32_t    slice_status;
    uint32_t    project_type;
    uint32_t    create_time;
    uint32_t    end_time;
    wxString img;
    wxString reason;
    wxString material;
    wxString localtask;
    wxString source;
    wxString key;
    wxString type;
    uint32_t    machine_type;
    uint32_t    padding;
    wxString printer_name;
    wxString machine_name;
    wxString gcode_name;
};

struct FilamentInfoObj
{
    wxColour    numColor;
    wxColour    numTextColor;
    wxString filament_type;
};

struct ColorBoxAndName
{
    int         slotNum;
    wxString filament_name;
    wxColour    filamentColorInfo = wxColour(255, 255, 255, 255);
    wxColour    textColorInfo     = wxColour(38, 38, 38, 255);
    int         sourceBoxId{-2};
};

struct PrinterSelectObj
{
    int      filamentNum;
    wxString filament;
    wxColour filamentColor;
    int      slotNum;
    wxColour slotColor;
    wxString slotFilament;
    int      sloopIndex;
    int      sourceBoxId{-2};

public:
    static bool CompareByIntNum(const PrinterSelectObj& obj1, const PrinterSelectObj& obj2) { return obj1.filamentNum < obj2.filamentNum; }
};

struct AmsSlotObjInfo
{
    int                   slotNum;
    wxString           filament_type;
    wxColour              filamentColorInfo;
    wxColour              textColorInfo;
    float                 capacityGap{0.0f};
    int                   iconType{0};
    wxString           sku{wxString()};
    wxString           filament_type_sub{wxString()};
    wxVector<wxColour> skuColors;
    int                   sourceBoxId{-2};

public:
    bool isQualInfo(AmsSlotObjInfo& obj);
};

struct AmsBoxObj
{
    wxVector<AmsSlotObjInfo> slotInfo;
    bool                        isEnable{true};
    int                         id{0};
    wxVector<int>            m_ids;

public:
    bool        isQualInfo(AmsBoxObj& obj);
    static bool CompareByIntNum(const AmsBoxObj& obj1, const AmsBoxObj& obj2) { return obj1.id < obj2.id; }
};

struct COTAVersion
{
    int         need_update;
    int         time_cost;
    int         force_update;
    int         panding;
    bool        isSupport{false};
    wxString firmware_version;
    wxString update_desc;
    wxString target_version;

public:
    bool isQualInfo(COTAVersion& obj);
};

struct PrinterOptions
{
    bool auto_leveling_support{false};
    bool vibration_compensation_support{false};
    bool flow_calibration_support{false};
    bool drying_first_support{false};
    bool camera_timelapse_support{false};
    bool gcode_3mf_support{false};

public:
    bool isQualInfo(PrinterOptions& obj);
};

struct PrinterObj
{
    wxString       printer_Name;
    int            printer_Type; // 2:k2 3:k3
    int            printer_id;
    int            printer_State; // 0:offline 1.free 2.buy
    bool           printer_IncludeAmsBox{false};
    int            machine_type;
    bool           is_lan{false};
    wxString    printer_type_str;
    wxString    description;
    wxString    deviceID;
    wxString    printer_Type_str;
    wxString    ip;
    wxString    uuid;
    wxString    url;
    wxString    username;
    wxString    password;
    wxString    clientid;
    wxString    ca;
    wxString    cert;
    wxString    key;
    wxString       label_name;
    COTAVersion    version;
    Peripherie     peripherie;
    PrinterOptions options;

public:
    bool isQualInfo(PrinterObj& obj);
};

struct SendAmsInfoObj
{
    wxVector<AmsBoxObj> m_backAmsBoxList;
    wxVector<int>       m_ids;
};

struct CloudClientOpObj
{
    int                     type;
    wxVector<PrinterObj> objList;
    wxString             deviceID;
    int                     state;
    wxVector<AmsBoxObj>  info;
    Peripherie              peripherie;
    int                     boxId;
    AmsSlotObjInfo          newObj;
    int                     slotNum;
};

struct LANSendRemoteEventDataObjMap
{
    wxString dryingStateObjMapStr;
    DryingStateObj dryingStateObjMapObj;
};

struct LANSendRemoteEventDataObj
{
    wxString                           deviceID;
    wxVector<PrinterSelectObj>         printerAmsInfoList;
    wxVector<LANSendRemoteEventDataObjMap> dryingStateObjMap;
    wxVector<int>                      functionsList;
};

struct FramGroupInfo
{
    int                         printer_id{-1};
    bool                        g_isSelect{false};
    wxString                    g_deviceID;
    wxString                    g_printerName;
    wxString                    g_state;
    wxString                    g_groupNum;
    wxString                    g_modelName;
    wxString                    g_linkeType;
    wxVector<AmsSlotObjInfo> g_slotInfoList;
};

struct FramGroupListInfo
{
    wxVector<FramGroupInfo> g_framGroupList;
};



struct SendLanPrinterObj
{
    wxString gcodeName;
    wxString filePath;
    wxString baseUrl;
    wxString deviceID;
};



struct SendFileObj
{
    wxString                        time;
    wxString                        weight;
    wxString                        layer;
    wxString                        imgStr;
    wxString                     printerName;
    wxVector<GcodeFilamentColor> colorInfoList;
};
struct SendFileSumObjMap
{
    int parteInfoMapInt;
    SendFileObj parteInfoMapObj;
};

struct SendFileSumObj
{
    wxString                        timeSum;
    wxString                        weightSum;
    wxString                        parteSum;
    wxString                     printerName;
    wxString                        imgStr;
    wxVector<GcodeFilamentColor> colorInfoListSum;
    wxVector<SendFileSumObjMap>     parteInfoMap;
};


struct NewVersionParObj
{
    int      id;
    int      versionCode;
    int      draft;
    int      del_flag;
    wxString param_version;
    wxString data_url;
    wxString update_desc_cn;
    wxString update_desc_en;
    wxString create_time;
    wxString update_time;
};

struct UpAndDownResultObj
{
    int      type{0}; // 0:up 1:down
    bool     result{true};
    wxString contentStr;
};

struct ZipFolderObj
{
    wxString folderPath;
    wxString zipFilePath;
    bool        isShowDialog{false};
};

struct UnZipFolderObj
{
    wxString zipFilePath;
    wxString outputDir;
    bool        isShowDialog{false};
};

struct PostDownLoadObj
{
    wxString url;
    wxString filePath;
    wxString fileName;
    wxString md5;
    int         id;
};

struct PostUpLoadObj
{
    wxString filePath;
    wxString fileName;
    int         id;
    wxString md5;
};

