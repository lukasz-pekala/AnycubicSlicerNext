#include "ACCustomPanel.hpp"

ACCustomPanel::ACCustomPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    Create(parent, id, pos, size, style, name);
}

bool ACCustomPanel::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    if (!wxPanel::Create(parent, id, pos, size, style, name))
        return false;

    
    return true;
}
