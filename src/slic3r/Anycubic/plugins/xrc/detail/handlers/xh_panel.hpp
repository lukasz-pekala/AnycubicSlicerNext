#pragma once

#include <wx/xrc/xh_panel.h>
#include "../widgets/ACCustomPanel.hpp"

class WXDLLIMPEXP_XRC ACPanelResourceHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(ACPanelResourceHandler);

public:
    ACPanelResourceHandler();

    virtual wxObject* DoCreateResource() wxOVERRIDE;
    virtual bool      CanHandle(wxXmlNode* node) wxOVERRIDE;
};
