#include "FilamentColorAndTypeObj.hpp"

#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/font.h>
#include <wx/dcgraph.h>
#include <string>
#include <stdio.h>
#include <plugins_sdk/event/detail/program_color.hxx>
#include <plugins_sdk/event/detail/util_tool.hxx>
#include "RemoteSlotColorPanel.hpp"



FilamentShowPopPanel::FilamentShowPopPanel(wxWindow*       parent,
                                           const wxColour& filamentColor,
                                           const wxString&  filamentStr,
                                           const wxString& filamentWeight)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
    , m_filamentColor(filamentColor)
    , m_filamentStr(filamentStr)
    , m_filamentWeight(filamentWeight)
{
    m_showTextColor  = getTextFitColor(filamentColor);
    m_filamentGapStr = _L("|");
    SetBackgroundColour(COLOR_Neutral_01);
    m_gapIndex_1 = FromDIP(10);
    m_gapIndex_2 = FromDIP(7);
    
    wxSize filamentStrSize    = GetTextExtent(m_filamentStr);
    wxSize filamentGapSize    = GetTextExtent(m_filamentGapStr);
    wxSize filamentWeightSize = GetTextExtent(m_filamentWeight);


    SetMinSize(wxSize(filamentStrSize.x + filamentGapSize.x+ filamentWeightSize.x + (m_gapIndex_1 * 2) + (m_gapIndex_2 * 2),std::max(filamentStrSize.y, filamentWeightSize.y) + FromDIP(4)));
    Bind(wxEVT_PAINT, &FilamentShowPopPanel::OnPaint, this);
    Refresh();
}

void FilamentShowPopPanel::OnPaint(wxPaintEvent&)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    wxSize size = GetSize();

    wxSize filamentStrSize    = dc.GetTextExtent(m_filamentStr);
    wxSize filamentGapSize    = dc.GetTextExtent(m_filamentGapStr);
    wxSize filamentWeightSize = dc.GetTextExtent(m_filamentWeight);

    int sum_x = filamentStrSize.x + filamentGapSize.x + filamentWeightSize.x + (2 * m_gapIndex_2);

    wxPoint filamentStrPt((size.x - sum_x) / 2, (size.y - filamentStrSize.y) / 2);

    wxPoint filamentGapPt(filamentStrPt.x + filamentStrSize.x + m_gapIndex_2, (size.y - filamentGapSize.y) / 2);

    wxPoint filamentWeightPt(filamentGapPt.x + filamentGapSize.x + m_gapIndex_2, (size.y - filamentWeightSize.y) / 2);


    wxRect rect(0, 0, size.x, size.y);

    dc.SetBrush(wxBrush(m_filamentColor));
    dc.SetPen(wxPen(StateColor::darkModeColorFor(COLOR_Ams_Box_borderColor), 1));

    dc.DrawRoundedRectangle(rect, FromDIP(4));


    dc.SetFont(Label::Body_13);
    dc.SetPen(m_showTextColor);
    dc.SetTextForeground(m_showTextColor);
    dc.DrawText(m_filamentStr, filamentStrPt);
    dc.DrawText(m_filamentGapStr, filamentGapPt);
    dc.DrawText(m_filamentWeight, filamentWeightPt);




    
}




FilamentShowPanel::FilamentShowPanel(wxWindow* parent, const wxColour& filamentColor, const wxString& filamentStr)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), m_filamentColor(filamentColor), m_filamentStr(filamentStr)
{
    SetBackgroundColour(COLOR_Neutral_01);
    Bind(wxEVT_PAINT, &FilamentShowPanel::OnPaint, this);
    SetMinSize(FromDIP(wxSize(50,22)));
    Refresh();
}

void FilamentShowPanel::OnPaint(wxPaintEvent&)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    wxSize size = GetSize();
    
    wxSize  filamentTextSize = dc.GetTextExtent(m_filamentStr);

    wxPoint filamentTextpt = wxPoint(0, ((size.y - filamentTextSize.y) / 2 ) + FromDIP(1));

    dc.SetFont(Label::Body_13);
    dc.SetPen(StateColor::darkModeColorFor(COLOR_FARM_CHECK_TEXT));
    dc.SetTextForeground(StateColor::darkModeColorFor(COLOR_FARM_CHECK_TEXT));
    dc.DrawText(m_filamentStr, filamentTextpt);

    
    wxRect rect(filamentTextSize.x + FromDIP(4), FromDIP(3), FromDIP(14), FromDIP(16));

    dc.SetBrush(wxBrush(m_filamentColor));
    dc.SetPen(wxPen(StateColor::darkModeColorFor(COLOR_Ams_Box_borderColor), 1));

    dc.DrawRoundedRectangle(rect, FromDIP(4));
}



