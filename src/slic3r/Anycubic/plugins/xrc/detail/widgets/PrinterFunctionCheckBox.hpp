#ifndef slic3r_GUI_Anycubic_RemotePrinting_FunctionCheckBox_hpp_
#define slic3r_GUI_Anycubic_RemotePrinting_FunctionCheckBox_hpp_

#include "slic3r/GUI/wxExtensions.hpp"
#include "slic3r/GUI/Widgets/StaticBox.hpp"
#include <wx/event.h>
#include <wx/wx.h>

class FunctionCheckBox : public wxNavigationEnabled<StaticBox>
{

public:
    FunctionCheckBox(wxWindow* parent,
                         wxString  showText,
                         bool       isCheck,
                         wxSize      winSize,
                         long      style      = 0);

    ~FunctionCheckBox() {}

    void init();

    void    render(wxDC& dc);
    void    paintEvent(wxPaintEvent& evt);
    void    mouseDown(wxMouseEvent& event);
    void    mouseLeave(wxMouseEvent& event);
    void    mouseEnter(wxMouseEvent& event);
    bool    GetCheckState() { return !m_isCheck; }
    void    SetCheckState();

    DECLARE_EVENT_TABLE()

private:
    wxWindow *m_parent;
    wxSize         m_winSize;
    wxString       m_showText;
    bool           m_isCheck;
    bool           m_isHover{false};

    wxBitmap m_nowCheckIco;

    wxBitmap m_CheckIco_enable;
    wxBitmap m_CheckIco_disable;
   
};



#endif // !slic3r_GUI_FunctionCheckBox_hpp_
