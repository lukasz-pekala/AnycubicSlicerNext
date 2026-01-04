#include "RemotePrintingAmsDialog.hpp"

#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/font.h>
#include <wx/dcgraph.h>
#include <string>
#include <stdio.h>
#include "slic3r/GUI/wxExtensions.hpp"
#include "slic3r/GUI/MsgDialog.hpp"
#include "slic3r/GUI/MainFrame.hpp"
#include <wx/richtooltip.h>
#include "slic3r/GUI/Plater.hpp"
#include <plugins_sdk/event/detail/program_color.hxx>
#include <plugins_sdk/event/detail/util_tool.hxx>
#include "slic3r/GUI/GLCanvas3D.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

BEGIN_EVENT_TABLE(ColorBoxObject, StaticBox)

EVT_LEFT_DOWN(ColorBoxObject::mouseDown)
EVT_PAINT(ColorBoxObject::paintEvent)

EVT_ENTER_WINDOW(ColorBoxObject::mouseEnter)
EVT_LEAVE_WINDOW(ColorBoxObject::mouseLeave)

END_EVENT_TABLE()



BEGIN_EVENT_TABLE(ColorAMSBoxDialog, PopupWindow)

EVT_LEFT_DOWN(ColorAMSBoxDialog::mouseDown)
EVT_LEFT_UP(ColorAMSBoxDialog::mouseReleased)
EVT_MOUSE_CAPTURE_LOST(ColorAMSBoxDialog::mouseCaptureLost)

EVT_MOTION(ColorAMSBoxDialog::mouseMove)

END_EVENT_TABLE()





	
ColorAMSBoxDialog::ColorAMSBoxDialog(
    wxWindow* parent, wxWindow* amsParent, wxString& filament, int parentIndex, int slotNum, wxPoint& posPint, std::string deviceID)
    : PopupWindow(parent, wxPU_CONTAINS_CONTROLS)
    , m_parent(parent)
    , m_selectFilamentType(filament)
    , m_posPint(posPint)
    , m_deviceID(deviceID)
    , m_parentIndex(parentIndex)
    , m_amsParent(amsParent)
    , m_slotNum(slotNum)

{
    Init();
    
}

static wxColour Imvec4ToWxColour(ImVec4 color)
{
    int      r = std::min(255, std::max(0, int(color.x * 255.0f)));
    int      g = std::min(255, std::max(0, int(color.y * 255.0f)));
    int      b = std::min(255, std::max(0, int(color.z * 255.0f)));
    int      a = std::min(255, std::max(0, int(color.w * 255.0f)));
    wxColour colors(r, g, b, a);
    return colors;
}