FilamentShowSinglePanel::FilamentShowSinglePanel(wxWindow* parent, const wxVector<FilamentTypeAndColorInfoObj>& infoList,bool showPop,bool isPopDialog)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), m_infoList(infoList), m_showPop(showPop), m_isPopDialog(isPopDialog)
{

    Init(infoList);
}


wxPanel* FilamentShowSinglePanel::CreatePanel(const wxColour& showColor, const wxString& showStr, const wxString& showWeight)
{ 
    wxPanel* panel = new wxPanel(this);
    panel->SetBackgroundColour(COLOR_Neutral_01);

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    wxPanel* showPanel = nullptr;
    if (m_isPopDialog) {
        showPanel = new FilamentShowPopPanel(panel, showColor, showStr, showWeight);
    } else {
        showPanel = new FilamentShowPanel(panel, showColor, showStr);
    }
    if (!m_isPopDialog) {
        wxStaticText* gap_index = new wxStaticText(panel, wxID_ANY, wxString("|"), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END);
        gap_index->Wrap(-1);
        gap_index->SetForegroundColour(COLOR_Neutral_05);
        gap_index->SetFont(Label::Body_11);

        sizer->Add(gap_index, 0, wxALIGN_CENTER);
        sizer->AddSpacer(FromDIP(16));
    }

    if (m_isPopDialog) {
        sizer->Add(showPanel, 1, wxEXPAND);
    } else {
        sizer->Add(showPanel);
    }

    if (!m_isPopDialog) {
        sizer->AddSpacer(FromDIP(16));
    }

    panel->SetSizer(sizer);
    sizer->Layout();
    return panel;

}

void FilamentShowSinglePanel::Init(const wxVector<FilamentTypeAndColorInfoObj>& infoList)
{
    this->SetBackgroundColour(COLOR_Neutral_01);
    wxSizer* mainSizer = !m_isPopDialog ? static_cast<wxSizer*>(new wxBoxSizer(wxHORIZONTAL)) :
                             static_cast<wxSizer*>(new wxGridSizer(infoList.size() >= 4 ? 4 : infoList.size(), FromDIP(4), FromDIP(4)));


    for (int i = 0; i < infoList.size(); i++) {
        if (i > 2 && !m_isPopDialog)
            break;
        const FilamentTypeAndColorInfoObj& obj = infoList[i];
        wxPanel*                           panel = CreatePanel(obj.showColor, obj.showStr, obj.weight);

        if (m_isPopDialog) {
            mainSizer->Add(panel, 1, wxEXPAND);
        } else {
            mainSizer->Add(panel);
        }
    }
    if (m_showPop && infoList.size() > 3) {
        Button* moreIcoInfo = new Button(this, "", "ams_box_more_ico", 0, 14);
        moreIcoInfo->SetTextColor(COLOR_FARM_TITLE_GRAY_TEXT);
        moreIcoInfo->SetPaddingSize(wxSize(0, 0));
        moreIcoInfo->SetBackgroundColor(wxColour(0, 0, 0, 0));
        moreIcoInfo->SetBorderWidth(0);
        moreIcoInfo->SetFont(Label::Body_13);
        
        mainSizer->Add(moreIcoInfo, 0, wxALIGN_CENTER , FromDIP(4));
        
        m_showPop = true;
    }
    if (!m_isPopDialog) {
        BindHoverEvents(this, this->GetSize());
    }


    SetSizerAndFit(mainSizer);
    mainSizer->Layout();
    Layout();
    Bind(EVT_POP_DELETE_EVENT, [this](wxPluginEvent& evt) { DeletePopWin(); });
}

void FilamentShowSinglePanel::DeletePopWin()
{
    if (m_filamentPopDialog) {
        //m_filamentPopDialog->Dismiss();
        delete m_filamentPopDialog;
        m_filamentPopDialog = nullptr;
    }
}

