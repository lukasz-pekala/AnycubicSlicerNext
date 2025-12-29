#include "wx/wxprec.h"



#include "xh_panel.hpp"

#ifndef WX_PRECOMP
    #include "wx/panel.h"
    #include "wx/frame.h"
#endif

wxIMPLEMENT_DYNAMIC_CLASS(ACPanelResourceHandler, wxXmlResourceHandler);

ACPanelResourceHandler::ACPanelResourceHandler()
{
    XRC_ADD_STYLE(wxTAB_TRAVERSAL);
    XRC_ADD_STYLE(wxWS_EX_VALIDATE_RECURSIVELY);
    AddWindowStyles();
}

wxObject* ACPanelResourceHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(panel, ACCustomPanel)

    panel->Create(m_parentAsWindow, GetID(), GetPosition(), GetSize(), GetStyle(wxT("style"), wxTAB_TRAVERSAL), GetName());

    SetupWindow(panel);
    CreateChildren(panel);

    return panel;
}

bool ACPanelResourceHandler::CanHandle(wxXmlNode* node) { return IsOfClass(node, wxT("ACCustomPanel")); }


