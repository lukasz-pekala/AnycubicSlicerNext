#include "AmsButton.hpp"
#include "StateColor.hpp"
#include "Label.hpp"
#include "StateHandler.hpp"
#include <wx/mstream.h>
#include <wx/dcgraph.h>
#include <wx/base64.h>
#include <plugins_sdk/event/detail/program_color.hxx>
#include <plugins_sdk/event/detail/util_tool.hxx>
#include "RemotePrintingAmsDialog.hpp"


BEGIN_EVENT_TABLE(AmsButton, StaticBox)

EVT_LEFT_DOWN(AmsButton::mouseDown)
EVT_PAINT(AmsButton::paintEvent)

END_EVENT_TABLE()



AmsButton::AmsButton(wxWindow* parent,
                     bool      initIndex, wxString filamentType,
                     wxSize    winSize, wxColour typeColor, 
                     int               countIndex,
                     std::string printerType,
                     std::string nowDeviceID,
                     int         slotNum, 
                     wxColour slotColor,long      style): 
    m_parent(parent),
    m_isInit(initIndex), 
    m_filamentType(filamentType), 
    m_winSize(winSize),
    m_slotNum(slotNum), 
    m_typeColor(typeColor), 
    m_slotColor(slotColor), 
    m_countIndex(countIndex), 
    m_nowDeviceID(nowDeviceID),
    m_printerType(printerType)
{
    init();
}

void AmsButton::OnChangeAutoEvent(wxPluginEvent& event)
{

    if (m_amsPop != nullptr) {
        m_downClick = false;
        m_amsPop->Close();
        delete m_amsPop;
        m_amsPop = nullptr;
    }
    m_nowDeviceID = event.GetString().ToStdString();
    int idType = event.GetId();
    int showSlotNum = event.GetInt();
    if (idType == -1 || showSlotNum == -1) {
        SetSlotIsEmpty();
        Refresh();
        return;
    }
    SendAmsInfoObj* d = static_cast<SendAmsInfoObj*>(event.GetSharedData());
    SetSlotShowNumber(showSlotNum);

    wxVector<AmsBoxObj>         infoList = d->m_backAmsBoxList;
    wxVector<AmsSlotObjInfo> slotInfoList;

    for (int i = 0; i < infoList.size(); i++) {
        if (!infoList[i].isEnable)
            continue;
        for (int j = 0; j < infoList[i].slotInfo.size(); j++) {
            AmsSlotObjInfo slotObj = infoList[i].slotInfo[j];
            slotObj.slotNum                     = slotObj.slotNum + (i * 4);
            slotInfoList.push_back(slotObj);
        }
    }
    int newIndex = showSlotNum - 1;
    if (slotInfoList.size() > newIndex) {
        SetSlotShowColor(slotInfoList[newIndex].filamentColorInfo);

        wxPluginEvent evt(EVT_AMS_INFO_CHANGE_EVENT);
        PrinterSelectObj* selectObj = new PrinterSelectObj();
        selectObj->filamentNum      = m_countIndex;
        selectObj->slotNum          = showSlotNum;
        selectObj->slotColor        = slotInfoList[newIndex].filamentColorInfo;
        selectObj->slotFilament     = slotInfoList[newIndex].filament_type;
        evt.SetSharedData(selectObj, nullptr, [](void*, void* a) { delete (PrinterSelectObj*) a; });
        wxPostEvent(m_parent, evt);

    }
    Refresh();

}
void AmsButton::OnChangeEvent(wxPluginEvent& event)
{
    int showSlotNum = event.GetInt();
    SetSlotShowNumber(showSlotNum);


    wxVector<AmsBoxObj>      infoList = m_plugin->GetPrinterKeyAmsInfoMap(m_nowDeviceID);
    wxVector<AmsSlotObjInfo> slotInfoList;
    
    for (int i = 0; i < infoList.size(); i++) {
        if (!infoList[i].isEnable)
            continue;
        for (int j = 0; j < infoList[i].slotInfo.size(); j++) {
            AmsSlotObjInfo slotObj = infoList[i].slotInfo[j];
            slotObj.slotNum        = slotObj.slotNum + (i * 4);
            slotInfoList.push_back(slotObj);
        }
    }
    if (slotInfoList.size() == 0)
        return;
    SetSlotShowColor(slotInfoList[showSlotNum - 1].filamentColorInfo);


    wxPluginEvent     evt_ams(EVT_AMS_INFO_CHANGE_EVENT);
    PrinterSelectObj* selectObj = new PrinterSelectObj();
    selectObj->filamentNum      = m_countIndex;
    selectObj->slotNum          = showSlotNum;
    selectObj->slotColor        = slotInfoList[showSlotNum - 1].filamentColorInfo;
    selectObj->slotFilament     = slotInfoList[showSlotNum - 1].filament_type;
    evt_ams.SetSharedData(selectObj, nullptr, [](void*, void* a) { delete (PrinterSelectObj*) a; });
    wxPostEvent(m_parent, evt_ams);

    m_plugin->SendFilamentSlotChangeEvent();

    OpAmsChange amsChangeObj;
    amsChangeObj.deviceID    = wxString(m_nowDeviceID);
    amsChangeObj.slotNum         = showSlotNum;
    amsChangeObj.countIndex      = m_countIndex;
    amsChangeObj.slotFilament    = slotInfoList[showSlotNum - 1].filament_type;

    m_plugin->SendOpAmsChange(amsChangeObj);
}