void FilamentShowSinglePanel::AmsInfoDialogOnHoverLeaveShowEvent()
{
    wxPoint mousePos;
    wxGetMousePosition(&mousePos.x, &mousePos.y);
    wxRect panelRect = this->GetScreenRect();

    if (!panelRect.Contains(mousePos)) {
        DeletePopWin();
    }
}

void FilamentShowSinglePanel::AmsInfoDialogOnHoverEnterShowEvent(wxWindow* win, wxSize size)
{
    if (!m_filamentPopDialog) {
        wxPoint pos    = win->GetScreenPosition() + wxPoint(0, win->FromDIP(8) + size.y);
        m_filamentPopDialog = new FilamentPopDialog(this, pos, m_infoList);
        m_filamentPopDialog->Move(pos);
        m_filamentPopDialog->Popup();
    }
}

void FilamentShowSinglePanel::UnBindHoverEvents(wxWindow* win, wxSize size)
{
    win->Unbind(wxEVT_ENTER_WINDOW, [this](wxMouseEvent& evt) {});

    win->Unbind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent& evt) {});

    for (auto* child : win->GetChildren()) {
        UnBindHoverEvents(child, size);
    }
}

void FilamentShowSinglePanel::BindHoverEvents(wxWindow* win, wxSize size)
{
    win->Bind(wxEVT_ENTER_WINDOW, [this, win, size](wxMouseEvent& evt) {
        AmsInfoDialogOnHoverEnterShowEvent(win, size);
        evt.Skip();
    });

    /*win->Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent& evt) {
        AmsInfoDialogOnHoverLeaveShowEvent();
        evt.Skip();
    });*/

    for (auto* child : win->GetChildren()) {
        BindHoverEvents(child, size);
    }
}

FilamentPopDialog::~FilamentPopDialog() 
{
    delete m_timer;
    m_timer = nullptr;

}
	
FilamentPopDialog::FilamentPopDialog(wxWindow* parent, wxPoint posPoint, const wxVector<FilamentTypeAndColorInfoObj>& infoList)
    : PopupWindow(parent, wxPU_CONTAINS_CONTROLS)
    , m_parent(parent)

{
    Init(infoList, posPoint);
}

void FilamentPopDialog::Init(const wxVector<FilamentTypeAndColorInfoObj>& infoList, wxPoint posPoint)
{
    SetBackgroundColour(COLOR_Neutral_01);
    wxBoxSizer*  mainSizer      = new wxBoxSizer(wxVERTICAL);

    wxPanel* amsBoxPanel = new FilamentShowSinglePanel(this, infoList,false,true);
    
    
    this->SetPosition(wxPoint(posPoint.x, posPoint.y));
    mainSizer->Add(amsBoxPanel, 1, wxALL | wxEXPAND, FromDIP(6));
    mainSizer->Layout();
    SetSize(wxSize(500,500));
    SetSizerAndFit(mainSizer);

    Layout();

    Slic3r::GUI::wxGetApp().UpdateDarkUIWin(this);
    Bind(wxEVT_PAINT, [this](wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(this);
        dc.SetBrush(wxBrush(GetBackgroundColour()));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(GetClientRect());

        dc.SetPen(wxPen(COLOR_Neutral_05, 1));
        wxRect rect = GetClientRect();
        dc.DrawRectangle(rect);
    });
    m_timer = new wxTimer;
    m_timer->Bind(wxEVT_TIMER, &FilamentPopDialog::OnTimer, this);
    m_timer->Start(100);

}


void FilamentPopDialog::OnTimer(wxTimerEvent& event)
{
    wxPoint mousePos;
    wxGetMousePosition(&mousePos.x, &mousePos.y);
    wxRect parentR = m_parent->GetScreenRect();
    if (!parentR.Contains(mousePos)) {
        this->Dismiss();
#ifndef _WIN32
        wxPluginEvent evt(EVT_POP_DELETE_EVENT);
        wxPostEvent(m_parent, evt);
#endif // _WIN32

    }
       
}

void FilamentPopDialog::OnDismiss()
{
    if (HasCapture())
        ReleaseMouse();
    wxPopupTransientWindow::OnDismiss();
#ifdef _WIN32
    wxPluginEvent evt(EVT_POP_DELETE_EVENT);
    wxPostEvent(m_parent, evt);
#endif // _WIN32

    

}
