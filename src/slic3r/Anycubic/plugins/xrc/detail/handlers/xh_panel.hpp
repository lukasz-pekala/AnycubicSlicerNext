#pragma once

#include "../widgets/ACCustomPanel.hpp"
#include "wx/xrc/xmlres.h"

class WXEXPORT ACPanelResourceHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(ACPanelResourceHandler);

public:
    ACPanelResourceHandler();

    virtual wxObject* DoCreateResource() wxOVERRIDE;
    virtual bool      CanHandle(wxXmlNode* node) wxOVERRIDE;
};
