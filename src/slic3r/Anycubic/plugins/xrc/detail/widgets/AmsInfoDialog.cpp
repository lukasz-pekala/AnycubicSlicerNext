#include "AmsInfoDialog.hpp"

#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/font.h>
#include <wx/dcgraph.h>
#include <string>
#include <stdio.h>
#include <plugins_sdk/event/detail/program_color.hxx>
#include "RemoteSlotColorPanel.hpp"
#include "slic3r/GUI/GUI_App.hpp"



AmsInfoPanel::AmsInfoPanel(wxWindow* win, const wxVector<AmsSlotObjInfo>& slotInfoList, bool showOne, bool showSingle, bool isPop)
    : wxPanel(win, wxID_ANY, wxDefaultPosition, wxDefaultSize), m_slotInfoList(slotInfoList), m_Pop(isPop)
{

    SetBackgroundColour(COLOR_Neutral_01);

    wxSize      panelSize(win->FromDIP(104), win->FromDIP(28));
    wxBoxSizer* color_sizer = new wxBoxSizer(wxHORIZONTAL);

    const int    groupSize = 4;
    const int    cols      = 2;
    wxGridSizer* gridSizer = new wxGridSizer(cols, 0, 0);

    bool isOne = m_slotInfoList.size() <= 4;

    for (size_t i = 0; i < m_slotInfoList.size(); i += groupSize) {
        if (showOne && i > 0)
            break;
        if (!showSingle && i > groupSize)
            break;

        wxVector<AmsSlotObjInfo> subList;
        size_t                      end = std::min(i + groupSize, m_slotInfoList.size());
        //subList.insert(subList.end(), m_slotInfoList.begin() + i, m_slotInfoList.begin() + end);
        subList.reserve(end - i);
        for (size_t j = i; j < end; ++j) {
            subList.push_back(m_slotInfoList[j]);
        }



        RemoteSlotColorPanel* panel = new RemoteSlotColorPanel(this, subList, panelSize, i, showSingle);
        if (!showOne) {
            gridSizer->Add(panel, 0, wxEXPAND | wxALL, win->FromDIP(4));
        } else {
            color_sizer->Add(panel, 0, wxEXPAND);
        }
    }
    if (!showOne) {
        color_sizer->Add(gridSizer, 1, wxEXPAND);
    }
    if (!m_Pop) {
        Button* moreIcoInfo = new Button(this, "", "ams_box_more_ico", 0, 14);
        moreIcoInfo->SetTextColor(COLOR_FARM_TITLE_GRAY_TEXT);
        moreIcoInfo->SetPaddingSize(wxSize(0, 0));
        moreIcoInfo->SetBackgroundColor(wxColour(0, 0, 0, 0));
        moreIcoInfo->SetBorderWidth(0);
        moreIcoInfo->SetFont(Label::Body_13);
        Bind(EVT_AMS_SHOW_CHANGE_MORE_EVENT, [this, moreIcoInfo, color_sizer](wxPluginEvent& evt) {
            bool show = evt.GetInt() == 1;
            m_showPop = show;
            
            if (color_sizer->IsShown(moreIcoInfo) != show) {
                if (show) {
                    color_sizer->Show(moreIcoInfo);
                } else {
                    color_sizer->Hide(moreIcoInfo);
                }
            }
            AmsBoxObj* d = static_cast<AmsBoxObj*>(evt.GetSharedData());
            m_slotInfoList = d->slotInfo;
            Refresh();
            
        });
        color_sizer->Add(moreIcoInfo, 0, wxALIGN_CENTER | wxLEFT, win->FromDIP(4));
        bool isBindEventIndex = true;
        if (!(m_slotInfoList.size() > (showOne ? 4 : 8))) {
            color_sizer->Hide(moreIcoInfo);
            if (showSingle)
                isBindEventIndex = false;
        }
        if (isBindEventIndex) {
            BindHoverEvents(this, panelSize);
        }
        m_showPop = true;
    }

    SetSizer(color_sizer);
    color_sizer->Layout();

    Bind(EVT_POP_DELETE_EVENT, [this](wxPluginEvent& evt) { DeletePopWin(); });
}

