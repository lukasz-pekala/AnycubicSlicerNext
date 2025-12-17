#ifndef slic3r_GUI_ACCustomDropDown_hpp_
#define slic3r_GUI_ACCustomDropDown_hpp_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <wx/stattext.h>
#include "slic3r/GUI/wxExtensions.hpp"
#include "slic3r/GUI/Widgets/StateHandler.hpp"
#include "slic3r/GUI/Widgets/PopupWindow.hpp"

#define DD_NO_CHECK_ICON    0x0001
#define DD_NO_TEXT          0x0002
#define DD_STYLE_MASK       0x0003

wxDECLARE_EVENT(EVT_CUSTOM_DISMISS, wxCommandEvent);

class ACCustomDropDown : public PopupWindow
{
    std::vector<wxString> &       texts;
    std::vector<wxString> &       tips;
    std::vector<wxBitmap> &     icons;
    bool                          need_sync  = false;
    int                         selection = -1;
    int                         hover_item = -1;

    double radius = 0;
    bool   use_content_width = false;
    bool   limit_max_content_width = false;
    bool   align_icon        = false;
    bool   text_off          = false;

    wxSize textSize;
    wxSize iconSize;
    wxSize rowSize;

    StateHandler state_handler;
    StateColor   text_color;
    StateColor   border_color;
    StateColor   selector_border_color;
    StateColor   selector_background_color;
    ScalableBitmap check_bitmap;

    bool pressedDown = false;
    boost::posix_time::ptime dismissTime;
    wxPoint                  offset; // x not used
    wxPoint                  dragStart;
    bool                     m_isShowBar{false};
    bool                     m_isRightBar{false};
    wxTimer*                 m_timer{nullptr};
    int                      m_timerIndex{-1};

    int m_widthMax{-1};


public:
    ACCustomDropDown(std::vector<wxString>& texts, std::vector<wxString>& tips, std::vector<wxBitmap>& icons, int widthMax =-1);
    
    ACCustomDropDown(
        wxWindow* parent,
             std::vector<wxString> &texts,
             std::vector<wxString> &tips,
             std::vector<wxBitmap> &icons,
             long           style     = 0);
    ~ACCustomDropDown();
    
    void Create(wxWindow *     parent,long           style     = 0);
    
public:
    void Invalidate(bool clear = false);

    int GetSelection() const { return selection; }

    void SetSelection(int n);

    wxString GetValue() const;
    void     SetValue(const wxString &value);

public:
    void SetCornerRadius(double radius);

    void SetBorderColor(StateColor const & color);

    void SetSelectorBorderColor(StateColor const & color);

    void SetTextColor(StateColor const &color);

    void SetSelectorBackgroundColor(StateColor const &color);

    void SetUseContentWidth(bool use, bool limit_max_content_width = false);

    void SetAlignIcon(bool align);

    void OnTimer(wxTimerEvent& event);
    void CloseTimerObj();

public:
    void Rescale();

    bool HasDismissLongTime();
    
protected:
    void OnDismiss() override;

private:
    void SetShowBar();

    void paintEvent(wxPaintEvent& evt);
    void paintNow();

    void render(wxDC& dc);

    void calculateSize(wxRect& rcContent, const int barSize, wxDC& dc);
    void calculateBarEvent(const int& hover, const wxPoint& now_point, const wxSize& winSize,bool isWheelMouse=false);
    friend class ACCustomComboBox;
    void messureSize();
    void autoPosition();

    // some useful events
    void mouseDown(wxMouseEvent& event);
    void mouseReleased(wxMouseEvent &event);
    void mouseCaptureLost(wxMouseCaptureLostEvent &event);
    void mouseMove(wxMouseEvent &event);
    void mouseWheelMoved(wxMouseEvent &event);

    void sendDropDownEvent();


    DECLARE_EVENT_TABLE()
};

#endif // !slic3r_GUI_DropDown_hpp_
