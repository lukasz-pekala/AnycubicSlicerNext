#include "custom_struct_manger.hxx"


wxString from_u8(const std::string& str)
{
    return wxString::FromUTF8(str.c_str());
}

std::string into_u8(const wxString& str)
{
    auto buffer_utf8 = str.utf8_str();
    return std::string(buffer_utf8.data());
}




bool LANInfoObj::isQualInfo(LANInfoObj& obj)
{
    if (auto_leveling_support != obj.auto_leveling_support)
        return false;
    if (vibration_compensation_support != obj.vibration_compensation_support)
        return false;
    if (flow_calibration_support != obj.flow_calibration_support)
        return false;
    if (drying_first_support != obj.drying_first_support)
        return false;
    if (camera_timelapse != obj.camera_timelapse)
        return false;
    if (gcode_3mf_support != obj.gcode_3mf_support)
        return false;


    return true;
}



bool PrinterObj::isQualInfo(PrinterObj& obj)
{
    if (printer_Name != obj.printer_Name)
        return false;
    if (printer_Type != obj.printer_Type)
        return false;
    if (printer_id != obj.printer_id)
        return false;
    if (printer_State != obj.printer_State)
        return false;
    if (printer_IncludeAmsBox != obj.printer_IncludeAmsBox)
        return false;
    if (machine_type != obj.machine_type)
        return false;

    if (is_lan != obj.is_lan)
        return false;
    if (printer_type_str != obj.printer_type_str)
        return false;
    if (description != obj.description)
        return false;
    if (deviceID != obj.deviceID)
        return false;
    if (printer_Type_str != obj.printer_Type_str)
        return false;
    if (ip != obj.ip)
        return false;


    if (uuid != obj.uuid)
        return false;
    if (url != obj.url)
        return false;
    if (username != obj.username)
        return false;
    if (password != obj.password)
        return false;
    if (clientid != obj.clientid)
        return false;
    if (ca != obj.ca)
        return false;


    if (cert != obj.cert)
        return false;
    if (key != obj.key)
        return false;
    if (label_name != obj.label_name)
        return false;
    if (!version.isQualInfo(obj.version))
        return false;
    if (!peripherie.isQualInfo(obj.peripherie))
        return false;
    if (!options.isQualInfo(obj.options))
        return false;

    return true;


}




bool PrinterOptions::isQualInfo(PrinterOptions& obj)
{

    if (auto_leveling_support != obj.auto_leveling_support)
        return false;
    if (vibration_compensation_support != obj.vibration_compensation_support)
        return false;
    if (flow_calibration_support != obj.flow_calibration_support)
        return false;
    if (drying_first_support != obj.drying_first_support)
        return false;
    if (camera_timelapse_support != obj.camera_timelapse_support)
        return false;
    if (gcode_3mf_support != obj.gcode_3mf_support)
        return false;

    return true;

}

bool COTAVersion::isQualInfo(COTAVersion& obj)
{
    if (need_update != obj.need_update)
        return false;
    if (time_cost != obj.time_cost)
        return false;
    if (panding != obj.panding)
        return false;
    if (firmware_version != obj.firmware_version)
        return false;
    if (update_desc != obj.update_desc)
        return false;
    if (target_version != obj.target_version)
        return false;

    return true;

}




bool Peripherie::isQualInfo(Peripherie& obj)
{
    if (camera != obj.camera)
        return false;
    if (multiColorBox != obj.multiColorBox)
        return false;
    if (udisk != obj.udisk)
        return false;

    return true;
}




bool AmsSlotObjInfo::isQualInfo(AmsSlotObjInfo& obj)
{
    if (slotNum != obj.slotNum)
        return false;
    if (filament_type != obj.filament_type)
        return false;
    if (filamentColorInfo != obj.filamentColorInfo)
        return false;
    if (filament_type_sub != obj.filament_type_sub)
        return false;
    if (iconType != obj.iconType)
        return false;
    if (filament_type_sub != obj.filament_type_sub)
        return false;
    if (skuColors.size() != obj.skuColors.size())
        return false;
    int nowSize = skuColors.size();
    for (int i = 0; i < nowSize; i++) {
        if (skuColors[i] != obj.skuColors[i]) {
            return false;
        }
    }

    return true;
}

bool AmsBoxObj::isQualInfo(AmsBoxObj& obj)
{
    if (isEnable != obj.isEnable) {
        return false;
    }
    if (slotInfo.size() != obj.slotInfo.size())
        return false;

    int nowSize = slotInfo.size();
    for (int i = 0; i < nowSize; i++) {
        if (!slotInfo[i].isQualInfo(obj.slotInfo[i])) {
            return false;
        }
    }

    return true;

}

