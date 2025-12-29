#pragma once


#include <slic3r/GUI/GUI_Utils.hpp>

#include <wx/wx.h>

wxDECLARE_EVENT(EVT_DIALOG_CLOSE_CHILD_EVENT, wxCommandEvent);

class ACShowDialog: public Slic3r::GUI::DPIDialog
{
private:
    wxSize m_size;
    wxBoxSizer* m_mainSizer{nullptr};
    wxPanel* m_panel{nullptr};
    bool        m_isEndModel{false};
    bool        m_deleteIndex{false};

public:
    explicit ACShowDialog(wxWindow* parent, bool isTestEnv, wxSize panelSize);
    ACShowDialog(ACShowDialog&&) = delete;
    ACShowDialog(const ACShowDialog&) = delete;
    ACShowDialog& operator=(ACShowDialog&&) = delete;
    ACShowDialog& operator=(const ACShowDialog&) = delete;
    ~ACShowDialog();

    void Init(wxSize panelSize);
    void ReplacePanel(wxPanel* panel);
    void OnDialogReturn(int code);
    void EndModal(int retCode) override
    {
        m_isEndModel = true;
        OnDialogReturn(retCode);
        wxDialog::EndModal(retCode);
    }    

protected:
    void msw_rescale();
    void on_dpi_changed(const wxRect& suggested_rect) override { msw_rescale(); }

    
};
