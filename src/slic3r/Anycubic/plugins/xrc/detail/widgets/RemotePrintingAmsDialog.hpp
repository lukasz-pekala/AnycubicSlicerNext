#ifndef slic3r_GUI_Anycubic_RemotePrinting_RemotePrintingAmsDialog_hpp_
#define slic3r_GUI_Anycubic_RemotePrinting_RemotePrintingAmsDialog_hpp_

#include <wx/wx.h>
#include "slic3r/GUI/GUI.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/GUI_Utils.hpp"
#include "slic3r/GUI/BitmapCache.hpp"
#include "libslic3r/Utils.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"
#include "slic3r/GUI/Widgets/Button.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

#include <plugins_sdk/event/detail/plugin_custom_event.hxx>
#include <plugins_sdk/event/detail/custom_struct_manger.hxx>
#include <plugins_sdk/event/detail/idata_provider_manger.hxx>





class ColorBoxObject : public wxNavigationEnabled<StaticBox>
{
public:
    ColorBoxObject(wxWindow* parent,
                   wxSize    winSize,
                   AmsSlotObjInfo amsSlotBoj,
                   bool      isCheck   = false,
                   std::string deviceID = "");
    ~ColorBoxObject() = default;

    void Init();
    void SetCheckState(bool index);

    int GetSlotNum() { return m_showSlotNum; }
    void     mouseDown(wxMouseEvent& event);
    void     mouseLeave(wxMouseEvent& event);
    void     mouseEnter(wxMouseEvent& event);
    void     render(wxAutoBufferedPaintDC& dc);
    void     edgeColorRender(wxAutoBufferedPaintDC&    dc,
                             const wxPoint&            point,
                             const wxColour& color, int type  = 0);
    void     paintEvent(wxPaintEvent& evt);

    DECLARE_EVENT_TABLE()

private:
    wxWindow* m_parent;
    int       m_showSlotNum{-1};
    wxBitmap  m_check_ico;
    wxString  m_showFilamentType;
    wxColour  m_showColor;
    wxColour  m_showColor_Text;
    bool      m_isEnable{true};
    std::string m_deviceID;
    wxSize    m_winSize;
    bool      m_isHover{false};
    bool      m_isCheck{false};
    AmsSlotObjInfo m_amsSlotObj;
};


class ColorAMSBoxDialog : public PopupWindow
{
public:
    ColorAMSBoxDialog(wxWindow* parent,
                      wxWindow* amsParent,
                      wxString& filamentType,
                      int       parentIndex,
                      int         slotNum,
                      wxPoint&    posPint,
                      std::string deviceID = "");
    ColorAMSBoxDialog(ColorAMSBoxDialog&&)          = delete;
    ColorAMSBoxDialog(const ColorAMSBoxDialog&) = delete;
    ColorAMSBoxDialog& operator=(ColorAMSBoxDialog&&) = delete;
    ColorAMSBoxDialog& operator=(const ColorAMSBoxDialog&) = delete;
    ~ColorAMSBoxDialog()                                       = default;

    void      Init();
    wxSize    GetWindowSize();
    wxWindow* GetAmsParentWin() { return m_amsParent; }
    void      msw_rescale();
    void      SetShowNewSlotInfo(wxWindow* amsParent, wxString& filamentType, int parentIndex, int slotNum, std::string deviceID);
    void      SetFilamentType(wxString& filamentType) { m_selectFilamentType = filamentType; }
    void      SetDeviceID(std::string deviceID) { m_deviceID = deviceID; }
    int       GetParentIndex() { return m_parentIndex; }
    wxString  GetFilamentType() { return m_selectFilamentType; }
    bool      GetMouseIsWin() { return m_isInlcudeWin; }

    void mouseDown(wxMouseEvent& event);
    void mouseCaptureLost(wxMouseCaptureLostEvent& event);
    void mouseMove(wxMouseEvent& event);
    void mouseReleased(wxMouseEvent& event);

    void SendEventToChild(const wxPoint& point,int eventType);

    virtual void OnDismiss() wxOVERRIDE;
    bool         HasDismissLongTime();

private:
    IDataProvider_AsmDialog* m_amsDialogProvider;

public:
    void IDataProvider_RegisterAmsDialog(IDataProvider_AsmDialog* view) { m_amsDialogProvider = view; }

    void IDataProvider_UnregisterDialog() { m_amsDialogProvider = nullptr; }



private:
    wxWindow *m_parent;
    wxWindow*                    m_amsParent{nullptr};
    wxPoint   m_posPint;
    wxString  m_selectFilamentType;
    std::string            m_deviceID;
    int       m_slotSum{-1};
    int       m_slotNum{-1};
    int       m_parentIndex{-1};
    Button*   m_warningInfo{nullptr};
    std::vector<ColorBoxObject*> m_colorBoxList;
    ColorBoxObject*              m_lastHoverBox = nullptr;
    bool                         m_isInlcudeWin{false};
    bool                         m_pressedDown{false};
    boost::posix_time::ptime     dismissTime;
    DECLARE_EVENT_TABLE()
};





#endif //!slic3r_GUI_Anycubic_RemotePrinting_RemotePrintingAmsDialog_hpp_