void AmsButton::init()
{
    StaticBox::Create(m_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    this->SetBackgroundColor(COLOR_Neutral_01);
    this->SetCornerRadius(6.0f);
    //this->SetColor2AndRadiusIndex(true);

    double whiteGap  = calculateColorDifference_RGB(m_typeColor, COLOR_Neutral_03);
    double blackGap  = calculateColorDifference_RGB(m_typeColor, COLOR_Neutral_10);
    m_typeColor_text = blackGap > whiteGap ? COLOR_Neutral_10 : COLOR_Neutral_01;


    m_white_down_ico = Plucgin_ScalableBitmap(this, "ico_remote_down_white", 12).bmp();
    m_gray_down_ico  = Plucgin_ScalableBitmap(this, "ico_remote_down_gray", 12).bmp();
    m_icoSize = wxSize(FromDIP(12), FromDIP(12));
    wxWindow::SetMinSize(m_winSize);
    Refresh();
    if (m_slotNum == -1) {
        SetSlotIsEmpty();
    } else {
        SetSlotShowColor(m_slotColor);
    }

    wxVector<AmsBoxObj>      infoList;
    wxVector<AmsSlotObjInfo> slotInfoList;

    for (int i = 0; i < infoList.size(); i++) {
        if (!infoList[i].isEnable)
            continue;
        for (int j = 0; j < infoList[i].slotInfo.size(); j++) {
            AmsSlotObjInfo slotObj = infoList[i].slotInfo[j];
            slotObj.slotNum                     = slotObj.slotNum + (i * 4);
            m_amsSlotObjMap[slotObj.slotNum] = slotObj;
        }
    }

    this->Bind(EVT_REMOTE_AMS_CHANGE, &AmsButton::OnChangeEvent, this);
    this->Bind(EVT_REMOTE_AMS_CHANGE_SEND, &AmsButton::OnChangeAutoEvent, this);
}

void AmsButton::SetSlotShowColor(wxColour slotColor) {

    m_slotColor = slotColor;
    double whiteGap = calculateColorDifference_RGB(m_slotColor, COLOR_Neutral_03);
    double blackGap = calculateColorDifference_RGB(m_slotColor, COLOR_Neutral_10);

    m_now_down_ico = whiteGap > 40 ? m_white_down_ico : m_gray_down_ico;
    m_slotColor_text = blackGap > whiteGap ? COLOR_Neutral_10 : COLOR_Neutral_01;

    Refresh();
}


void AmsButton::SetSlotShowNumber(int showNum) 
{
    m_slotNum = showNum;
    m_isEmpty = false;
}

void AmsButton::SetSlotIsEmpty() 
{
    m_isEmpty = true;
    m_slotColor = COLOR_Neutral_03;
    m_slotNum   = -1;
    m_slotColor_text = COLOR_Text_Gray;
}

void AmsButton::paintEvent(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    render(dc);
}

void AmsButton::render(wxAutoBufferedPaintDC& dc)
{
    // background
    StaticBox::render(dc);

    wxSize   size = GetSize();
    wxRect   rc(0, 0, size.x, size.y);
    int      rectHeightSize = rc.height / 2;
    wxColour parentColour = GetParentBackgroundColor(GetParent());
    dc.SetBackground(parentColour);
    int dipRadius = FromDIP(6);

    bool upIsWhite = false;
    if (m_typeColor.Red() == 255 && m_typeColor.Green() == 255 && m_typeColor.Blue() == 255) {
        upIsWhite = true;
    }

    dc.SetBrush(!upIsWhite ? m_typeColor : COLOR_Ams_White_Change_Color);
    dc.SetPen(*wxTRANSPARENT_PEN);
    int    up_rectHeightSize = rectHeightSize - 1;
    wxRect up_rc(0, 0, rc.width, up_rectHeightSize);
    dc.DrawRoundedRectangle(up_rc, dipRadius);

    bool downIsWhite = false;
    if (m_slotColor.Red() == 255 && m_slotColor.Green() == 255 && m_slotColor.Blue() == 255) {
        downIsWhite = true;
    }

    wxRect down_rc(0, rectHeightSize, rc.width, rectHeightSize);
    m_downRect = down_rc;
    bool isDrawIndex = false;

    int showColorType = m_amsSlotObjMap[m_slotNum].iconType;

    if (showColorType == 1 || showColorType == 2) {
        if (showColorType == 1) {
            DrawRoundedRectangleColorEvent(dc, down_rc, m_amsSlotObjMap[m_slotNum].skuColors, dipRadius, false);
        } else {
            DrawRoundedRectangleColorVerticalEvent(dc, down_rc, m_amsSlotObjMap[m_slotNum].skuColors, dipRadius, false);
        }
        isDrawIndex = true;
    } else {
        dc.SetBrush(!downIsWhite ? m_slotColor : COLOR_Ams_White_Change_Color);
        dc.DrawRoundedRectangle(down_rc, dipRadius);
        if (showColorType == 3) {
            isDrawIndex           = true;
            wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
            if (gc) {
                wxGraphicsPath path = gc->CreatePath();

                double x = down_rc.x+1;
                double y = down_rc.y+1;
                double w = down_rc.width - 2;
                double h = down_rc.height - 2;
                double r = dipRadius;
                path.MoveToPoint(x, y);
                path.AddLineToPoint(x + w, y);
                path.AddLineToPoint(x + w, y + h - r);
                path.AddArcToPoint(x + w, y + h, x + w - r, y + h, r);
                path.AddLineToPoint(x + r, y + h);
                path.AddArcToPoint(x, y + h, x, y + h - r, r);
                path.AddLineToPoint(x, y);
                path.CloseSubpath();
                gc->SetBrush(*wxTRANSPARENT_BRUSH);
                gc->SetPen(wxPen(m_amsSlotObjMap[m_slotNum].skuColors[1], 3));
                gc->StrokePath(path);

                delete gc;
            }
        }
    }


    int dipBoardWidth = border_width == 0 ? 0 : FromDIP(border_width);
    

    dc.SetBrush(!upIsWhite ? m_typeColor : COLOR_Ams_White_Change_Color);
    dc.SetFont(Label::Body_14);
    dc.SetPen(wxPen(!upIsWhite ? m_typeColor : COLOR_Ams_White_Change_Color, dipBoardWidth));
    


    wxPointList pt_list_1;
    pt_list_1.push_back(new wxPoint(0, up_rectHeightSize - dipRadius - 1));
    pt_list_1.push_back(new wxPoint(0, up_rectHeightSize - 1));
    pt_list_1.push_back(new wxPoint(dipRadius, up_rectHeightSize - 1));
    dc.DrawPolygon(&pt_list_1, 0, 0);

    wxPointList pt_list_2;
    pt_list_2.push_back(new wxPoint(rc.width, up_rectHeightSize - dipRadius - 1));
    pt_list_2.push_back(new wxPoint(rc.width, up_rectHeightSize - 1));
    pt_list_2.push_back(new wxPoint(rc.width - dipRadius, up_rectHeightSize - 1));
    dc.DrawPolygon(&pt_list_2, 0, 0);


    dc.SetBrush(!downIsWhite ? m_slotColor : COLOR_Ams_White_Change_Color);
    dc.SetPen(wxPen(!downIsWhite ? m_slotColor : COLOR_Ams_White_Change_Color, dipBoardWidth));
    int start_rectPos_y = rc.y + rectHeightSize;

    wxPointList pt_list_3;
    pt_list_3.push_back(new wxPoint(0, rectHeightSize + dipRadius));
    pt_list_3.push_back(new wxPoint(0, rectHeightSize));
    pt_list_3.push_back(new wxPoint(dipRadius, rectHeightSize));
    if (!isDrawIndex)
        dc.DrawPolygon(&pt_list_3, 0, 0);

    wxPointList pt_list_4;
    pt_list_4.push_back(new wxPoint(rc.width, rectHeightSize + dipRadius));
    pt_list_4.push_back(new wxPoint(rc.width, rectHeightSize));
    pt_list_4.push_back(new wxPoint(rc.width - dipRadius, rectHeightSize));
    if (!isDrawIndex)
        dc.DrawPolygon(&pt_list_4, 0, 0);

    wxString slotStr = m_slotNum == -1 ? wxString("-") : wxString::Format("%d", m_slotNum);

    wxRect textSize;
    dc.GetTextExtent(slotStr, &textSize.width, &textSize.height, &textSize.x, &textSize.y);

    wxRect textSize_type;
    dc.GetTextExtent(m_filamentType, &textSize_type.width, &textSize_type.height, &textSize_type.x, &textSize_type.y);

    dc.SetTextForeground(m_typeColor_text);
    wxPoint type_textPoint(up_rc.x + ((up_rc.width / 2) - (textSize_type.width / 2)),
                           up_rc.y + ((up_rc.height / 2) - (textSize_type.height / 2)));
    dc.DrawText(m_filamentType, type_textPoint);

    dc.SetTextForeground((showColorType == 1 || showColorType==2) ? COLOR_Neutral_01 : m_slotColor_text);

    if (m_isEmpty) {

        wxPoint textPoint(down_rc.x + ((down_rc.width / 2) - (textSize.width / 2)), down_rc.y + ((down_rc.height / 2) - (textSize.height / 2)));
        
        dc.DrawText(slotStr, textPoint);

    } else {
        int ico_text_gap = FromDIP(4);
        int sum_width    = m_icoSize.x + ico_text_gap + textSize.x;
        int start_x   = down_rc.x + ((down_rc.width / 2) - (sum_width / 2));

        int start_y_ico  = down_rc.y + ((down_rc.height / 2) - (m_icoSize.y / 2));
        int start_y_text = down_rc.y + ((down_rc.height / 2) - (textSize.height / 2));

        wxPoint downIcoPoint(start_x, start_y_ico);

        wxPoint textPoint(start_x + m_icoSize.x + ico_text_gap, start_y_text);


        dc.DrawBitmap(m_now_down_ico, downIcoPoint);
        dc.DrawText(slotStr, textPoint);
    }
    
}

void AmsButton::SetShowFilamentDialog(bool index) 
{
    if (index) {
        wxPoint                         now_show_winPos = this->GetScreenPosition() + wxPoint(0, FromDIP(8) + m_winSize.y);
        if (m_amsPop == nullptr) {
            m_amsPop = new ColorAMSBoxDialog(m_plugin,this, this, m_filamentType, m_countIndex, m_slotNum, now_show_winPos, m_nowDeviceID);
            m_amsPop->SetPosition(now_show_winPos);
            m_amsPop->Bind(EVT_AMS_POP_DISMISS, [this](auto&) {
                m_downClick = false;
            });
            m_amsPop->Bind(EVT_AMS_POP_SELECT, [this](auto&) { 
                m_downClick = false; 
                m_amsPop->Close();
                if (m_amsPop)
                {
                    m_amsPop->Destroy();
                    m_amsPop = nullptr;
                }

            });
        }
        if (m_downClick) {
            m_amsPop->Hide();
            m_amsPop->Close();
            m_downClick = false;
            delete m_amsPop;
            m_amsPop = nullptr;
        } else {
            if (m_amsPop->HasDismissLongTime()) {
                m_amsPop->SetPosition(now_show_winPos);
                m_downClick = true;
                m_amsPop->Popup(m_amsPop);
            } else {
                m_amsPop->Hide();
                m_amsPop->Close();
                m_downClick = false;
                delete m_amsPop;
                m_amsPop = nullptr;
            }
        }

    } 
}


void AmsButton::mouseDown(wxMouseEvent& event)
{
    wxPoint now_mousePos =  event.GetPosition();
    if (m_downRect.Contains(now_mousePos) /*&& !m_isEmpty*/ && !m_nowDeviceID.empty()) {
        wxVector<PrinterObj>   printerInfo = m_plugin->GetPrinterList(m_printerType);
        wxVector<AmsBoxObj>     amsBoxLists = m_plugin->GetPrinterKeyAmsInfoMap(m_nowDeviceID);
        bool                                 isInclude     = false;
        for (PrinterObj obj : printerInfo) {
            if (obj.deviceID == m_nowDeviceID && obj.machine_type >= 3 && !amsBoxLists.empty()) {
                isInclude = true;
            }
        }
        int i = isInclude ? 1 : 0;
        if (isInclude) {
            SetShowFilamentDialog(true);
        }
    }
    //event.Skip();
}