void ColorAMSBoxDialog::Init()
{
    wxVector<AmsBoxObj>      infoList;
    wxVector<AmsSlotObjInfo> slotInfoList;

    for (int i = 0; i < infoList.size(); i++) {
        if (!infoList[i].isEnable)
            continue;
        for (int j = 0; j < infoList[i].slotInfo.size(); j++) {
            AmsSlotObjInfo  slotObj   = infoList[i].slotInfo[j];
            slotObj.slotNum           = slotObj.slotNum + (i * 4);
            slotInfoList.push_back(slotObj);
        }
    }

    this->SetBackgroundColour(COLOR_Neutral_01);
    this->SetForegroundColour(COLOR_Neutral_01);
    
    m_slotSum                   = slotInfoList.size();
    wxBoxSizer*  mainSizer      = new wxBoxSizer(wxVERTICAL);
    wxGridSizer* m_mainSizer    = new wxGridSizer(0, 4, 0, 0);
    wxBoxSizer*  m_warningSizer = new wxBoxSizer(wxVERTICAL);
    m_colorBoxList.clear();
    for (int i = 0; i < m_slotSum; i++) {
        
        ColorBoxObject* colorBox  = new ColorBoxObject(this, wxSize(FromDIP(74), FromDIP(74)), slotInfoList[i],i == m_slotNum - 1, m_deviceID);
        m_colorBoxList.push_back(colorBox);
        if (i == 0 || i == 4) {
            m_mainSizer->Add(colorBox, 0, wxBOTTOM, FromDIP(4));
        } else {
            m_mainSizer->Add(colorBox, 0,  wxBOTTOM|wxLEFT, FromDIP(4));
        }
    }
    
    Button* m_warningInfo = new Button(this, _L("Only the same filament can be selected"));

    StateColor report_bg;
    StateColor fgColor;
    m_warningInfo->SetCanFocus(false);
    m_warningInfo->SetPaddingSize(wxSize(0, 0));
    report_bg.append(COLOR_Neutral_01, StateColor::Normal);
    fgColor.append(COLOR_Red_01, StateColor::Normal);
    m_warningInfo->SetBackgroundColor(report_bg);
    m_warningInfo->SetTextColor(fgColor);
    m_warningInfo->SetBorderWidth(0);
    m_warningInfo->SetFont(Label::Body_14);
    
    this->Bind(wxEVT_PAINT, [this](wxPaintEvent& evt) {
        wxPaintDC dc(this);
        dc.SetPen(wxPen(wxColour(COLOR_Ams_dialog_borderColor)));
        dc.SetBrush(GetBackgroundColour());
        dc.DrawRectangle(wxPoint(0, 0), this->GetClientSize());
        evt.Skip();
    });
    this->SetPosition(wxPoint(m_posPint.x, m_posPint.y));
    mainSizer->Add(m_mainSizer, 0, wxALL, FromDIP(16));
    m_mainSizer->Layout();
    m_warningSizer->Add(m_warningInfo);
    m_warningSizer->Layout();
    mainSizer->Add(m_warningSizer, 1, wxEXPAND | wxLEFT, FromDIP(16));

    SetSizer(mainSizer);
#ifdef __APPLE__
    SetSize(GetWindowSize());
#else
    SetSize(GetWindowSize());
#endif // __APPLE__
    Layout();
    Refresh();
    this->Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent& evt) { 
        m_isInlcudeWin = false;
        evt.Skip(); 
        });
    this->Bind(wxEVT_ENTER_WINDOW, [this](wxMouseEvent& evt) { 
        m_isInlcudeWin = true;
        evt.Skip();
        });
#ifdef __WXOSX__
    // PopupWindow releases mouse on idle, which may cause various problems,
    //  such as losting mouse move, and dismissing soon on first LEFT_DOWN event.
    Bind(wxEVT_IDLE, [](wxIdleEvent& evt) {});
#endif
    wxGetApp().UpdateDarkUIWin(this);

}

void ColorAMSBoxDialog::SendEventToChild(const wxPoint& point, int eventType) 
{
    if(m_lastHoverBox){
        wxMouseEvent newEvt(eventType == 0 ? wxEVT_LEAVE_WINDOW : eventType == 1 ? wxEVT_ENTER_WINDOW : wxEVT_LEFT_DOWN);
        newEvt.SetPosition(m_lastHoverBox->ScreenToClient(point));
        wxPostEvent(m_lastHoverBox, newEvt);
    }else{
        for (ColorBoxObject* colorBox : m_colorBoxList) {
            wxRect rectScreen(colorBox->GetScreenPosition(), colorBox->GetSize());
            if (rectScreen.Contains(point)) {
                wxMouseEvent newEvt(eventType == 0 ? wxEVT_LEAVE_WINDOW : eventType == 1 ? wxEVT_ENTER_WINDOW : wxEVT_LEFT_DOWN);
                newEvt.SetPosition(colorBox->ScreenToClient(point));
                wxPostEvent(colorBox, newEvt);
                break;
            }
        }
    }
}

