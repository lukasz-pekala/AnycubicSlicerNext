#pragma once

#include "slic3r/GUI/wxExtensions.hpp"
#include "slic3r/GUI/Widgets/StaticBox.hpp"
#include <wx/event.h>
#include <wx/wx.h>
#include "RemotePrintingAmsDialog.hpp"
#include <plugins_sdk/event/detail/plugin_custom_event.hxx>
#include <plugins_sdk/event/detail/custom_struct_manger.hxx>
#include <plugins_sdk/event/detail/idata_provider_manger.hxx>


class AmsButton : public wxNavigationEnabled<StaticBox>
{

public:
    AmsButton(wxWindow* parent,
              bool      init,
              wxString  filamentType,
              wxSize    winSize,
              wxColour  typeColor,
              int       countIndex,
              std::string printerType = "",
              std::string nowDeviceID = "",
              int  slotNum   = -1,
              wxColour  slotColor = wxColour(245,245,245),
              long      style      = 0);

    ~AmsButton() {}

    void init();

    void OnChangeEvent(wxPluginEvent& event);
    void OnChangeAutoEvent(wxPluginEvent& event);

    void render(wxAutoBufferedPaintDC& dc);
    void paintEvent(wxPaintEvent& evt);

    void         mouseDown(wxMouseEvent& event);
    void SetSlotShowColor(wxColour slotColor);
    void SetSlotShowNumber(int showNum);
    void         SetSlotIsEmpty();
    void         SetShowFilamentDialog(bool index);
    int          GetCountIndex() { return m_countIndex; }


    DECLARE_EVENT_TABLE()


private:
    wxWindow *m_parent;
    wxSize         m_winSize;
    wxString       m_filamentType;
    int            m_slotNum{-1};
    wxColour       m_typeColor;
    wxColour       m_typeColor_text;
    wxColour       m_slotColor;
    wxColour       m_slotColor_text;
    bool           m_isClick{false};
    bool           m_isInit{false};
    wxBitmap*      m_dropBitmap{nullptr};
    wxRect         m_downRect;
    bool           m_downClick{false};

    wxBitmap       m_white_down_ico;
    wxBitmap       m_gray_down_ico;
    wxBitmap       m_now_down_ico;
    wxSize         m_icoSize;
    bool           m_isEmpty{false};
    int            m_countIndex{-1};
    std::string    m_nowDeviceID;
    std::map<int, AmsSlotObjInfo> m_amsSlotObjMap;

    ColorAMSBoxDialog* m_amsPop{nullptr};
    std::string                     m_printerType{""};

private:
    IDataProvider_AmsButton* m_amsButtonProvider;

public:
    void IDataProvider_RegisterAmsButton(IDataProvider_AmsButton* view) { m_amsButtonProvider = view; }

    void IDataProvider_UnregisterAmsButton() { m_amsButtonProvider = nullptr; }

};




