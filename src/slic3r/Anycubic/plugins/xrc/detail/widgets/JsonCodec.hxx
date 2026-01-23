#pragma once

#include <string>
#include <vector>
#include <wx/string.h>
#include <wx/colour.h>
#include <wx/vector.h>
#include <nlohmann/json.hpp>
#include "custom_struct_manger.hxx"

namespace Anycubic::Plugins {

using json = nlohmann::json;

inline void to_json(json& j, const COTAVersion& c);
inline void from_json(const json& j, COTAVersion& c);
inline void to_json(json& j, const Peripherie& c);
inline void from_json(const json& j, Peripherie& c);
inline void to_json(json& j, const PrinterOptions& c);
inline void from_json(const json& j, PrinterOptions& c);
inline void to_json(json& j, const DryingStateObj& c);
inline void from_json(const json& j, DryingStateObj& c);
inline void to_json(json& j, const SendFileObj& c);
inline void from_json(const json& j, SendFileObj& c);
inline void to_json(json& j, const ModelSlicerInfo& c);
inline void from_json(const json& j, ModelSlicerInfo& c);
inline void to_json(json& j, const intDoubleMap& c);
inline void from_json(const json& j, intDoubleMap& c);
inline void to_json(json& j, const AmsLinkMappingObj& c);
inline void from_json(const json& j, AmsLinkMappingObj& c);
inline void to_json(json& j, const AmsSlotObjInfo& c);
inline void from_json(const json& j, AmsSlotObjInfo& c);
inline void to_json(json& j, const GcodeFilamentColor& c);
inline void from_json(const json& j, GcodeFilamentColor& c);
inline void to_json(json& j, const CloudSlicerInfoObj_FilamentColor& c);
inline void from_json(const json& j, CloudSlicerInfoObj_FilamentColor& c);
inline void to_json(json& j, const FramGroupCloudObj& c);
inline void from_json(const json& j, FramGroupCloudObj& c);
inline void to_json(json& j, const AmsBoxObj& c);
inline void from_json(const json& j, AmsBoxObj& c);
inline void to_json(json& j, const PrinterObj& c);
inline void from_json(const json& j, PrinterObj& c);
inline void to_json(json& j, const PrinterSelectObj& c);
inline void from_json(const json& j, PrinterSelectObj& c);
inline void to_json(json& j, const LANSendRemoteEventDataObjMap& c);
inline void from_json(const json& j, LANSendRemoteEventDataObjMap& c);
inline void to_json(json& j, const FramGroupInfo& c);
inline void from_json(const json& j, FramGroupInfo& c);
inline void to_json(json& j, const SendFileSumObjMap& c);
inline void from_json(const json& j, SendFileSumObjMap& c);
inline void to_json(json& j, const LANInfoObj& c);
inline void from_json(const json& j, LANInfoObj& c);
inline void to_json(json& j, const CPrintOptions& c);
inline void from_json(const json& j, CPrintOptions& c);
inline void to_json(json& j, const AmsSlotObj& c);
inline void from_json(const json& j, AmsSlotObj& c);
inline void to_json(json& j, const CPrintOptionsResponse& c);
inline void from_json(const json& j, CPrintOptionsResponse& c);
inline void to_json(json& j, const FramGroupSendTaskObj& c);
inline void from_json(const json& j, FramGroupSendTaskObj& c);
inline void to_json(json& j, const FramGroupListInfo& c);
inline void from_json(const json& j, FramGroupListInfo& c);
inline void to_json(json& j, const FramGroupCloudList& c);
inline void from_json(const json& j, FramGroupCloudList& c);
inline void to_json(json& j, const LANSendRemoteEventDataObj& c);
inline void from_json(const json& j, LANSendRemoteEventDataObj& c);
inline void to_json(json& j, const LANInfoObjList& c);
inline void from_json(const json& j, LANInfoObjList& c);
inline void to_json(json& j, const OpAmsChange& c);
inline void from_json(const json& j, OpAmsChange& c);



inline std::string WxToUtf8(const wxString& s)
{
    auto buffer_utf8 = s.utf8_str();
    return std::string(buffer_utf8.data());
}

inline wxString Utf8ToWx(const std::string& s)
{

    return wxString::FromUTF8(s.c_str());
}
inline void to_json(json& j, const wxColour& c) {
    j = {
        {"r", c.Red()},
        {"g", c.Green()},
        {"b", c.Blue()},
        {"a", c.Alpha()}
    };
}

inline void from_json(const json& j, wxColour& c) {
    c.Set(
        j.value("r", 0),
        j.value("g", 0),
        j.value("b", 0),
        j.value("a", 255)
    );
}

/* ================= traits ================= */

template<typename T>
inline constexpr bool is_basic_v =
std::is_arithmetic_v<T> || std::is_enum_v<T>;

template<typename T, typename = void>
struct has_plugins_to_json : std::false_type {};

template<typename T>
struct has_plugins_to_json<T, std::void_t<
    decltype(Anycubic::Plugins::to_json(
        std::declval<json&>(),
        std::declval<const T&>()))
    >> : std::true_type {};

template<typename T>
inline constexpr bool has_plugins_to_json_v = has_plugins_to_json<T>::value;

template<typename T, typename = void>
struct has_plugins_from_json : std::false_type {};

template<typename T>
struct has_plugins_from_json<T, std::void_t<
    decltype(Anycubic::Plugins::from_json(
        std::declval<const json&>(),
        std::declval<T&>()))
    >> : std::true_type {};

template<typename T>
inline constexpr bool has_plugins_from_json_v = has_plugins_from_json<T>::value;

template<typename>
struct always_false : std::false_type {};

/* ================= element -> json ================= */

inline json element_to_json(const wxColour& c) {
    json j;
    Anycubic::Plugins::to_json(j, c);
    return j;
}

template<typename T>
json element_to_json(const T& v)
{
    if constexpr (std::is_same_v<T, wxString>) {
        return WxToUtf8(v);
    }
    else if constexpr (std::is_same_v<T, std::string>) {
        return v;
    }
    else if constexpr (is_basic_v<T>) {
        return v;
    }
    else if constexpr (has_plugins_to_json_v<T>) {
        json j;
        Anycubic::Plugins::to_json(j, v);
        return j;
    }
    else {
        static_assert(always_false<T>::value,
            "No to_json(json&, const T&) for this type");
    }
}

/* ================= json -> element ================= */


inline wxColour json_to_element(const json& j)
{
    wxColour c;
    Anycubic::Plugins::from_json(j, c);
    return c;
}

template<typename T>
T json_to_element(const json& j)
{
    if constexpr (std::is_same_v<T, wxString>) {
        return Utf8ToWx(j.get<std::string>());
    }
    else if constexpr (std::is_same_v<T, std::string>) {
        return j.get<std::string>();
    }
    else if constexpr (is_basic_v<T>) {
        return j.get<T>();
    }
    else if constexpr (has_plugins_from_json_v<T>) {
        T v{};
        Anycubic::Plugins::from_json(j, v);
        return v;
    }
    else {
        static_assert(always_false<T>::value,
            "No from_json(json const&, T&) for this type");
    }
}

/* ================= wxVector ================= */

template<typename T>
json vector_to_json(const wxVector<T>& vec)
{
    json j = json::array();
    for (const auto& v : vec)
        j.push_back(element_to_json(v));
    return j;
}

template<typename T>
wxVector<T> json_to_vector(const json& j)
{
    static_assert(
        is_basic_v<T> || has_plugins_from_json_v<T>,
        "wxVector<T>: T must be deserializable"
        );

    wxVector<T> vec;
    if (!j.is_array()) return vec;

    for (const auto& item : j)
        vec.push_back(json_to_element<T>(item));
    return vec;
}


template<typename T>
inline std::string to_json_string(const T& obj) {
    return element_to_json(obj).dump(2);
}

template<typename T>
inline T from_json_string(const std::string& s) {
    if (s.empty()) return {};
    return json_to_element<T>(json::parse(s));
}

template<typename T>
inline std::string to_json_string_any(const wxVector<T>& vec) {
    return vector_to_json(vec).dump(2);
}

template<typename T>
inline wxVector<T> from_json_string_any(const std::string& s) {
    if (s.empty()) return {};
    return json_to_vector<T>(json::parse(s));
}

inline wxVector<wxString>from_json_string_wxstring_vector(const nlohmann::json& j)
{
    wxVector<wxString> vec;

    nlohmann::json arr;

    if (j.is_array())
    {
        arr = j;
    }
    else if (j.is_string())
    {
        arr = nlohmann::json::parse(
            j.get_ref<const std::string&>(),
            nullptr,
            false
        );
    }
    else
    {
        return vec;
    }

    if (!arr.is_array())
        return vec;

    vec.reserve(arr.size());

    for (const auto& item : arr)
    {
        if (!item.is_string())
            continue;

        const std::string& s = item.get_ref<const std::string&>();
        vec.push_back(wxString::FromUTF8(s.c_str()));
    }

    return vec;
}




/* ---------------- FileUploadObj ---------------- */
inline void to_json(json& j, const FileUploadObj& f) {
    j = {{"isCloud", f.isCloud}, {"processList", vector_to_json(f.processList)}, {"errList", vector_to_json(f.errList)}};
}
inline void from_json(const json& j, FileUploadObj& f) {
    f.isCloud = j.value("isCloud", true);
    if (j.contains("processList")) f.processList = j.at("processList").get<wxVector<int>>();
    if (j.contains("errList")) f.errList = from_json_string_wxstring_vector(j.at("errList"));
}

/* ---------------- RemotePrintObj ---------------- */
inline void to_json(json& j, const RemotePrintObj& r) {
    j = { {"succeed", r.succeed},
          {"printer_id", WxToUtf8(r.printer_id)},
          {"result_string", WxToUtf8(r.result_string)},
          {"device_id", WxToUtf8(r.device_id)},
          {"task_id", WxToUtf8(r.task_id)},
          {"isLANPrint", r.isLANPrint} };
}
inline void from_json(const json& j, RemotePrintObj& r) {
    r.succeed = j.value("succeed", false);
    if (j.contains("printer_id")) r.printer_id = Utf8ToWx(j.at("printer_id").get<std::string>());
    if (j.contains("result_string")) r.result_string = Utf8ToWx(j.at("result_string").get<std::string>());
    if (j.contains("device_id")) r.device_id = Utf8ToWx(j.at("device_id").get<std::string>());
    if (j.contains("task_id")) r.task_id = Utf8ToWx(j.at("task_id").get<std::string>());
    r.isLANPrint = j.value("isLANPrint", false);
}

/* ---------------- OpAmsChange ---------------- */
inline void to_json(json& j, const OpAmsChange& o) {
    j = { {"deviceID", WxToUtf8(o.deviceID)}, {"countIndex", o.countIndex}, {"slotNum", o.slotNum}, {"slotFilament", WxToUtf8(o.slotFilament)} };
}
inline void from_json(const json& j, OpAmsChange& o) {
    if (j.contains("deviceID")) o.deviceID = Utf8ToWx(j.at("deviceID").get<std::string>());
    o.countIndex = j.value("countIndex", 0);
    o.slotNum = j.value("slotNum", 0);
    if (j.contains("slotFilament")) o.slotFilament = Utf8ToWx(j.at("slotFilament").get<std::string>());
}

/* ---------------- AmsSlotObj ---------------- */
inline void to_json(json& j, const AmsSlotObj& a) {
    j = { {"count", a.count}, {"slotName", a.slotName}, {"filament", WxToUtf8(a.filament)}, {"filamentColor",element_to_json(a.filamentColor)}, {"sourceBoxId", a.sourceBoxId} };
}
inline void from_json(const json& j, AmsSlotObj& a) {
    a.count = j.value("count", 0);
    a.slotName = j.value("slotName", 0);
    if (j.contains("filament")) a.filament = Utf8ToWx(j.at("filament").get<std::string>());
    if (j.contains("filamentColor")) a.filamentColor = json_to_element<wxColour>(j.at("filamentColor"));
    a.sourceBoxId = j.value("sourceBoxId", -2);
}

/* ---------------- intDoubleMap ---------------- */
inline void to_json(json& j, const intDoubleMap& m) { j = { {"slotInt", m.slotInt}, {"slotDouble", m.slotDouble} }; }
inline void from_json(const json& j, intDoubleMap& m) { m.slotInt = j.value("slotInt", 0); m.slotDouble = j.value("slotDouble", 0.0); }

/* ---------------- AmsLinkMappingObj ---------------- */
inline void to_json(json& j, const AmsLinkMappingObj& a) {
    j = { {"num", a.num}, {"filamentName", WxToUtf8(a.filamentName)}, {"filamentColor", element_to_json(a.filamentColor)}, {"slotInfoMap", vector_to_json(a.slotInfoMap)} };
}
inline void from_json(const json& j, AmsLinkMappingObj& a) {
    a.num = j.value("num", 0);
    if (j.contains("filamentName")) a.filamentName = Utf8ToWx(j.at("filamentName").get<std::string>());
    if (j.contains("filamentColor")) a.filamentColor = json_to_element<wxColour>(j.at("filamentColor"));
    if (j.contains("slotInfoMap")) a.slotInfoMap = json_to_vector<intDoubleMap>(j.at("slotInfoMap"));;
}

/* ---------------- GcodeFilamentColor ---------------- */
inline void to_json(json& j, const GcodeFilamentColor& g) {
    json color_j;
    to_json(color_j, g.filamentColor);

    j = { {"num", g.num}, {"filament", WxToUtf8(g.filament)}, {"filamentColor", color_j}, {"sloop", g.sloop}, {"weight", WxToUtf8(g.weight)} };
}
inline void from_json(const json& j, GcodeFilamentColor& g) {
    g.num = j.value("num", 0);
    if (j.contains("filament")) g.filament = Utf8ToWx(j.at("filament").get<std::string>());
    if (j.contains("filamentColor")) g.filamentColor = json_to_element<wxColour>(j.at("filamentColor"));
    g.sloop = j.value("sloop", 0);
    if (j.contains("weight")) g.weight = Utf8ToWx(j.at("weight").get<std::string>());
}


/* ---------------- CloudSlicerInfoObj_FilamentColor ---------------- */
inline void to_json(json& j, const CloudSlicerInfoObj_FilamentColor& f) {
    j = { {"material_type", WxToUtf8(f.material_type)}, {"color", f.color} };
}
inline void from_json(const json& j, CloudSlicerInfoObj_FilamentColor& f) {
    if (j.contains("material_type")) f.material_type = Utf8ToWx(j.at("material_type").get<std::string>());
    if (j.contains("color")) f.color = j.at("color").get<wxVector<int>>();
}

/* ---------------- ModelSlicerInfo ---------------- */
inline void to_json(json& j, const ModelSlicerInfo& m) {
    j = {
    {"fileID", m.fileID},
    {"isCloudStart", m.isCloudStart},
    {"gcodeName", WxToUtf8(m.gcodeName)},
    {"imgBase64", WxToUtf8(m.imgBase64)},
    {"print_time", WxToUtf8(m.print_time)},
    {"used_filament", WxToUtf8(m.used_filament)},
    {"filamentWeight_d", m.filamentWeight_d},
    {"print_time_f", m.print_time_f},
    {"modleLayers", WxToUtf8(m.modleLayers)},
    {"filament_length", WxToUtf8(m.filament_length)},
    {"filament_type", WxToUtf8(m.filament_type)},
    {"filament_color", vector_to_json(m.filament_color)},          // wxVector<int>
    {"machine_type", m.machine_type},
    {"printerName", WxToUtf8(m.printerName)},
    {"isGcode", m.isGcode},
    {"extruder_idsList", vector_to_json(m.extruder_idsList)},       // wxVector<unsigned int>
    {"filamentColors_slicer", vector_to_json(m.filamentColors_slicer)}, // wxVector<wxString>
    {"filamentTypess_slicer", vector_to_json(m.filamentTypess_slicer)}, // wxVector<wxString>
    {"filament_types_gcode", vector_to_json(m.filament_types_gcode)},   // wxVector<wxString>
    {"extruder_colors_gcode", vector_to_json(m.extruder_colors_gcode)}, // wxVector<wxString>
    {"gcodeFilamentColorList", vector_to_json(m.gcodeFilamentColorList)}, // wxVector<GcodeFilamentColor>
    {"filamentSlicerList", vector_to_json(m.filamentSlicerList)}        // wxVector<CloudSlicerInfoObj_FilamentColor>
    };

}
inline void from_json(const json& j, ModelSlicerInfo& m) {
    m.fileID = j.value("fileID", -1);
    m.isCloudStart = j.value("isCloudStart", false);
    if (j.contains("gcodeName")) m.gcodeName = Utf8ToWx(j.at("gcodeName").get<std::string>());
    if (j.contains("imgBase64")) m.imgBase64 = Utf8ToWx(j.at("imgBase64").get<std::string>());
    if (j.contains("print_time")) m.print_time = Utf8ToWx(j.at("print_time").get<std::string>());
    if (j.contains("used_filament")) m.used_filament = Utf8ToWx(j.at("used_filament").get<std::string>());
    m.filamentWeight_d = j.value("filamentWeight_d", 0.0);
    m.print_time_f = j.value("print_time_f", 0.0f);
    if (j.contains("modleLayers")) m.modleLayers = Utf8ToWx(j.at("modleLayers").get<std::string>());
    if (j.contains("filament_length")) m.filament_length = Utf8ToWx(j.at("filament_length").get<std::string>());
    if (j.contains("filament_type")) m.filament_type = Utf8ToWx(j.at("filament_type").get<std::string>());
    m.machine_type = j.value("machine_type", 0);
    if (j.contains("printerName")) m.printerName = Utf8ToWx(j.at("printerName").get<std::string>());
    m.isGcode = j.value("isGcode", false);
    if (j.contains("filament_color")) m.filament_color = j.at("filament_color").get<wxVector<int>>();
    if (j.contains("extruder_idsList")) m.extruder_idsList = j.at("extruder_idsList").get<wxVector<unsigned int>>();
    if (j.contains("filamentColors_slicer")) m.filamentColors_slicer = from_json_string_wxstring_vector(j.at("filamentColors_slicer"));
    if (j.contains("filamentTypess_slicer")) m.filamentTypess_slicer = from_json_string_wxstring_vector(j.at("filamentTypess_slicer"));
    if (j.contains("filament_types_gcode")) m.filament_types_gcode = from_json_string_wxstring_vector(j.at("filament_types_gcode"));
    if (j.contains("extruder_colors_gcode")) m.extruder_colors_gcode = from_json_string_wxstring_vector(j.at("extruder_colors_gcode"));
    if (j.contains("gcodeFilamentColorList")) m.gcodeFilamentColorList = json_to_vector<GcodeFilamentColor>(j.at("gcodeFilamentColorList"));
    if (j.contains("filamentSlicerList")) m.filamentSlicerList = json_to_vector<CloudSlicerInfoObj_FilamentColor>(j.at("filamentSlicerList"));
}

/* ---------------- CloudSlicerInfoObj ---------------- */
inline void to_json(json& j, const CloudSlicerInfoObj& c) {
    
    j = {
        {"openType", c.openType}, {"machine_type", c.machine_type}, {"machine_name", WxToUtf8(c.machine_name)},
        {"thumbnail", WxToUtf8(c.thumbnail)}, {"name", WxToUtf8(c.name)}, {"ams_info", WxToUtf8(c.ams_info)},
        {"filament_color", vector_to_json(c.filament_color)}, {"print_time", WxToUtf8(c.print_time)}, {"estimate", c.estimate},
        {"total_layers", c.total_layers}, {"used_filament", c.used_filament}, {"length_filament", c.length_filament}
    };
}
inline void from_json(const json& j, CloudSlicerInfoObj& c) {
    c.openType = j.value("openType", 0);
    c.machine_type = j.value("machine_type", 0);
    if (j.contains("machine_name")) c.machine_name = Utf8ToWx(j.at("machine_name").get<std::string>());
    if (j.contains("thumbnail")) c.thumbnail = Utf8ToWx(j.at("thumbnail").get<std::string>());
    if (j.contains("name")) c.name = Utf8ToWx(j.at("name").get<std::string>());
    if (j.contains("ams_info")) c.ams_info = Utf8ToWx(j.at("ams_info").get<std::string>());
    if (j.contains("filament_color")) c.filament_color = json_to_vector<CloudSlicerInfoObj_FilamentColor>(j.at("filament_color"));
    if (j.contains("print_time")) c.print_time = Utf8ToWx(j.at("print_time").get<std::string>());
    c.estimate = j.value("estimate", 0);
    c.total_layers = j.value("total_layers", 0);
    c.used_filament = j.value("used_filament", 0.0);
    c.length_filament = j.value("length_filament", 0.0);
}

/* ---------------- R_BatchResult ---------------- */
inline void to_json(json& j, const R_BatchResult& r) {
    j = { {"estimate", r.estimate}, {"total_layers", r.total_layers}, {"supplies_usage", r.supplies_usage}, {"print_time", WxToUtf8(r.print_time)} };
}
inline void from_json(const json& j, R_BatchResult& r) {
    r.estimate = j.value("estimate", 0);
    r.total_layers = j.value("total_layers", 0);
    r.supplies_usage = j.value("supplies_usage", 0.0);
    if (j.contains("print_time")) r.print_time = Utf8ToWx(j.at("print_time").get<std::string>());
}

/* ---------------- ZipFinishObj ---------------- */
inline void to_json(json& j, const ZipFinishObj& z) {
    j = { {"isLan", z.isLan}, {"gcodeName", WxToUtf8(z.gcodeName)}, {"filePath", WxToUtf8(z.filePath)}, {"baseUrl", WxToUtf8(z.baseUrl)} };
}
inline void from_json(const json& j, ZipFinishObj& z) {
    z.isLan = j.value("isLan", false);
    if (j.contains("gcodeName")) z.gcodeName = Utf8ToWx(j.at("gcodeName").get<std::string>());
    if (j.contains("filePath")) z.filePath = Utf8ToWx(j.at("filePath").get<std::string>());
    if (j.contains("baseUrl")) z.baseUrl = Utf8ToWx(j.at("baseUrl").get<std::string>());
}


/* ---------------- FramGroupSendTaskObj ---------------- */
inline void to_json(json& j, const FramGroupSendTaskObj& f) {
    j = {
        {"printer_ids", f.printer_ids},
        {"is_start_print", f.is_start_print},
        {"print_number", f.print_number},
        {"work_project_group_id", f.work_project_group_id},
        {"bulk_id", f.bulk_id},
        {"empty_group_id", f.empty_group_id}
    };
}

inline void from_json(const json& j, FramGroupSendTaskObj& f) {
    if (j.contains("printer_ids")) f.printer_ids = j.at("printer_ids").get<wxVector<int>>();
    f.is_start_print = j.value("is_start_print", false);
    f.print_number = j.value("print_number", 1);
    f.work_project_group_id = j.value("work_project_group_id", 0);
    f.bulk_id = j.value("bulk_id", 0);
    f.empty_group_id = j.value("empty_group_id", 0);
}

/* ---------------- FramGroupCloudObj ---------------- */
inline void to_json(json& j, const FramGroupCloudObj& f) {
    j = {
        {"id", f.id},
        {"user_id", f.user_id},
        {"name", WxToUtf8(f.name)},
        {"print_number", f.print_number},
        {"status", f.status}
    };
}

inline void from_json(const json& j, FramGroupCloudObj& f) {
    f.id = j.value("id", 0);
    f.user_id = j.value("user_id", 0);
    if (j.contains("name")) f.name = Utf8ToWx(j.at("name").get<std::string>());
    f.print_number = j.value("print_number", 0);
    f.status = j.value("status", 0);
}

/* ---------------- FramGroupCloudList ---------------- */
inline void to_json(json& j, const FramGroupCloudList& f) {
    j = { {"g_framGroupCloudList", vector_to_json(f.g_framGroupCloudList)} };
}

inline void from_json(const json& j, FramGroupCloudList& f) {
    if (j.contains("g_framGroupCloudList"))
        f.g_framGroupCloudList = json_to_vector<FramGroupCloudObj>(j.at("g_framGroupCloudList"));
}

/* ---------------- WebWakeUpObj ---------------- */
inline void to_json(json& j, const WebWakeUpObj& w) {
    j = {
        {"accessToken", WxToUtf8(w.accessToken)},
        {"hash", WxToUtf8(w.hash)},
        {"fileName", WxToUtf8(w.fileName)},
        {"userId", WxToUtf8(w.userId)},
        {"fileId", w.fileId},
        {"fileType", w.fileType},
        {"regionCn", w.regionCn},
        {"prod", w.prod}
    };
}

inline void from_json(const json& j, WebWakeUpObj& w) {
    if (j.contains("accessToken")) w.accessToken = Utf8ToWx(j.at("accessToken").get<std::string>());
    if (j.contains("hash")) w.hash = Utf8ToWx(j.at("hash").get<std::string>());
    if (j.contains("fileName")) w.fileName = Utf8ToWx(j.at("fileName").get<std::string>());
    if (j.contains("userId")) w.userId = Utf8ToWx(j.at("userId").get<std::string>());
    w.fileId = j.value("fileId", -1);
    w.fileType = j.value("fileType", -1);
    w.regionCn = j.value("regionCn", false);
    w.prod = j.value("prod", false);
}

/* ---------------- LANSendFileResultObj ---------------- */
inline void to_json(json& j, const LANSendFileResultObj& l) {
    j = {
        {"succeed", l.succeed},
        {"resultStr", WxToUtf8(l.resultStr)},
        {"deviceID", WxToUtf8(l.deviceID)}
    };
}

inline void from_json(const json& j, LANSendFileResultObj& l) {
    l.succeed = j.value("succeed", false);
    if (j.contains("resultStr")) l.resultStr = Utf8ToWx(j.at("resultStr").get<std::string>());
    if (j.contains("deviceID")) l.deviceID = Utf8ToWx(j.at("deviceID").get<std::string>());
}



/* ---------------- LANInfoObj ---------------- */
inline void to_json(json& j, const LANInfoObj& l) {
    j = {
        {"gcodeName", WxToUtf8(l.gcodeName)},
        {"filePath", WxToUtf8(l.filePath)},
        {"baseUrl", WxToUtf8(l.baseUrl)},
        {"deviceID", WxToUtf8(l.deviceID)},
        {"auto_leveling_support", l.auto_leveling_support},
        {"vibration_compensation_support", l.vibration_compensation_support},
        {"flow_calibration_support", l.flow_calibration_support},
        {"drying_first_support", l.drying_first_support},
        {"camera_timelapse", l.camera_timelapse},
        {"gcode_3mf_support", l.gcode_3mf_support}
    };
}

inline void from_json(const json& j, LANInfoObj& l) {
    if (j.contains("gcodeName")) l.gcodeName = Utf8ToWx(j.at("gcodeName").get<std::string>());
    if (j.contains("filePath")) l.filePath = Utf8ToWx(j.at("filePath").get<std::string>());
    if (j.contains("baseUrl")) l.baseUrl = Utf8ToWx(j.at("baseUrl").get<std::string>());
    if (j.contains("deviceID")) l.deviceID = Utf8ToWx(j.at("deviceID").get<std::string>());
    l.auto_leveling_support = j.value("auto_leveling_support", false);
    l.vibration_compensation_support = j.value("vibration_compensation_support", false);
    l.flow_calibration_support = j.value("flow_calibration_support", false);
    l.drying_first_support = j.value("drying_first_support", false);
    l.camera_timelapse = j.value("camera_timelapse", false);
    l.gcode_3mf_support = j.value("gcode_3mf_support", false);
}

/* ---------------- LANInfoObjList ---------------- */
inline void to_json(json& j, const LANInfoObjList& l) {
    j = { {"lanInfoObjList", vector_to_json(l.lanInfoObjList)} };
}

inline void from_json(const json& j, LANInfoObjList& l) {
    if (j.contains("lanInfoObjList"))
        l.lanInfoObjList = json_to_vector<LANInfoObj>(j.at("lanInfoObjList"));
}

/* ---------------- Peripherie ---------------- */
inline void to_json(json& j, const Peripherie& p) {
    j = {
        {"camera", p.camera},
        {"multiColorBox", p.multiColorBox},
        {"udisk", p.udisk}
    };
}

inline void from_json(const json& j, Peripherie& p) {
    p.camera = j.value("camera", -1);
    p.multiColorBox = j.value("multiColorBox", -1);
    p.udisk = j.value("udisk", -1);
}

/* ---------------- CPrintOptions ---------------- */
inline void to_json(json& j, const CPrintOptions& c) {
    j = {
        {"function_name", WxToUtf8(c.function_name)},
        {"function_des", WxToUtf8(c.function_des)},
        {"name", WxToUtf8(c.name)}
    };
}

inline void from_json(const json& j, CPrintOptions& c) {
    if (j.contains("function_name")) c.function_name = Utf8ToWx(j.at("function_name").get<std::string>());
    if (j.contains("function_des")) c.function_des = Utf8ToWx(j.at("function_des").get<std::string>());
    if (j.contains("name")) c.name = Utf8ToWx(j.at("name").get<std::string>());
}

/* ---------------- DryingStateObj ---------------- */
inline void to_json(json& j, const DryingStateObj& d) {
    j = {
        {"deviceID", WxToUtf8(d.deviceID)},
        {"selectIndex", d.selectIndex},
        {"dryTemp", WxToUtf8(d.dryTemp)},
        {"drtTime", WxToUtf8(d.drtTime)},
        {"isEnable", d.isEnable}
    };
}

inline void from_json(const json& j, DryingStateObj& d) {
    if (j.contains("deviceID")) d.deviceID = Utf8ToWx(j.at("deviceID").get<std::string>());
    d.selectIndex = j.value("selectIndex", -1);
    if (j.contains("dryTemp")) d.dryTemp = Utf8ToWx(j.at("dryTemp").get<std::string>());
    if (j.contains("drtTime")) d.drtTime = Utf8ToWx(j.at("drtTime").get<std::string>());
    d.isEnable = j.value("isEnable", false);
}

/* ---------------- CPrinterFunctions ---------------- */
inline void to_json(json& j, const CPrinterFunctions& c) {
    j = {
        {"ai", c.ai},
        {"photography", c.photography},
        {"leveling", c.leveling},
        {"resonance", c.resonance},
        {"dryEnable", c.dryEnable},
        {"flowCalibration", c.flowCalibration}
    };
}

inline void from_json(const json& j, CPrinterFunctions& c) {
    c.ai = j.value("ai", false);
    c.photography = j.value("photography", false);
    c.leveling = j.value("leveling", true);
    c.resonance = j.value("resonance", false);
    c.dryEnable = j.value("dryEnable", false);
    c.flowCalibration = j.value("flowCalibration", false);
}

/* ---------------- CPrintOptionsResponse ---------------- */
inline void to_json(json& j, const CPrintOptionsResponse& c) {
    j = {
        {"machine_type", c.machine_type},
        {"padding", c.padding},
        {"name", WxToUtf8(c.name)},
        {"optionList", vector_to_json(c.optionList)}
    };
}

inline void from_json(const json& j, CPrintOptionsResponse& c) {
    c.machine_type = j.value("machine_type", 0);
    c.padding = j.value("padding", 0);
    if (j.contains("name")) c.name = Utf8ToWx(j.at("name").get<std::string>());
    if (j.contains("optionList")) c.optionList = json_to_vector<CPrintOptions>(j.at("optionList"));
}



/* ---------------- FilamentInfoObj ---------------- */
inline void to_json(json& j, const FilamentInfoObj& f) {
    j = {
        {"numColor", element_to_json(f.numColor)},
        {"numTextColor", element_to_json(f.numTextColor)},
        {"filament_type", WxToUtf8(f.filament_type)}
    };
}

inline void from_json(const json& j, FilamentInfoObj& f) {
    if (j.contains("numColor")) f.numColor = json_to_element<wxColour>(j.at("numColor"));
    if (j.contains("numTextColor")) f.numTextColor = json_to_element<wxColour>(j.at("numTextColor"));
    if (j.contains("filament_type")) f.filament_type = Utf8ToWx(j.at("filament_type").get<std::string>());
}

/* ---------------- ColorBoxAndName ---------------- */
inline void to_json(json& j, const ColorBoxAndName& c) {
    j = {
        {"slotNum", c.slotNum},
        {"filament_name", WxToUtf8(c.filament_name)},
        {"filamentColorInfo", element_to_json(c.filamentColorInfo)},
        {"textColorInfo", element_to_json(c.textColorInfo)},
        {"sourceBoxId", c.sourceBoxId}
    };
}

inline void from_json(const json& j, ColorBoxAndName& c) {
    c.slotNum = j.value("slotNum", 0);
    if (j.contains("filament_name")) c.filament_name = Utf8ToWx(j.at("filament_name").get<std::string>());
    if (j.contains("filamentColorInfo")) c.filamentColorInfo = json_to_element<wxColour>(j.at("filamentColorInfo"));
    if (j.contains("textColorInfo")) c.textColorInfo = json_to_element<wxColour>(j.at("textColorInfo"));
    c.sourceBoxId = j.value("sourceBoxId", -2);
}

/* ---------------- PrinterSelectObj ---------------- */
inline void to_json(json& j, const PrinterSelectObj& p) {
    j = {
        {"filamentNum", p.filamentNum},
        {"filament", WxToUtf8(p.filament)},
        {"filamentColor", element_to_json(p.filamentColor)},
        {"slotNum", p.slotNum},
        {"slotColor", element_to_json(p.slotColor)},
        {"slotFilament", WxToUtf8(p.slotFilament)},
        {"sloopIndex", p.sloopIndex},
        {"sourceBoxId", p.sourceBoxId}
    };
}

inline void from_json(const json& j, PrinterSelectObj& p) {
    p.filamentNum = j.value("filamentNum", 0);
    if (j.contains("filament")) p.filament = Utf8ToWx(j.at("filament").get<std::string>());
    if (j.contains("filamentColor")) p.filamentColor = json_to_element<wxColour>(j.at("filamentColor"));
    p.slotNum = j.value("slotNum", 0);
    if (j.contains("slotColor")) p.slotColor = json_to_element<wxColour>(j.at("slotColor"));
    if (j.contains("slotFilament")) p.slotFilament = Utf8ToWx(j.at("slotFilament").get<std::string>());
    p.sloopIndex = j.value("sloopIndex", 0);
    p.sourceBoxId = j.value("sourceBoxId", -2);
}

/* ---------------- AmsSlotObjInfo ---------------- */
inline void to_json(json& j, const AmsSlotObjInfo& a) {
    j = {
        {"slotNum", a.slotNum},
        {"filament_type", WxToUtf8(a.filament_type)},
        {"filamentColorInfo", element_to_json(a.filamentColorInfo)},
        {"textColorInfo", element_to_json(a.textColorInfo)},
        {"capacityGap", a.capacityGap},
        {"iconType", a.iconType},
        {"sku", WxToUtf8(a.sku)},
        {"filament_type_sub", WxToUtf8(a.filament_type_sub)},
        {"skuColors", vector_to_json(a.skuColors)},
        {"sourceBoxId", a.sourceBoxId}
    };
}

inline void from_json(const json& j, AmsSlotObjInfo& a) {
    a.slotNum = j.value("slotNum", 0);
    if (j.contains("filament_type")) a.filament_type = Utf8ToWx(j.at("filament_type").get<std::string>());
    if (j.contains("filamentColorInfo")) a.filamentColorInfo = json_to_element<wxColour>(j.at("filamentColorInfo"));
    if (j.contains("textColorInfo")) a.textColorInfo = json_to_element<wxColour>(j.at("textColorInfo"));
    a.capacityGap = j.value("capacityGap", 0.0f);
    a.iconType = j.value("iconType", 0);
    if (j.contains("sku")) a.sku = Utf8ToWx(j.at("sku").get<std::string>());
    if (j.contains("filament_type_sub")) a.filament_type_sub = Utf8ToWx(j.at("filament_type_sub").get<std::string>());
    if (j.contains("skuColors")) a.skuColors = json_to_vector<wxColour>(j.at("skuColors"));
    a.sourceBoxId = j.value("sourceBoxId", -2);
}

/* ---------------- AmsBoxObj ---------------- */
inline void to_json(json& j, const AmsBoxObj& a) {
    j = {
        {"slotInfo", vector_to_json(a.slotInfo)},
        {"isEnable", a.isEnable},
        {"id", a.id},
        {"m_ids", a.m_ids}
    };
}

inline void from_json(const json& j, AmsBoxObj& a) {
    if (j.contains("slotInfo")) a.slotInfo = json_to_vector<AmsSlotObjInfo>(j.at("slotInfo"));
    a.isEnable = j.value("isEnable", true);
    a.id = j.value("id", 0);
    if (j.contains("m_ids")) a.m_ids = j.at("m_ids").get<wxVector<int>>();
}

/* ---------------- COTAVersion ---------------- */
inline void to_json(json& j, const COTAVersion& c) {
    j = {
        {"need_update", c.need_update},
        {"time_cost", c.time_cost},
        {"force_update", c.force_update},
        {"panding", c.panding},
        {"isSupport", c.isSupport},
        {"firmware_version", WxToUtf8(c.firmware_version)},
        {"update_desc", WxToUtf8(c.update_desc)},
        {"target_version", WxToUtf8(c.target_version)}
    };
}

inline void from_json(const json& j, COTAVersion& c) {
    c.need_update = j.value("need_update", 0);
    c.time_cost = j.value("time_cost", 0);
    c.force_update = j.value("force_update", 0);
    c.panding = j.value("panding", 0);
    c.isSupport = j.value("isSupport", false);
    if (j.contains("firmware_version")) c.firmware_version = Utf8ToWx(j.at("firmware_version").get<std::string>());
    if (j.contains("update_desc")) c.update_desc = Utf8ToWx(j.at("update_desc").get<std::string>());
    if (j.contains("target_version")) c.target_version = Utf8ToWx(j.at("target_version").get<std::string>());
}

/* ---------------- PrinterOptions ---------------- */
inline void to_json(json& j, const PrinterOptions& p) {
    j = {
        {"auto_leveling_support", p.auto_leveling_support},
        {"vibration_compensation_support", p.vibration_compensation_support},
        {"flow_calibration_support", p.flow_calibration_support},
        {"drying_first_support", p.drying_first_support},
        {"camera_timelapse_support", p.camera_timelapse_support},
        {"gcode_3mf_support", p.gcode_3mf_support}
    };
}

inline void from_json(const json& j, PrinterOptions& p) {
    p.auto_leveling_support = j.value("auto_leveling_support", false);
    p.vibration_compensation_support = j.value("vibration_compensation_support", false);
    p.flow_calibration_support = j.value("flow_calibration_support", false);
    p.drying_first_support = j.value("drying_first_support", false);
    p.camera_timelapse_support = j.value("camera_timelapse_support", false);
    p.gcode_3mf_support = j.value("gcode_3mf_support", false);
}

/* ---------------- PrinterObj ---------------- */
inline void to_json(json& j, const PrinterObj& p) {
    j = {
        {"printer_Name", WxToUtf8(p.printer_Name)},
        {"printer_Type", p.printer_Type},
        {"printer_id", p.printer_id},
        {"printer_State", p.printer_State},
        {"printer_IncludeAmsBox", p.printer_IncludeAmsBox},
        {"machine_type", p.machine_type},
        {"is_lan", p.is_lan},
        {"printer_type_str", WxToUtf8(p.printer_type_str)},
        {"description", WxToUtf8(p.description)},
        {"deviceID", WxToUtf8(p.deviceID)},
        {"printer_Type_str", WxToUtf8(p.printer_Type_str)},
        {"ip", WxToUtf8(p.ip)},
        {"uuid", WxToUtf8(p.uuid)},
        {"url", WxToUtf8(p.url)},
        {"username", WxToUtf8(p.username)},
        {"password", WxToUtf8(p.password)},
        {"clientid", WxToUtf8(p.clientid)},
        {"ca", WxToUtf8(p.ca)},
        {"cert", WxToUtf8(p.cert)},
        {"key", WxToUtf8(p.key)},
        {"label_name", WxToUtf8(p.label_name)},
        {"version", element_to_json(p.version)},
        {"peripherie", element_to_json(p.peripherie)},
        {"options", element_to_json(p.options)}
    };
}

inline void from_json(const json& j, PrinterObj& p) {
    if (j.contains("printer_Name")) p.printer_Name = Utf8ToWx(j.at("printer_Name").get<std::string>());
    p.printer_Type = j.value("printer_Type", 0);
    p.printer_id = j.value("printer_id", 0);
    p.printer_State = j.value("printer_State", 0);
    p.printer_IncludeAmsBox = j.value("printer_IncludeAmsBox", false);
    p.machine_type = j.value("machine_type", 0);
    p.is_lan = j.value("is_lan", false);
    if (j.contains("printer_type_str")) p.printer_type_str = Utf8ToWx(j.at("printer_type_str").get<std::string>());
    if (j.contains("description")) p.description = Utf8ToWx(j.at("description").get<std::string>());
    if (j.contains("deviceID")) p.deviceID = Utf8ToWx(j.at("deviceID").get<std::string>());
    if (j.contains("printer_Type_str")) p.printer_Type_str = Utf8ToWx(j.at("printer_Type_str").get<std::string>());
    if (j.contains("ip")) p.ip = Utf8ToWx(j.at("ip").get<std::string>());
    if (j.contains("uuid")) p.uuid = Utf8ToWx(j.at("uuid").get<std::string>());
    if (j.contains("url")) p.url = Utf8ToWx(j.at("url").get<std::string>());
    if (j.contains("username")) p.username = Utf8ToWx(j.at("username").get<std::string>());
    if (j.contains("password")) p.password = Utf8ToWx(j.at("password").get<std::string>());
    if (j.contains("clientid")) p.clientid = Utf8ToWx(j.at("clientid").get<std::string>());
    if (j.contains("ca")) p.ca = Utf8ToWx(j.at("ca").get<std::string>());
    if (j.contains("cert")) p.cert = Utf8ToWx(j.at("cert").get<std::string>());
    if (j.contains("key")) p.key = Utf8ToWx(j.at("key").get<std::string>());
    if (j.contains("label_name")) p.label_name = Utf8ToWx(j.at("label_name").get<std::string>());
    if (j.contains("version"))  p.version = json_to_element<COTAVersion>(j.at("version"));
    if (j.contains("peripherie")) p.peripherie = json_to_element<Peripherie>(j.at("peripherie"));
    if (j.contains("options"))  p.options = json_to_element<PrinterOptions>(j.at("options"));
}


/* ---------------- SendAmsInfoObj ---------------- */
inline void to_json(json& j, const SendAmsInfoObj& s) {
    j = {
        {"m_backAmsBoxList", vector_to_json(s.m_backAmsBoxList)},
        {"m_ids", vector_to_json(s.m_ids)}
    };
}

inline void from_json(const json& j, SendAmsInfoObj& s) {
    if (j.contains("m_backAmsBoxList")) s.m_backAmsBoxList = json_to_vector<AmsBoxObj>(j.at("m_backAmsBoxList"));
    if (j.contains("m_ids")) s.m_ids = j.at("m_ids").get<wxVector<int>>();
}

/* ---------------- CloudClientOpObj ---------------- */
inline void to_json(json& j, const CloudClientOpObj& c) {
    j = {
        {"type", c.type},
        {"objList", vector_to_json(c.objList)},
        {"deviceID", WxToUtf8(c.deviceID)},
        {"state", c.state},
        {"info", vector_to_json(c.info)},
        {"peripherie", element_to_json(c.peripherie)},
        {"boxId", c.boxId},
        {"newObj", element_to_json(c.newObj)},
        {"slotNum", c.slotNum}
    };
}

inline void from_json(const json& j, CloudClientOpObj& c) {
    c.type = j.value("type", 0);
    if (j.contains("objList")) c.objList = json_to_vector<PrinterObj>(j.at("objList"));
    if (j.contains("deviceID")) c.deviceID = Utf8ToWx(j.at("deviceID").get<std::string>());
    c.state = j.value("state", 0);
    if (j.contains("info")) c.info = json_to_vector<AmsBoxObj>(j.at("info"));
    if (j.contains("peripherie")) c.peripherie = json_to_element<Peripherie>(j.at("peripherie"));
    c.boxId = j.value("boxId", 0);
    if (j.contains("newObj")) c.newObj = json_to_element<AmsSlotObjInfo>(j.at("newObj"));
    c.slotNum = j.value("slotNum", 0);
}

/* ---------------- LANSendRemoteEventDataObjMap ---------------- */
inline void to_json(json& j, const LANSendRemoteEventDataObjMap& l) {
    j = {
        {"dryingStateObjMapStr", WxToUtf8(l.dryingStateObjMapStr)},
        {"dryingStateObjMapObj", element_to_json(l.dryingStateObjMapObj)}
    };
}

inline void from_json(const json& j, LANSendRemoteEventDataObjMap& l) {
    if (j.contains("dryingStateObjMapStr")) l.dryingStateObjMapStr = Utf8ToWx(j.at("dryingStateObjMapStr").get<std::string>());
    if (j.contains("dryingStateObjMapObj")) l.dryingStateObjMapObj = json_to_element<DryingStateObj>(j.at("dryingStateObjMapObj"));
}

/* ---------------- LANSendRemoteEventDataObj ---------------- */
inline void to_json(json& j, const LANSendRemoteEventDataObj& l) {
    j = {
        {"deviceID", WxToUtf8(l.deviceID)},
        {"printerAmsInfoList", vector_to_json(l.printerAmsInfoList)},
        {"dryingStateObjMap", vector_to_json(l.dryingStateObjMap)},
        {"functionsList", l.functionsList}
    };
}

inline void from_json(const json& j, LANSendRemoteEventDataObj& l) {
    if (j.contains("deviceID")) l.deviceID = Utf8ToWx(j.at("deviceID").get<std::string>());
    if (j.contains("printerAmsInfoList")) l.printerAmsInfoList = json_to_vector<PrinterSelectObj>(j.at("printerAmsInfoList"));
    if (j.contains("dryingStateObjMap")) l.dryingStateObjMap = json_to_vector<LANSendRemoteEventDataObjMap>(j.at("dryingStateObjMap"));
    if (j.contains("functionsList")) l.functionsList = j.at("functionsList").get<wxVector<int>>();
}

/* ---------------- FramGroupInfo ---------------- */
inline void to_json(json& j, const FramGroupInfo& f) {
    j = {
        {"printer_id", f.printer_id},
        {"g_isSelect", f.g_isSelect},
        {"g_deviceID", WxToUtf8(f.g_deviceID)},
        {"g_printerName", WxToUtf8(f.g_printerName)},
        {"g_state", WxToUtf8(f.g_state)},
        {"g_groupNum", WxToUtf8(f.g_groupNum)},
        {"g_modelName", WxToUtf8(f.g_modelName)},
        {"g_linkeType", WxToUtf8(f.g_linkeType)},
        {"g_slotInfoList", vector_to_json(f.g_slotInfoList)}
    };
}

inline void from_json(const json& j, FramGroupInfo& f) {
    f.printer_id = j.value("printer_id", -1);
    f.g_isSelect = j.value("g_isSelect", false);
    if (j.contains("g_deviceID")) f.g_deviceID = Utf8ToWx(j.at("g_deviceID").get<std::string>());
    if (j.contains("g_printerName")) f.g_printerName = Utf8ToWx(j.at("g_printerName").get<std::string>());
    if (j.contains("g_state")) f.g_state = Utf8ToWx(j.at("g_state").get<std::string>());
    if (j.contains("g_groupNum")) f.g_groupNum = Utf8ToWx(j.at("g_groupNum").get<std::string>());
    if (j.contains("g_modelName")) f.g_modelName = Utf8ToWx(j.at("g_modelName").get<std::string>());
    if (j.contains("g_linkeType")) f.g_linkeType = Utf8ToWx(j.at("g_linkeType").get<std::string>());
    if (j.contains("g_slotInfoList")) f.g_slotInfoList = json_to_vector<AmsSlotObjInfo>(j.at("g_slotInfoList"));
}

/* ---------------- FramGroupListInfo ---------------- */
inline void to_json(json& j, const FramGroupListInfo& f) {
    j = { {"g_framGroupList", vector_to_json(f.g_framGroupList)} };
}

inline void from_json(const json& j, FramGroupListInfo& f) {
    if (j.contains("g_framGroupList")) f.g_framGroupList = json_to_vector<FramGroupInfo>(j.at("g_framGroupList"));
}


/* ---------------- SendLanPrinterObj ---------------- */
inline void to_json(json& j, const SendLanPrinterObj& s) {
    j = {
        {"gcodeName", WxToUtf8(s.gcodeName)},
        {"filePath", WxToUtf8(s.filePath)},
        {"baseUrl", WxToUtf8(s.baseUrl)},
        {"deviceID", WxToUtf8(s.deviceID)}
    };
}

inline void from_json(const json& j, SendLanPrinterObj& s) {
    if (j.contains("gcodeName")) s.gcodeName = Utf8ToWx(j.at("gcodeName").get<std::string>());
    if (j.contains("filePath")) s.filePath = Utf8ToWx(j.at("filePath").get<std::string>());
    if (j.contains("baseUrl")) s.baseUrl = Utf8ToWx(j.at("baseUrl").get<std::string>());
    if (j.contains("deviceID")) s.deviceID = Utf8ToWx(j.at("deviceID").get<std::string>());
}

/* ---------------- SendFileObj ---------------- */
inline void to_json(json& j, const SendFileObj& s) {
    j = {
        {"time", WxToUtf8(s.time)},
        {"weight", WxToUtf8(s.weight)},
        {"layer", WxToUtf8(s.layer)},
        {"imgStr", WxToUtf8(s.imgStr)},
        {"printerName", WxToUtf8(s.printerName)},
        {"colorInfoList", vector_to_json(s.colorInfoList)}
    };
}

inline void from_json(const json& j, SendFileObj& s) {
    if (j.contains("time")) s.time = Utf8ToWx(j.at("time").get<std::string>());
    if (j.contains("weight")) s.weight = Utf8ToWx(j.at("weight").get<std::string>());
    if (j.contains("layer")) s.layer = Utf8ToWx(j.at("layer").get<std::string>());
    if (j.contains("imgStr")) s.imgStr = Utf8ToWx(j.at("imgStr").get<std::string>());
    if (j.contains("printerName")) s.printerName = Utf8ToWx(j.at("printerName").get<std::string>());
    if (j.contains("colorInfoList")) s.colorInfoList = json_to_vector<GcodeFilamentColor>(j.at("colorInfoList"));
}

/* ---------------- SendFileSumObjMap ---------------- */
inline void to_json(json& j, const SendFileSumObjMap& s) {
    j = {
        {"parteInfoMapInt", s.parteInfoMapInt},
        {"parteInfoMapObj", element_to_json(s.parteInfoMapObj)}
    };
}

inline void from_json(const json& j, SendFileSumObjMap& s) {
    s.parteInfoMapInt = j.value("parteInfoMapInt", 0);
    if (j.contains("parteInfoMapObj")) s.parteInfoMapObj = json_to_element< SendFileObj>(j.at("parteInfoMapObj"));
}

/* ---------------- SendFileSumObj ---------------- */
inline void to_json(json& j, const SendFileSumObj& s) {
    j = {
        {"timeSum", WxToUtf8(s.timeSum)},
        {"weightSum", WxToUtf8(s.weightSum)},
        {"parteSum", WxToUtf8(s.parteSum)},
        {"printerName", WxToUtf8(s.printerName)},
        {"imgStr", WxToUtf8(s.imgStr)},
        {"colorInfoListSum", vector_to_json(s.colorInfoListSum)},
        {"parteInfoMap", vector_to_json(s.parteInfoMap)}
    };
}

inline void from_json(const json& j, SendFileSumObj& s) {
    if (j.contains("timeSum")) s.timeSum = Utf8ToWx(j.at("timeSum").get<std::string>());
    if (j.contains("weightSum")) s.weightSum = Utf8ToWx(j.at("weightSum").get<std::string>());
    if (j.contains("parteSum")) s.parteSum = Utf8ToWx(j.at("parteSum").get<std::string>());
    if (j.contains("printerName")) s.printerName = Utf8ToWx(j.at("printerName").get<std::string>());
    if (j.contains("imgStr")) s.imgStr = Utf8ToWx(j.at("imgStr").get<std::string>());
    if (j.contains("colorInfoListSum")) s.colorInfoListSum = json_to_vector<GcodeFilamentColor>(j.at("colorInfoListSum"));
    if (j.contains("parteInfoMap")) s.parteInfoMap = json_to_vector<SendFileSumObjMap>(j.at("parteInfoMap"));
}

/* ---------------- NewVersionParObj ---------------- */
inline void to_json(json& j, const NewVersionParObj& n) {
    j = {
        {"id", n.id},
        {"versionCode", n.versionCode},
        {"draft", n.draft},
        {"del_flag", n.del_flag},
        {"param_version", WxToUtf8(n.param_version)},
        {"data_url", WxToUtf8(n.data_url)},
        {"update_desc_cn", WxToUtf8(n.update_desc_cn)},
        {"update_desc_en", WxToUtf8(n.update_desc_en)},
        {"create_time", WxToUtf8(n.create_time)},
        {"update_time", WxToUtf8(n.update_time)}
    };
}

inline void from_json(const json& j, NewVersionParObj& n) {
    n.id = j.value("id", 0);
    n.versionCode = j.value("versionCode", 0);
    n.draft = j.value("draft", 0);
    n.del_flag = j.value("del_flag", 0);
    if (j.contains("param_version")) n.param_version = Utf8ToWx(j.at("param_version").get<std::string>());
    if (j.contains("data_url")) n.data_url = Utf8ToWx(j.at("data_url").get<std::string>());
    if (j.contains("update_desc_cn")) n.update_desc_cn = Utf8ToWx(j.at("update_desc_cn").get<std::string>());
    if (j.contains("update_desc_en")) n.update_desc_en = Utf8ToWx(j.at("update_desc_en").get<std::string>());
    if (j.contains("create_time")) n.create_time = Utf8ToWx(j.at("create_time").get<std::string>());
    if (j.contains("update_time")) n.update_time = Utf8ToWx(j.at("update_time").get<std::string>());
}

/* ---------------- UpAndDownResultObj ---------------- */
inline void to_json(json& j, const UpAndDownResultObj& u) {
    j = {
        {"type", u.type},
        {"result", u.result},
        {"contentStr", WxToUtf8(u.contentStr)}
    };
}

inline void from_json(const json& j, UpAndDownResultObj& u) {
    u.type = j.value("type", 0);
    u.result = j.value("result", true);
    if (j.contains("contentStr")) u.contentStr = Utf8ToWx(j.at("contentStr").get<std::string>());
}

/* ---------------- ZipFolderObj ---------------- */
inline void to_json(json& j, const ZipFolderObj& z) {
    j = {
        {"folderPath", WxToUtf8(z.folderPath)},
        {"zipFilePath", WxToUtf8(z.zipFilePath)},
        {"isShowDialog", z.isShowDialog}
    };
}

inline void from_json(const json& j, ZipFolderObj& z) {
    if (j.contains("folderPath")) z.folderPath = Utf8ToWx(j.at("folderPath").get<std::string>());
    if (j.contains("zipFilePath")) z.zipFilePath = Utf8ToWx(j.at("zipFilePath").get<std::string>());
    z.isShowDialog = j.value("isShowDialog", false);
}

/* ---------------- UnZipFolderObj ---------------- */
inline void to_json(json& j, const UnZipFolderObj& u) {
    j = {
        {"zipFilePath", WxToUtf8(u.zipFilePath)},
        {"outputDir", WxToUtf8(u.outputDir)},
        {"isShowDialog", u.isShowDialog}
    };
}

inline void from_json(const json& j, UnZipFolderObj& u) {
    if (j.contains("zipFilePath")) u.zipFilePath = Utf8ToWx(j.at("zipFilePath").get<std::string>());
    if (j.contains("outputDir")) u.outputDir = Utf8ToWx(j.at("outputDir").get<std::string>());
    u.isShowDialog = j.value("isShowDialog", false);
}

/* ---------------- PostDownLoadObj ---------------- */
inline void to_json(json& j, const PostDownLoadObj& p) {
    j = {
        {"url", WxToUtf8(p.url)},
        {"filePath", WxToUtf8(p.filePath)},
        {"fileName", WxToUtf8(p.fileName)},
        {"md5", WxToUtf8(p.md5)},
        {"id", p.id}
    };
}

inline void from_json(const json& j, PostDownLoadObj& p) {
    if (j.contains("url")) p.url = Utf8ToWx(j.at("url").get<std::string>());
    if (j.contains("filePath")) p.filePath = Utf8ToWx(j.at("filePath").get<std::string>());
    if (j.contains("fileName")) p.fileName = Utf8ToWx(j.at("fileName").get<std::string>());
    if (j.contains("md5")) p.md5 = Utf8ToWx(j.at("md5").get<std::string>());
    p.id = j.value("id", 0);
}

/* ---------------- PostUpLoadObj ---------------- */
inline void to_json(json& j, const PostUpLoadObj& p) {
    j = {
        {"filePath", WxToUtf8(p.filePath)},
        {"fileName", WxToUtf8(p.fileName)},
        {"id", p.id},
        {"md5", WxToUtf8(p.md5)}
    };
}

inline void from_json(const json& j, PostUpLoadObj& p) {
    if (j.contains("filePath")) p.filePath = Utf8ToWx(j.at("filePath").get<std::string>());
    if (j.contains("fileName")) p.fileName = Utf8ToWx(j.at("fileName").get<std::string>());
    p.id = j.value("id", 0);
    if (j.contains("md5")) p.md5 = Utf8ToWx(j.at("md5").get<std::string>());
}



inline void to_json(json& j, const std::map<int, AmsSlotObj>& slotInfo) {
    j = json::object();
    for (const auto& [key, val] : slotInfo) {
        j[std::to_string(key)] = element_to_json(val);
    }
}

inline void from_json(const json& j, std::map<int, AmsSlotObj>& slotInfo) {
    slotInfo.clear();
    for (auto it = j.begin(); it != j.end(); ++it) {
        int key = std::stoi(it.key());
        AmsSlotObj val = json_to_element<AmsSlotObj>(it.value());
        slotInfo[key] = val;
    }
}






} // namespace Anycubic::Plugins