void ColorAMSBoxDialog::mouseMove(wxMouseEvent& evt)
{
#ifdef __APPLE__
    wxPoint         mousePosScreen = ClientToScreen(evt.GetPosition());
    ColorBoxObject* currentBox     = nullptr;

    for (ColorBoxObject* colorBox : m_colorBoxList) {
        wxRect rectScreen(colorBox->GetScreenPosition(), colorBox->GetSize());
        if (rectScreen.Contains(mousePosScreen)) {
            currentBox = colorBox;
            break;
        }
    }

    if (currentBox != m_lastHoverBox) {
        if (m_lastHoverBox) {
            wxMouseEvent leaveEvt(wxEVT_LEAVE_WINDOW);
            leaveEvt.SetPosition(m_lastHoverBox->ScreenToClient(mousePosScreen));
            wxPostEvent(m_lastHoverBox, leaveEvt);
        }

        if (currentBox) {
            wxMouseEvent enterEvt(wxEVT_ENTER_WINDOW);
            enterEvt.SetPosition(currentBox->ScreenToClient(mousePosScreen));
            wxPostEvent(currentBox, enterEvt);
        }
        m_lastHoverBox = currentBox;
    }

    Refresh();
#endif // __APPLE__
    evt.Skip(); 
    
}


void ColorAMSBoxDialog::mouseDown(wxMouseEvent& event)
{
    if (!IsShown())
        return;
    m_pressedDown = true;
    CaptureMouse();
#ifdef __APPLE__
    SendEventToChild(ClientToScreen(event.GetPosition()), 2);
#endif // __APPLE__

}

void ColorAMSBoxDialog::mouseReleased(wxMouseEvent& event)
{
    if (m_pressedDown) {
        m_pressedDown = false;
        if (HasCapture())
            ReleaseMouse();
        
    }
}

void ColorAMSBoxDialog::mouseCaptureLost(wxMouseCaptureLostEvent& event)
{
    wxMouseEvent evt(wxEVT_LEFT_UP);
    event.SetEventObject(this);
    GetEventHandler()->ProcessEvent(evt);
}


void ColorAMSBoxDialog::OnDismiss() 
{
    if (HasCapture())
        ReleaseMouse();

    dismissTime = boost::posix_time::microsec_clock::universal_time();
    wxPluginEvent e(EVT_AMS_POP_DISMISS);
    GetEventHandler()->ProcessEvent(e);
}

bool ColorAMSBoxDialog::HasDismissLongTime()
{
    auto now = boost::posix_time::microsec_clock::universal_time();
    return !IsShown() && (now - dismissTime).total_milliseconds() >= 2;
}


void ColorAMSBoxDialog::SetShowNewSlotInfo(wxWindow* amsParent, wxString& filamentType, int parentIndex, int slotNum, std::string deviceID)
{
    m_selectFilamentType = filamentType;
    m_deviceID           = deviceID;
    m_parentIndex = parentIndex;
    m_amsParent   = amsParent;
    m_slotNum     = slotNum;

    for (ColorBoxObject* colorBox : m_colorBoxList) 
    {
        colorBox->SetCheckState(false);
        if (slotNum != -1) {
            if (colorBox->GetSlotNum() == slotNum) {
                colorBox->SetCheckState(true);
            }
        } 

    }
}

wxSize ColorAMSBoxDialog::GetWindowSize()
{
    int winSize_W = 340;
    int conunt    = m_slotSum / 4;
    int winSize_H = 144 + (conunt > 1 ? (conunt - 1) * 74 : 0);
    return FromDIP(wxSize(winSize_W, winSize_H));
}

void ColorAMSBoxDialog::msw_rescale()
{
    wxSize _size = GetWindowSize();
    this->SetMinSize(_size);
    Fit();
    this->Layout();

    Refresh();
}


ColorBoxObject::ColorBoxObject(
    wxWindow* parent,
    wxSize winSize,
    AmsSlotObjInfo amsSlotBoj,
    bool isCheck,
    std::string deviceID):
    m_parent(parent)
    , m_winSize(winSize)
    , m_deviceID(deviceID)
    , m_isCheck(isCheck)
    , m_amsSlotObj(amsSlotBoj)
{
    Init();
}