void AmsInfoPanel::DeletePopWin() 
{
    if (m_amsPopDialog) {
        delete m_amsPopDialog;
        m_amsPopDialog = nullptr;
    }
    
}

void AmsInfoPanel::AmsInfoDialogOnHoverLeaveShowEvent()
{
    wxPoint mousePos;
    wxGetMousePosition(&mousePos.x, &mousePos.y);
    wxRect  panelRect = this->GetScreenRect();

    if (!panelRect.Contains(mousePos)) {
        DeletePopWin();
    }
}

void AmsInfoPanel::AmsInfoDialogOnHoverEnterShowEvent(wxWindow* win, wxSize size)
{
    
    if (!m_amsPopDialog&&m_showPop) {
        wxPoint pos    = win->GetScreenPosition() + wxPoint(0, win->FromDIP(8) + size.y);
        m_amsPopDialog = new AmsInfoPop(this, pos, m_slotInfoList);
        m_amsPopDialog->Move(pos);
        m_amsPopDialog->Popup();
    }
}

void AmsInfoPanel::UnBindHoverEvents(wxWindow* win, wxSize size) 
{

    win->Unbind(wxEVT_ENTER_WINDOW, [this](wxMouseEvent& evt) {});

    win->Unbind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent& evt) {});

    for (auto* child : win->GetChildren()) {
        UnBindHoverEvents(child, size);
    }

}

void AmsInfoPanel::BindHoverEvents(wxWindow* win, wxSize size)
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

AmsInfoPop::~AmsInfoPop() 
{
    delete m_timer;
    m_timer = nullptr;
    
}
	
AmsInfoPop::AmsInfoPop(wxWindow* parent, wxPoint posPoint, const wxVector<AmsSlotObjInfo>& slotInfoList)
    : PopupWindow(parent, wxPU_CONTAINS_CONTROLS)
    , m_parent(parent)

{
    Init(slotInfoList, posPoint);
    
}

void AmsInfoPop::Init(const wxVector<AmsSlotObjInfo>& slotInfoList, wxPoint posPoint)
{
    
    wxBoxSizer*  mainSizer      = new wxBoxSizer(wxVERTICAL);
    wxPanel*    amsBoxPanel =  new AmsInfoPanel(this, slotInfoList, false, true, true);
    
    this->SetPosition(wxPoint(posPoint.x, posPoint.y));
    mainSizer->Add(amsBoxPanel, 1, wxALL | wxEXPAND, FromDIP(1));
    mainSizer->Layout();
    SetSize(wxSize(500,500));
    SetSizerAndFit(mainSizer);

    Layout();

    Slic3r::GUI::wxGetApp().UpdateDarkUIWin(this);
    m_timer = new wxTimer;
    m_timer->Bind(wxEVT_TIMER, &AmsInfoPop::OnTimer, this);
    m_timer->Start(100);

}

void AmsInfoPop::OnTimer(wxTimerEvent& event)
{
    wxPoint mousePos;
    wxGetMousePosition(&mousePos.x, &mousePos.y);
    if (!m_parent->GetScreenRect().Contains(mousePos)) {
        this->Dismiss();
#ifndef _WIN32
        wxPluginEvent evt(EVT_POP_DELETE_EVENT);
        wxPostEvent(m_parent, evt);
#endif // _WIN32
    }
}


void AmsInfoPop::OnDismiss() 
{
    if (HasCapture())
        ReleaseMouse();
    wxPopupTransientWindow::OnDismiss();
#ifdef _WIN32
    wxPluginEvent evt(EVT_POP_DELETE_EVENT);
    wxPostEvent(m_parent, evt);
#endif // _WIN32


}

