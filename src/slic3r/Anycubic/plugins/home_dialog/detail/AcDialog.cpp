#include "AcDialog.hpp"

#include <plugins_base/funcation.hxx>

#include <slic3r/GUI/GUI_App.hpp>
#include <plugins_sdk/event/detail/plugin_custom_event.hxx>

#ifdef __WXMSW__
#define DEFAULT_STYLE (wxCAPTION | wxCLOSE_BOX | wxNO_BORDER)
#elif defined(__WXMAC__)
#define DEFAULT_STYLE (wxSTAY_ON_TOP | wxCAPTION | wxCLOSE_BOX | wxNO_BORDER)
#elif defined(__WXGTK__)
#define DEFAULT_STYLE (wxCAPTION | wxCLOSE_BOX)
#endif // __WXMSW__

wxDEFINE_EVENT(EVT_DIALOG_CLOSE_CHILD_EVENT, wxCommandEvent);


const int dialog_style(bool isTestEnv)
{
    int s = DEFAULT_STYLE;
    if (isTestEnv) {
        s |= wxRESIZE_BORDER;
    }
    return s;
}

ACShowDialog::~ACShowDialog() 
{

}

ACShowDialog::ACShowDialog(wxWindow* parent, bool isTestEnv, wxSize panelSize)
    :Slic3r::GUI::DPIDialog(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, dialog_style(isTestEnv)), m_size(FromDIP(panelSize))
{
    Init(panelSize);
    
}


void ACShowDialog::Init(wxSize panelSize)
{
    m_mainSizer = new wxBoxSizer(wxHORIZONTAL);
    m_panel               = new wxPanel(this);
    m_mainSizer->Add(m_panel, 1, wxEXPAND);
    m_mainSizer->Layout();
    SetSizer(m_mainSizer);
#ifdef __APPLE__
    SetSize(panelSize);
#else
    this->SetMinSize(panelSize);
    SetSize(panelSize);
#endif
    Slic3r::GUI::wxGetApp().UpdateDlgDarkUI(this);


    this->Bind(EVT_DIALOG_CLOSE_CHILD_EVENT, [this](wxCommandEvent& evt) {
        wxCommandEvent close_evt(EVT_DIALOG_CLOSE_EVENT);
        close_evt.SetInt(evt.GetInt());
        this->ProcessEvent(close_evt);
        if (!m_isEndModel)
            EndModal(wxID_CANCEL);
        Destroy();
    });

    this->Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent& evt) {
        if (!m_deleteIndex) {
            m_deleteIndex = true;
            evt.Veto();
            int            returnCode = GetReturnCode();
            wxCommandEvent close_evt(EVT_DIALOG_CLOSE_CHILD_EVENT);
            close_evt.SetInt(returnCode);
            wxPostEvent(this, close_evt);
        } else {
            evt.Skip();
        }
    });

}

void ACShowDialog::ReplacePanel(wxPanel* panel) 
{
    {
        wxWindowUpdateLocker noUpdates(this);
        wxPanel*             oldpanel = m_panel;

        m_panel = panel;
        Slic3r::GUI::wxGetApp().UpdateDarkUIWin(m_panel);

        bool replaced = m_mainSizer->Replace(oldpanel, m_panel);
        oldpanel->Destroy();
    }
    Layout();


}

void ACShowDialog::msw_rescale()
{
    this->SetMinSize(m_size);

    Fit();

    this->Layout();

    Refresh();
}

void ACShowDialog::OnDialogReturn(int code) 
{ 
    if (m_isEndModel) {
        wxCommandEvent closeEvt(wxEVT_CLOSE_WINDOW);
        this->SetReturnCode(code);
        this->ProcessEvent(closeEvt);
    }

}