void ColorBoxObject::Init()
{
    m_showSlotNum      = m_amsSlotObj.slotNum;

    m_showFilamentType = wxString::Format("%s", m_amsSlotObj.filament_type);

    m_showColor = m_amsSlotObj.filamentColorInfo;

    m_isEnable = m_showFilamentType != "-";

    StaticBox::Create(m_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    this->SetBorderWidth(0);
    this->SetBackgroundColor(COLOR_Neutral_01);
    double whiteGap  = calculateColorDifference_RGB(m_showColor, COLOR_Neutral_03);
    double blackGap  = calculateColorDifference_RGB(m_showColor, COLOR_Neutral_10);
    m_showColor_Text = whiteGap > 40 ? COLOR_Neutral_01 : COLOR_Neutral_10;

    m_check_ico = ScalableBitmap(this, "ico_remote_ams_check", 16).bmp();


    wxWindow::SetMinSize(m_winSize);
    Refresh();
}

void ColorBoxObject::SetCheckState(bool index) 
{ 

    m_isCheck = index;
    Refresh();
}

void ColorBoxObject::mouseLeave(wxMouseEvent& event) 
{ 
    m_isHover = false;
    Refresh();
    event.Skip();
}

void ColorBoxObject::mouseEnter(wxMouseEvent& event) 
{
    m_isHover = true;
    Refresh();
    event.Skip();
}

void ColorBoxObject::mouseDown(wxMouseEvent& event) 
{
    bool     isOpIndex  = false;
    wxString parentType = dynamic_cast<ColorAMSBoxDialog*>(m_parent)->GetFilamentType();
    if (!m_isCheck && m_isEnable && (parentType == m_showFilamentType || m_showFilamentType == "?")) {
        wxPluginEvent evt(EVT_REMOTE_AMS_CHANGE);
        evt.SetInt(m_showSlotNum);
        wxPostEvent(dynamic_cast<ColorAMSBoxDialog*>(m_parent)->GetAmsParentWin(), evt);
        m_parent->Hide();
        isOpIndex = true;
    } else if (m_isCheck && m_isEnable) {
        m_parent->Hide();
        isOpIndex = true;
    }
    if (isOpIndex) {
        wxPluginEvent e(EVT_AMS_POP_SELECT);
        wxPostEvent(m_parent, e);
    }

    event.Skip();
}

void ColorBoxObject::paintEvent(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    render(dc);
}


void ColorBoxObject::edgeColorRender(wxAutoBufferedPaintDC& dc, const wxPoint& point, const wxColour& colors, int type)
{

    wxColour edgeColor    = wxGetApp().dark_mode() ? COLOR_Neutral_01 : colors;
    int circle_big = FromDIP(31);
    int circle_small = FromDIP(9);

    dc.SetPen(wxPen(edgeColor, FromDIP(1)));
    dc.SetBrush(edgeColor);
    dc.DrawCircle(point, type == 1 ? circle_big : circle_small);


}


void ColorBoxObject::render(wxAutoBufferedPaintDC& dc) 
{
    StaticBox::render(dc);

    wxSize size = GetSize();
    wxRect rc(0, 0, size.x, size.y);

    wxColour parentColour = GetParentBackgroundColor(GetParent());
    dc.SetBackground(parentColour);

    int dipRadius = FromDIP(8);
    int circularSize = FromDIP(60);
    wxPoint circularPoint(rc.width / 2, rc.height / 2);
    
    dc.SetFont(Label::Body_15);

    dc.SetTextForeground(m_showColor_Text);

    wxString showSlotNum = wxString::Format("%d", m_showSlotNum);

    wxRect textSize_slot;
    dc.GetTextExtent(showSlotNum, &textSize_slot.width, &textSize_slot.height, &textSize_slot.x, &textSize_slot.y);

    wxRect textSize_type;
    dc.GetTextExtent(m_showFilamentType, &textSize_type.width, &textSize_type.height, &textSize_type.x, &textSize_type.y);

    int width_middle = rc.width / 2;

    wxPoint slotNumPoint(rc.x + (width_middle - (textSize_slot.width / 2)), FromDIP(8));
    
    wxPoint filamentTypePoint(rc.x + (width_middle - (textSize_type.width / 2)), rc.height - FromDIP(8) - textSize_type.height);
    int     circle_big = FromDIP(30);
    int     circle_small = FromDIP(8);
    wxColour edgeColor = wxGetApp().dark_mode() ? COLOR_Neutral_01 : m_showColor;


    if (!m_isEnable) {
        edgeColorRender(dc, circularPoint, m_showColor, 1);

        dc.SetBrush(COLOR_Neutral_07);
        dc.DrawCircle(circularPoint, circle_big);



        edgeColorRender(dc, circularPoint, m_showColor);
        dc.SetBrush(wxGetApp().dark_mode() ?COLOR_Neutral_13: COLOR_Neutral_01);
        dc.DrawCircle(circularPoint, circle_small);
        dc.SetTextForeground(COLOR_Neutral_01);

        dc.DrawText(m_showFilamentType, filamentTypePoint);
        dc.DrawText(showSlotNum, slotNumPoint);
        return;
    }
    int showColorType = m_amsSlotObj.iconType;

    if (m_isCheck) {

        dc.SetBrush(StateColor::darkModeColorFor(COLOR_Ams_check_color));
        dc.SetPen(StateColor::darkModeColorFor(COLOR_Ams_check_color));
        dc.DrawRoundedRectangle(rc, dipRadius);

        wxPoint checkIcoPoint(FromDIP(56), FromDIP(56));
        dc.DrawBitmap(m_check_ico, checkIcoPoint);


    } 
    if (m_isHover)  {

        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(COLOR_Blue_06);
        dc.DrawRoundedRectangle(rc, dipRadius);

    }
    bool isWhite = false;
    if (m_showColor.Red() == 255 && m_showColor.Green() == 255 && m_showColor.Blue() == 255) {
        isWhite = true;
    }
    if (wxGetApp().dark_mode() && isWhite) {
        dc.SetBrush(COLOR_Neutral_01);
        dc.SetPen(COLOR_Neutral_13);
        dc.DrawCircle(circularPoint, circle_big);

        dc.SetBrush(COLOR_Neutral_13);
        dc.SetPen(COLOR_Neutral_13);
        dc.DrawCircle(circularPoint, circle_small);

    } else {

        if (showColorType == 1 || showColorType == 2) {
            DrawGradientColorEvent(dc, circularPoint, m_amsSlotObj.skuColors, FromDIP(30), showColorType == 1 ? true : false);
            dc.SetTextForeground(COLOR_Neutral_01);
        } else {

            wxColour showColor = !isWhite ? m_showColor : parentColour;

            edgeColorRender(dc, circularPoint, showColor, 1);
            dc.SetBrush(showColor);
            
            dc.SetPen(showColorType == 3 ? wxPen(m_amsSlotObj.skuColors[1], FromDIP(3)) : wxPen(!isWhite ? m_showColor : COLOR_Ams_Box_borderColor));
            dc.DrawCircle(circularPoint, circle_big);
        }

        wxColour brush_small = showColorType != 0 ? COLOR_Neutral_01 :
                                                    !isWhite ?
                                                    wxGetApp().dark_mode() ? COLOR_Neutral_13:COLOR_Neutral_01 :
                                                    parentColour;
        wxColour penColor    = showColorType != 0 ? showColorType == 3 ? m_amsSlotObj.skuColors[1] : COLOR_Neutral_01 :
                               !isWhite           ? COLOR_Neutral_01 :  COLOR_Ams_Box_borderColor;
        edgeColorRender(dc, circularPoint, brush_small);

        dc.SetBrush(brush_small);
        if (showColorType != 0) {
            dc.SetPen(showColorType == 3 ? wxPen(penColor, FromDIP(3)) : wxPen(penColor));
        } else {
            dc.SetPen(penColor);
        }
        dc.DrawCircle(circularPoint, circle_small);
        
    }
    dc.DrawText(m_showFilamentType, filamentTypePoint);
    dc.DrawText(showSlotNum, slotNumPoint);
}



