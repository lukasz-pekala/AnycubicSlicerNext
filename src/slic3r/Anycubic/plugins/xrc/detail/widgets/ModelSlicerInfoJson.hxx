#pragma once

#include <wx/string.h>
#include <wx/colour.h>
#include <wx/vector.h>
#include <map>
#include <nlohmann/json.hpp>
#include "custom_struct_manger.hxx"

using json = nlohmann::json;

/* ---------------- wxString helpers ---------------- */

inline std::string WxToUtf8(const wxString& s) { return std::string(s.utf8_string()); }

inline wxString Utf8ToWx(const std::string& s) { return wxString::FromUTF8(s.c_str()); }

/* ---------------- wxColour JSON ---------------- */

inline void to_json(json& j, const wxColour& c) { j = json{{"r", c.Red()}, {"g", c.Green()}, {"b", c.Blue()}, {"a", c.Alpha()}}; }

inline void from_json(const json& j, wxColour& c)
{
    int r = j.value("r", 0);
    int g = j.value("g", 0);
    int b = j.value("b", 0);
    int a = j.value("a", 255);
    c.Set(r, g, b, a);
}



/* ============================================================
   ================= JSON adapters =============================
   ============================================================ */

/* ---- GcodeFilamentColor ---- */

inline void to_json(json& j, const GcodeFilamentColor& f)
{
    j = json{{"num", f.num},
             {"filament", WxToUtf8(f.filament)},
             {"filamentColor", f.filamentColor},
             {"sloop", f.sloop},
             {"weight", WxToUtf8(f.weight)}};
}

inline void from_json(const json& j, GcodeFilamentColor& f)
{
    f.num = j.value("num", 0);

    if (j.contains("filament"))
        f.filament = Utf8ToWx(j.at("filament").get<std::string>());

    if (j.contains("filamentColor"))
        f.filamentColor = j.at("filamentColor").get<wxColour>();

    f.sloop = j.value("sloop", 0);

    if (j.contains("weight"))
        f.weight = Utf8ToWx(j.at("weight").get<std::string>());
}

/* ---- CloudSlicerInfoObj_FilamentColor ---- */

inline void to_json(json& j, const CloudSlicerInfoObj_FilamentColor& f)
{
    j = json{{"material_type", WxToUtf8(f.material_type)}, {"color", f.color}};
}

inline void from_json(const json& j, CloudSlicerInfoObj_FilamentColor& f)
{
    if (j.contains("material_type"))
        f.material_type = Utf8ToWx(j.at("material_type").get<std::string>());

    if (j.contains("color"))
        f.color = j.at("color").get<wxVector<int>>();
}

/* ---- ModelSlicerInfo ---- */

inline void to_json(json& j, const ModelSlicerInfo& m)
{
    j = json{{"fileID", m.fileID},
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
             {"filament_color", m.filament_color},
             {"machine_type", m.machine_type},
             {"printerName", WxToUtf8(m.printerName)},
             {"isGcode", m.isGcode},
             {"extruder_idsList", m.extruder_idsList},
             {"filamentColors_slicer", m.filamentColors_slicer},
             {"filamentTypess_slicer", m.filamentTypess_slicer},
             {"filament_types_gcode", m.filament_types_gcode},
             {"extruder_colors_gcode", m.extruder_colors_gcode},
             {"gcodeFilamentColorList", m.gcodeFilamentColorList},
             {"filamentSlicerList", m.filamentSlicerList}};
}

inline void from_json(const json& j, ModelSlicerInfo& m)
{
    m.fileID       = j.value("fileID", -1);
    m.isCloudStart = j.value("isCloudStart", false);

    if (j.contains("gcodeName"))
        m.gcodeName = Utf8ToWx(j.at("gcodeName").get<std::string>());
    if (j.contains("imgBase64"))
        m.imgBase64 = Utf8ToWx(j.at("imgBase64").get<std::string>());
    if (j.contains("print_time"))
        m.print_time = Utf8ToWx(j.at("print_time").get<std::string>());
    if (j.contains("used_filament"))
        m.used_filament = Utf8ToWx(j.at("used_filament").get<std::string>());
    if (j.contains("modleLayers"))
        m.modleLayers = Utf8ToWx(j.at("modleLayers").get<std::string>());
    if (j.contains("filament_length"))
        m.filament_length = Utf8ToWx(j.at("filament_length").get<std::string>());
    if (j.contains("filament_type"))
        m.filament_type = Utf8ToWx(j.at("filament_type").get<std::string>());
    if (j.contains("printerName"))
        m.printerName = Utf8ToWx(j.at("printerName").get<std::string>());

    m.filamentWeight_d = j.value("filamentWeight_d", 0.0);
    m.print_time_f     = j.value("print_time_f", 0.0f);
    m.machine_type     = j.value("machine_type", 0);
    m.isGcode          = j.value("isGcode", false);

    if (j.contains("filament_color"))
        m.filament_color = j.at("filament_color").get<wxVector<int>>();
    if (j.contains("extruder_idsList"))
        m.extruder_idsList = j.at("extruder_idsList").get<wxVector<unsigned int>>();
    if (j.contains("filamentColors_slicer"))
        m.filamentColors_slicer = j.at("filamentColors_slicer").get<wxVector<wxString>>();
    if (j.contains("filamentTypess_slicer"))
        m.filamentTypess_slicer = j.at("filamentTypess_slicer").get<wxVector<wxString>>();
    if (j.contains("filament_types_gcode"))
        m.filament_types_gcode = j.at("filament_types_gcode").get<wxVector<wxString>>();
    if (j.contains("extruder_colors_gcode"))
        m.extruder_colors_gcode = j.at("extruder_colors_gcode").get<wxVector<wxString>>();
    if (j.contains("gcodeFilamentColorList"))
        m.gcodeFilamentColorList = j.at("gcodeFilamentColorList").get<wxVector<GcodeFilamentColor>>();
    if (j.contains("filamentSlicerList"))
        m.filamentSlicerList = j.at("filamentSlicerList").get<wxVector<CloudSlicerInfoObj_FilamentColor>>();
}
