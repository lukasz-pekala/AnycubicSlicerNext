#include "PrinterFunctionCheckBox.hpp"
#include "slic3r/GUI/Widgets/StateColor.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"
#include "slic3r/GUI/Widgets/StateHandler.hpp"
#include <plugins_sdk/event/detail/program_color.hxx>
#include <wx/mstream.h>
#include "slic3r/GUI/GUI.hpp"
#include <wx/dcgraph.h>
#include <wx/base64.h>


BEGIN_EVENT_TABLE(FunctionCheckBox, StaticBox)

EVT_LEFT_DOWN(FunctionCheckBox::mouseDown)
EVT_PAINT(FunctionCheckBox::paintEvent)
EVT_ENTER_WINDOW(FunctionCheckBox::mouseEnter)
EVT_LEAVE_WINDOW(FunctionCheckBox::mouseLeave)

END_EVENT_TABLE()



FunctionCheckBox::FunctionCheckBox(wxWindow* parent, wxString showText, bool isCheck, wxSize winSize, long style)
    : m_parent(parent), m_showText(showText), m_isCheck(isCheck)
    , m_winSize(winSize)
{
    init();
}

void FunctionCheckBox::init()
{
    StaticBox::Create(m_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    this->SetBackgroundColor(COLOR_Neutral_01);
    this->SetBorderWidth(0);

    wxString check_ico_enable          = "ico_remote_check_enable";
    wxString check_ico_diasable  = "ico_remote_check_disable";

    m_CheckIco_enable  = ScalableBitmap(this, check_ico_enable.ToStdString(), 16).bmp();
    m_CheckIco_disable = ScalableBitmap(this, check_ico_diasable.ToStdString(), 16).bmp();

    m_nowCheckIco = m_isCheck ? m_CheckIco_enable : m_CheckIco_disable;

    wxWindow::SetMinSize(m_winSize);
    Refresh();
}


void FunctionCheckBox::paintEvent(wxPaintEvent& evt)
{
    wxPaintDC dc(this);
    render(dc);
}

void FunctionCheckBox::render(wxDC& dc)
{
    // background
    StaticBox::render(dc);
    dc.SetFont(Label::Body_13);
    wxSize size = GetSize();

    wxRect   rc(0, 0, size.x, size.y);
    wxColour parentColour   = GetParentBackgroundColor(GetParent());
    dc.SetBackground(parentColour);

    int dipRadius = FromDIP(6);

    dc.SetBrush(GetBackgroundColour());
    dc.SetPen(m_isCheck || m_isHover ? COLOR_Blue_06 : COLOR_Ams_Check_borderColor);
    dc.DrawRoundedRectangle(rc, dipRadius);


    wxPoint checkIcoPoint(FromDIP(8), FromDIP(6));
    dc.DrawBitmap(m_nowCheckIco,checkIcoPoint);


    wxRect textSize_type;
    dc.GetTextExtent(m_showText, &textSize_type.width, &textSize_type.height, &textSize_type.x, &textSize_type.y);

    dc.SetTextForeground(m_isCheck ? COLOR_Blue_06 : COLOR_Remote_gray);
    wxPoint textPoint(checkIcoPoint.x + FromDIP(24), rc.y + ((rc.height / 2) - (textSize_type.height / 2)));
    dc.DrawText(m_showText, textPoint);
}

void FunctionCheckBox::SetCheckState() 
{
    m_isCheck     = !m_isCheck;
    m_nowCheckIco = m_isCheck ? m_CheckIco_enable : m_CheckIco_disable;
    Refresh();
}

void FunctionCheckBox::mouseDown(wxMouseEvent& event)
{
    m_isCheck = !m_isCheck;
    m_nowCheckIco = m_isCheck ? m_CheckIco_enable : m_CheckIco_disable;
    Refresh();
    event.Skip();
}

void FunctionCheckBox::mouseLeave(wxMouseEvent& event)
{
    m_isHover = false;
    Refresh();
    event.Skip();
}

void FunctionCheckBox::mouseEnter(wxMouseEvent& event)
{
    m_isHover = true;
    Refresh();
    event.Skip();
}
