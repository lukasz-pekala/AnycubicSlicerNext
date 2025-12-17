#ifndef slic3r_GUI_Anycubic_RemotePrinting_RemoteSlotColorPanel_hpp_
#define slic3r_GUI_Anycubic_RemotePrinting_RemoteSlotColorPanel_hpp_

#include "slic3r/GUI/wxExtensions.hpp"
#include "slic3r/GUI/Widgets/StaticBox.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include <wx/event.h>
#include <wx/wx.h>
#include <plugins_sdk/event/detail/plugin_custom_event.hxx>
#include <plugins_sdk/event/detail/custom_struct_manger.hxx>
#include <plugins_sdk/event/detail/idata_provider_manger.hxx>



class RemoteSlotColorPanel : public wxNavigationEnabled<StaticBox>
{
public:
    RemoteSlotColorPanel(wxWindow* parent, wxVector<AmsSlotObjInfo> sltInfo, wxSize winSize, int id, bool showSingle,long style = 0);

    ~RemoteSlotColorPanel();

    void init();
    void BindEvent(bool showSingle);

    void render(wxDC& dc);
    void paintEvent(wxPaintEvent& evt);
    void SetColorList(wxVector<AmsSlotObjInfo> sltInfo) 
    {
        m_sltInfo = sltInfo;
        Refresh();
    }

    DECLARE_EVENT_TABLE()

private:
    int                                      m_id;
    wxWindow*                                m_parent;
    wxSize                                   m_winSize;
    wxVector<AmsSlotObjInfo> m_sltInfo;


private:
    IDataProvider_colorPanel* m_colorPanelProvider;

public:
    void IDataProvider_RegisterColorPanel(IDataProvider_colorPanel* view) { m_colorPanelProvider = view; }

    void IDataProvider_UnregisterColorPanel() { m_colorPanelProvider = nullptr; }

};



#endif // !slic3r_GUI_RemoteSlotColorPanel_hpp_
