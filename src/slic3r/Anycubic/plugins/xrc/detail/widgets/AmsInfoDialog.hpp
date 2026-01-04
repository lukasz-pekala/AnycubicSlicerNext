#ifndef slic3r_GUI_Anycubic_RemotePrinting_AmsInfoDialog_hpp_
#define slic3r_GUI_Anycubic_RemotePrinting_AmsInfoDialog_hpp_

#include <wx/wx.h>
#include <slic3r/GUI/Widgets/PopupWindow.hpp>
#include <plugins_sdk/event/detail/plugin_custom_event.hxx>

#include "custom_struct_manger.hxx"

class AmsInfoPanel : public wxPanel
{

public:
    AmsInfoPanel(wxWindow* win, const wxVector<AmsSlotObjInfo>& slotInfoList, bool showOne, bool showSingle,bool isPop);
    AmsInfoPanel(AmsInfoPanel&&)      = delete;
    AmsInfoPanel(const AmsInfoPanel&) = delete;
    AmsInfoPanel&           operator=(AmsInfoPanel&&) = delete;
    AmsInfoPanel& operator=(const AmsInfoPanel&) = delete;
    ~AmsInfoPanel()                                      = default;


    void BindHoverEvents(wxWindow* win, wxSize size);
    void UnBindHoverEvents(wxWindow* win, wxSize size);
    void AmsInfoDialogOnHoverLeaveShowEvent();
    void AmsInfoDialogOnHoverEnterShowEvent(wxWindow* win, wxSize size);
    void DeletePopWin();

private:
    bool                         m_Pop{false};
    bool                         m_showPop{false};
    wxWindow* m_parent;
    wxPopupTransientWindow*      m_amsPopDialog{nullptr};
    wxVector<AmsSlotObjInfo>     m_slotInfoList;

};



class AmsInfoPop : public PopupWindow
{
public:
    AmsInfoPop(wxWindow* parent, wxPoint posPoint, const wxVector<AmsSlotObjInfo>& slotInfoList);
    AmsInfoPop(AmsInfoPop&&)          = delete;
    AmsInfoPop(const AmsInfoPop&) = delete;
    AmsInfoPop& operator=(AmsInfoPop&&) = delete;
    AmsInfoPop& operator=(const AmsInfoPop&) = delete;
    ~AmsInfoPop();

    void Init(const wxVector<AmsSlotObjInfo>& slotInfoList, wxPoint posPoint);
    void OnTimer(wxTimerEvent& event);

    virtual void OnDismiss() wxOVERRIDE;


private:
    wxWindow *m_parent;
    wxTimer*  m_timer{nullptr};
    bool      m_showPop{false};
    
};




#endif //!slic3r_GUI_Anycubic_RemotePrinting_AmsInfoDialog_hpp_
