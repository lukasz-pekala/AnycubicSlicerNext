#ifndef slic3r_GUI_ACStaticBox_hpp_
#define slic3r_GUI_ACStaticBox_hpp_

#include "slic3r/GUI/Widgets/StateHandler.hpp"

#include <wx/window.h>

class ACStaticBox : public wxWindow
{
public:
    ACStaticBox();

    ACStaticBox(wxWindow* parent,
             wxWindowID      id        = wxID_ANY,
             const wxPoint & pos       = wxDefaultPosition,
             const wxSize &  size      = wxDefaultSize, 
             long style = 0,
             wxString name = "");

    enum CornerRadiusType {
        CornerTopLeft     = 0x01,
        CornerTopRight    = 0x02,
        CornerBottomLeft  = 0x04,
        CornerBottomRight = 0x08,
        CornerTop         = CornerTopLeft | CornerTopRight,
        CornerBottom      = CornerBottomLeft | CornerBottomRight,
        CornerLeft        = CornerTopLeft | CornerBottomLeft,
        CornerRight       = CornerTopRight | CornerBottomRight,
        CornerAll         = CornerLeft | CornerRight,
    };

    bool Create(wxWindow* parent,
        wxWindowID      id        = wxID_ANY,
        const wxPoint & pos       = wxDefaultPosition,
        const wxSize &  size      = wxDefaultSize, 
        long style = 0,
    wxString name = "");

    void SetCornerRadius(double radius);

    void SetCornerRadiusType(CornerRadiusType type);

    void SetCornerRadius(double radius, CornerRadiusType type);

    void SetBorderWidth(int width);

    void SetBorderColor(StateColor const& color);

    void SetBorderColorNormal(wxColor const &color);

    void SetBackgroundColor(StateColor const& color);

    void SetBackgroundColorNormal(wxColor const &color);

    void SetBackgroundColor2(StateColor const &color);

    static wxColor GetParentBackgroundColor(wxWindow * parent);

    void clearColor();

    void setTakeFocusedAsHovered(bool);
    void setAmsIndex(bool index) { isAmsIndex = index; }
    void setAmsBroderIndex(bool index) { m_isBroderIndex = index; }
    void setEnable(bool index) { m_isEnable = index; }
    void SetColor2AndRadiusIndex(bool index) { m_isColor2AndRadius = index; }

protected:
    void eraseEvent(wxEraseEvent& evt);

    void paintEvent(wxPaintEvent& evt);

    void onSizeChanged(wxSizeEvent& e);
    virtual void render(wxDC& dc);

    //virtual void doRender(wxDC& dc);
    virtual void messureSize(){};
protected:
    bool           m_isBroderIndex{false};
    bool           m_isColor2AndRadius{false};
    bool           m_isEnable{false};
    bool           isAmsIndex{false};
    double radius;
    int radiusType;
    int border_width = 1;
    StateHandler state_handler;
    StateColor   border_color;
    StateColor   background_color;
    StateColor   background_color2;

    bool m_sizeValid = false;
    DECLARE_EVENT_TABLE()
};

#endif // !slic3r_GUI_StaticBox_hpp_
