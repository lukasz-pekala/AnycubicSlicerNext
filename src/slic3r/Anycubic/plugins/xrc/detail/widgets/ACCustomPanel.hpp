#pragma once

#include <wx/panel.h>

class ACCustomPanel : public wxPanel
{
public:
    ACCustomPanel() = default;

    ACCustomPanel(wxWindow*       parent,
                  wxWindowID      id    = wxID_ANY,
                  const wxPoint&  pos   = wxDefaultPosition,
                  const wxSize&   size  = wxDefaultSize,
                  long            style = wxTAB_TRAVERSAL,
                  const wxString& name  = wxASCII_STR("ACCustomPanel"));

    bool Create(wxWindow*       parent,
                wxWindowID      id    = wxID_ANY,
                const wxPoint&  pos   = wxDefaultPosition,
                const wxSize&   size  = wxDefaultSize,
                long            style = wxTAB_TRAVERSAL,
                const wxString& name  = wxASCII_STR("ACCustomPanel"));
};
