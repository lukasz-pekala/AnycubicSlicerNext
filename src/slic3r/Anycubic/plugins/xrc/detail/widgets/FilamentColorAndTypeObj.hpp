#ifndef slic3r_GUI_Anycubic_RemotePrinting_FilamentColorAndTypeObj_hpp_
#define slic3r_GUI_Anycubic_RemotePrinting_FilamentColorAndTypeObj_hpp_

#include <wx/wx.h>
#include "slic3r/GUI/GUI.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/GUI_Utils.hpp"
#include <wx/dcbuffer.h>
#include <plugins_sdk/event/detail/plugin_custom_event.hxx>
#include <plugins_sdk/event/detail/custom_struct_manger.hxx>
#include <plugins_sdk/event/detail/idata_provider_manger.hxx>




struct FilamentTypeAndColorInfoObj
{
    wxColour showColor;
    wxString showStr;
    wxString weight;

};

class FilamentShowPopPanel : public wxPanel
{
public:
    FilamentShowPopPanel(wxWindow* parent, const wxColour& filamentColor, const wxString& filamentStr, const wxString& filamentWeight);
    ~FilamentShowPopPanel() = default;

private:
    wxColour m_filamentColor;
    wxColour m_showTextColor;
    wxString m_filamentStr;
    wxString m_filamentWeight;
    wxString m_filamentGapStr;
    int      m_gapIndex_1;
    int      m_gapIndex_2;

    void OnPaint(wxPaintEvent&);
};



class FilamentShowPanel : public wxPanel
{
public:
    FilamentShowPanel(wxWindow* parent, const wxColour& filamentColor, const wxString& filamentStr);
    ~FilamentShowPanel() = default;

private:
    wxColour m_filamentColor;
    wxString m_filamentStr;

    void OnPaint(wxPaintEvent&);
};


class FilamentShowSinglePanel : public wxPanel
{
public:
    FilamentShowSinglePanel(wxWindow* parent, const wxVector<FilamentTypeAndColorInfoObj>& infoList,bool showPop = false,bool isPopDialog=false);
    ~FilamentShowSinglePanel() = default;


    void BindHoverEvents(wxWindow* win, wxSize size);
    void UnBindHoverEvents(wxWindow* win, wxSize size);
    void AmsInfoDialogOnHoverLeaveShowEvent();
    void AmsInfoDialogOnHoverEnterShowEvent(wxWindow* win, wxSize size);
    void DeletePopWin();

private:
    wxPanel*                                 CreatePanel(const wxColour& showColor, const wxString& showStr, const wxString& showWeight);
    void                                     Init(const wxVector<FilamentTypeAndColorInfoObj>& infoList);
    wxPopupTransientWindow* m_filamentPopDialog{nullptr};
    wxVector<FilamentTypeAndColorInfoObj>    m_infoList;
    bool                                     m_showPop{false};
    bool                                     m_isPopDialog{false};
};


class FilamentPopDialog : public PopupWindow
{
public:
    FilamentPopDialog(wxWindow* parent, wxPoint posPoint, const wxVector<FilamentTypeAndColorInfoObj>& infoList);
    FilamentPopDialog(FilamentPopDialog&&) = delete;
    FilamentPopDialog(const FilamentPopDialog&) = delete;
    FilamentPopDialog& operator=(FilamentPopDialog&&) = delete;
    FilamentPopDialog& operator=(const FilamentPopDialog&) = delete;
    ~FilamentPopDialog();

    void Init(const wxVector<FilamentTypeAndColorInfoObj>& infoList, wxPoint posPoint);
    void OnTimer(wxTimerEvent& event);

    virtual void OnDismiss() wxOVERRIDE;


private:
    wxWindow *m_parent;
    wxTimer*  m_timer{nullptr};
    
};



#endif //!slic3r_GUI_Anycubic_RemotePrinting_FilamentColorAndTypeObj_hpp_
