#ifndef slic3r_GUI_StaticBox_hpp_
#define slic3r_GUI_StaticBox_hpp_

#include "../wxExtensions.hpp"
#include "StateHandler.hpp"

#include <wx/window.h>

class StaticBox : public wxWindow
{
public:
    StaticBox();

    StaticBox(wxWindow* parent,
             wxWindowID      id        = wxID_ANY,
             const wxPoint & pos       = wxDefaultPosition,
             const wxSize &  size      = wxDefaultSize, 
             long style = 0);

    bool Create(wxWindow* parent,
        wxWindowID      id        = wxID_ANY,
        const wxPoint & pos       = wxDefaultPosition,
        const wxSize &  size      = wxDefaultSize, 
        long style = 0);

    void SetCornerRadius(double radius);

    void SetBorderWidth(int width);

    void SetBorderColor(StateColor const & color);

    void SetBorderColorNormal(wxColor const &color);

    void SetBackgroundColor(StateColor const &color);

    void SetBackgroundColorNormal(wxColor const &color);

    void SetBackgroundColor2(StateColor const &color);

    static wxColor GetParentBackgroundColor(wxWindow * parent);

protected:
    void eraseEvent(wxEraseEvent& evt);

    void paintEvent(wxPaintEvent& evt);

    void render(wxDC& dc);

    virtual void doRender(wxDC& dc);

protected:
    double radius;
    int border_width = 1;
    StateHandler state_handler;
    StateColor   border_color;
    StateColor   background_color;
    StateColor   background_color2;

    DECLARE_EVENT_TABLE()
};

class SplitLine : public StaticBox
{
public:
    enum SplitDir
    {
        SplitHorizontal = wxHORIZONTAL,
        SplitVertical = wxVERTICAL,
    };

public:
    SplitLine(wxWindow* parent, SplitDir dir = SplitHorizontal, int lineLen = 14, int lineWidth = 1, long style = 0);

    bool Create(wxWindow* parent, SplitDir dir = SplitHorizontal, int lineLen = 14, int lineWidth = 1, long style = 0);

    void setLineColour(const wxColour& c);
    void setLinePadding(int padding);

    void sys_color_changed();
protected:
    void Rescale();
    void messureSize();
    void render(wxDC& dc);

private:
    SplitDir m_splitDir = SplitHorizontal;
    int m_padding = 4;
    int m_lineWidth = 1;
    int m_lineLen = 14;
    wxColour m_lineColour = wxColour(0,0,0);
};


#endif // !slic3r_GUI_StaticBox_hpp_
