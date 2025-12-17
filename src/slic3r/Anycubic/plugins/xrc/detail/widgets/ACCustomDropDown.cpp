#include "ACCustomDropDown.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"

#include <wx/display.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>

#ifdef __WXGTK__
#include <gtk/gtk.h>
#endif

wxDEFINE_EVENT(EVT_CUSTOM_DISMISS, wxCommandEvent);

BEGIN_EVENT_TABLE(ACCustomDropDown, PopupWindow)

EVT_LEFT_DOWN(ACCustomDropDown::mouseDown)
EVT_LEFT_UP(ACCustomDropDown::mouseReleased)
EVT_MOUSE_CAPTURE_LOST(ACCustomDropDown::mouseCaptureLost)
EVT_MOTION(ACCustomDropDown::mouseMove)
EVT_MOUSEWHEEL(ACCustomDropDown::mouseWheelMoved)

// catch paint events
EVT_PAINT(ACCustomDropDown::paintEvent)

END_EVENT_TABLE()

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

ACCustomDropDown::ACCustomDropDown(std::vector<wxString> &texts,
                   std::vector<wxString> &tips, std::vector<wxBitmap>& icons, int widthMax)
    : texts(texts)
    , tips(tips)
    , icons(icons)
    , m_widthMax(widthMax)
    , state_handler(this)
    , border_color(0xDBDBDB)
    , text_color(0x363636)
    , selector_border_color(std::make_pair(0x437DFF, (int) StateColor::Hovered),
        std::make_pair(*wxWHITE, (int) StateColor::Normal))
    , selector_background_color(std::make_pair(0xEFF5FF, (int) StateColor::Checked), // ORCA updated background color for checked item
        std::make_pair(*wxWHITE, (int) StateColor::Normal))
{
}

ACCustomDropDown::ACCustomDropDown(wxWindow *             parent,
                   std::vector<wxString> &texts,
                   std::vector<wxString> &tips,
                   std::vector<wxBitmap> &icons,
                   long           style)
    : ACCustomDropDown(texts, tips, icons)
{
    Create(parent, style);
}

void ACCustomDropDown::Create(wxWindow *     parent,
         long           style)
{
    PopupWindow::Create(parent, wxPU_CONTAINS_CONTROLS);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour(*wxWHITE);
    state_handler.attach({&border_color, &text_color, &selector_border_color, &selector_background_color});
    state_handler.update_binds();
    if ((style & DD_NO_CHECK_ICON) == 0)
        check_bitmap = ScalableBitmap(this, "checked", 12);
    text_off = style & DD_NO_TEXT;

    // BBS set default font
    SetFont(Label::Body_13);
#ifdef __WXOSX__
    // PopupWindow releases mouse on idle, which may cause various problems,
    //  such as losting mouse move, and dismissing soon on first LEFT_DOWN event.
    Bind(wxEVT_IDLE, [] (wxIdleEvent & evt) {});
#endif
    if (m_timer == nullptr) {
        m_timer = new wxTimer(this, wxID_ANY);
        this->Bind(wxEVT_TIMER, &ACCustomDropDown::OnTimer, this, m_timer->GetId());
        m_timer->Start(500);
    }
}

ACCustomDropDown::~ACCustomDropDown() 
{ 
    CloseTimerObj();
}

void ACCustomDropDown::OnTimer(wxTimerEvent& event) 
{ 
    if (m_isShowBar) {

        if (m_timerIndex > 0) {

            m_isShowBar = false;
            m_timerIndex = 0;
            paintNow();

        }
        if (!m_isRightBar)
            m_timerIndex++;
    }

}

void ACCustomDropDown::CloseTimerObj() 
{
    if (m_timer != nullptr) {
        if (m_timer->IsRunning())
            m_timer->Stop();
        delete m_timer;
        m_timer = nullptr;
    }
}

void ACCustomDropDown::Invalidate(bool clear)
{
    if (clear) {
        selection = hover_item = -1;
        offset = wxPoint();
    }
    assert(selection < (int) texts.size());
    need_sync = true;
}

void ACCustomDropDown::SetSelection(int n)
{
    //assert(n < (int) texts.size());
    if (n >= (int) texts.size())
        n = -1;
    if (selection == n) return;
    selection = n;
    if (need_sync) { // for icon Size
        messureSize();
        need_sync = true;
    }
    paintNow();
}

wxString ACCustomDropDown::GetValue() const
{
    return selection >= 0 ? texts[selection] : wxString();
}

void ACCustomDropDown::SetValue(const wxString &value)
{
    auto i = std::find(texts.begin(), texts.end(), value);
    selection = i == texts.end() ? -1 : std::distance(texts.begin(), i);
}

void ACCustomDropDown::SetCornerRadius(double radius)
{
    this->radius = radius;
    paintNow();
}

void ACCustomDropDown::SetBorderColor(StateColor const &color)
{
    border_color = color;
    state_handler.update_binds();
    paintNow();
}

void ACCustomDropDown::SetSelectorBorderColor(StateColor const &color)
{
    selector_border_color = color;
    state_handler.update_binds();
    paintNow();
}

void ACCustomDropDown::SetTextColor(StateColor const &color)
{
    text_color = color;
    state_handler.update_binds();
    paintNow();
}

void ACCustomDropDown::SetSelectorBackgroundColor(StateColor const &color)
{
    selector_background_color = color;
    state_handler.update_binds();
    paintNow();
}

void ACCustomDropDown::SetUseContentWidth(bool use, bool limit_max_content_width)
{
    if (use_content_width == use)
        return;
    use_content_width = use;
    this->limit_max_content_width = limit_max_content_width;
    need_sync = true;
    messureSize();
}

void ACCustomDropDown::SetAlignIcon(bool align) { align_icon = align; }

void ACCustomDropDown::Rescale()
{
    need_sync = true;
}

bool ACCustomDropDown::HasDismissLongTime()
{
    auto now = boost::posix_time::microsec_clock::universal_time();
    return !IsShown() &&
        (now - dismissTime).total_milliseconds() >= 20;
}

void ACCustomDropDown::paintEvent(wxPaintEvent& evt)
{
    // depending on your system you may need to look at double-buffered dcs
    wxBufferedPaintDC dc(this);
    render(dc);
}

/*
 * Alternatively, you can use a clientDC to paint on the panel
 * at any time. Using this generally does not free you from
 * catching paint events, since it is possible that e.g. the window
 * manager throws away your drawing when the window comes to the
 * background, and expects you will redraw it when the window comes
 * back (by sending a paint event).
 */
void ACCustomDropDown::paintNow()
{
    // depending on your system you may need to look at double-buffered dcs
    //wxClientDC dc(this);
    //render(dc);
    Refresh();
}

static wxSize GetBmpSize(wxBitmap & bmp)
{
#ifdef __APPLE__
    return bmp.GetScaledSize();
#else
    return bmp.GetSize();
#endif
}

void ACCustomDropDown::SetShowBar() 
{ 
    m_isShowBar = true;
    m_timerIndex = 0;
    paintNow();
}

void ACCustomDropDown::calculateBarEvent(const int& hover, const wxPoint& now_point, const wxSize& winSize, bool isWheelMouse)
{ 
    if (isWheelMouse) {
        SetShowBar();
    } else if (hover > -1) {
        int  gap_index    = winSize.x - now_point.x;
        bool now_itemShow = gap_index < 20 && gap_index > 0;
        m_isRightBar      = now_itemShow;

        if (now_itemShow) {
            SetShowBar();
        }
    }
}


void ACCustomDropDown::calculateSize(wxRect& rcContent, const int barSize, wxDC& dc)
{
    //rcContent.Deflate(barSize, 1);// x =0 y=170 w=536 h=34  x =15 y=171 w=506 h=32
    rcContent.x = 4;
    rcContent.y += 1;
    rcContent.width -= (barSize + 8);
    rcContent.height -= 2;
    dc.DrawRectangle(rcContent);
    //rcContent.Inflate(barSize, 1);
    rcContent.x = 0;
    rcContent.y -= 1;
    rcContent.width += (barSize + 8);
    rcContent.height += 2;

    

}

/*
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void ACCustomDropDown::render(wxDC &dc)
{
    if (texts.size() == 0) return;
    int states = state_handler.states();
    dc.SetPen(wxPen(border_color.colorForStates(states)));
    dc.SetBrush(wxBrush(StateColor::darkModeColorFor(GetBackgroundColour())));
    // if (GetWindowStyle() & wxBORDER_NONE)
    //    dc.SetPen(wxNullPen);

    int barSize      = m_isShowBar?FromDIP(10):0;
    int bar_Gap_Size = FromDIP(2);


    // draw background
    wxSize size = GetSize();
    if (radius == 0)
        dc.DrawRectangle(0, 0, size.x, size.y);
    else
        dc.DrawRoundedRectangle(0, 0, size.x, size.y, radius);

    // draw hover rectangle
    wxRect rcContent = {{0, offset.y}, rowSize};
    if (hover_item >= 0 && (states & StateColor::Hovered)) {
        rcContent.y += rowSize.y * hover_item;
        if (rcContent.GetBottom() > 0 && rcContent.y < size.y) {
            if (selection == hover_item)
                dc.SetBrush(wxBrush(selector_background_color.colorForStates(states | StateColor::Checked)));
            dc.SetPen(wxPen(selector_border_color.colorForStates(states)));
            calculateSize(rcContent, barSize, dc);
        }
        rcContent.y = offset.y;
    }
    
    // draw checked rectangle
    if (selection >= 0 && (selection != hover_item || (states & StateColor::Hovered) == 0)) {
        rcContent.y += rowSize.y * selection;
        if (rcContent.GetBottom() > 0 && rcContent.y < size.y) {
            dc.SetBrush(wxBrush(selector_background_color.colorForStates(states | StateColor::Checked)));
            dc.SetPen(wxPen(selector_background_color.colorForStates(states)));
            calculateSize(rcContent, barSize, dc);
        }
        rcContent.y = offset.y;
    }
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    {
        wxSize offset = (rowSize - textSize) / 2;
        rcContent.Deflate(0, offset.y);
    }

    // draw position bar
    if (rowSize.y * texts.size() > size.y) {
        int    height = rowSize.y * texts.size();
        wxRect rect   = {size.x - (barSize + bar_Gap_Size), -offset.y * size.y / height, barSize,
                       size.y * size.y / height};
        dc.SetPen(wxPen(border_color.defaultColor()));
        dc.SetBrush(wxBrush(*wxLIGHT_GREY));
        dc.DrawRoundedRectangle(rect, FromDIP(6));
        rcContent.width -= (barSize + bar_Gap_Size);
    }

    // draw check icon
    rcContent.x += 5;
    rcContent.width -= 5;
    if (check_bitmap.bmp().IsOk()) {
        int  letfGap = FromDIP(8);
        auto szBmp = check_bitmap.GetBmpSize();
        if (selection >= 0) {
            wxPoint pt = rcContent.GetLeftTop();
            pt.y += (rcContent.height - szBmp.y) / 2;
            pt.y += rowSize.y * selection;
            pt.x += letfGap;
            if (pt.y + szBmp.y > 0 && pt.y < size.y)
                dc.DrawBitmap(check_bitmap.bmp(), pt);
        }
        rcContent.x += szBmp.x + 5 + letfGap;
        rcContent.width -= szBmp.x + 5 + letfGap;
    }
    // draw texts & icons
    dc.SetTextForeground(text_color.colorForStates(states));
    for (int i = 0; i < texts.size(); ++i) {
        if (rcContent.GetBottom() < 0) {
            rcContent.y += rowSize.y;
            continue;
        }
        if (rcContent.y > size.y) break;
        wxPoint pt   = rcContent.GetLeftTop();
        auto & icon = icons[i];
        auto size2 = GetBmpSize(icon);
        if (iconSize.x > 0) {
            if (icon.IsOk()) {
                pt.y += (rcContent.height - size2.y) / 2;
                dc.DrawBitmap(icon, pt);
            }
            pt.x += iconSize.x + 5;
            pt.y = rcContent.y;
        } else if (icon.IsOk()) {
            pt.y += (rcContent.height - size2.y) / 2;
            dc.DrawBitmap(icon, pt);
            pt.x += size2.x + 5;
            pt.y = rcContent.y;
        }
        auto text = texts[i];
        if (!text_off && !text.IsEmpty()) {
            wxSize tSize = dc.GetMultiLineTextExtent(text);
            if (pt.x + tSize.x > rcContent.GetRight()) {
                if (i == hover_item && tips[i].IsEmpty())
                    SetToolTip(text);
                text = wxControl::Ellipsize(text, dc, wxELLIPSIZE_END,
                                            rcContent.GetRight() - pt.x);
            }
            pt.y += (rcContent.height - textSize.y) / 2;
            dc.SetFont(GetFont());
            dc.DrawText(text, pt);
        }
        rcContent.y += rowSize.y;
    }
}

void ACCustomDropDown::messureSize()
{
    if (!need_sync) return;
    textSize = wxSize();
    iconSize = wxSize();
    wxClientDC dc(GetParent() ? GetParent() : this);
    for (size_t i = 0; i < texts.size(); ++i) {
        wxSize size1 = text_off ? wxSize() : dc.GetMultiLineTextExtent(texts[i]);
        if (icons[i].IsOk()) {
            wxSize size2 = GetBmpSize(icons[i]);
            if (size2.x > iconSize.x) iconSize = size2;
            if (!align_icon) {
                size1.x += size2.x + (text_off ? 0 : 5);
            }
        }
        if (size1.x > textSize.x) textSize = size1;
    }
    if (!align_icon) iconSize.x = 0;
    wxSize szContent = textSize;
    szContent.x += 10;
    if (check_bitmap.bmp().IsOk()) {
        auto szBmp = check_bitmap.GetBmpSize();
        szContent.x += szBmp.x + 5;
    }
    if (iconSize.x > 0) szContent.x += iconSize.x + (text_off ? 0 : 5);
    if (iconSize.y > szContent.y) szContent.y = iconSize.y;
    szContent.y += 10;
    if (texts.size() > 15) szContent.x += 6;
    if (GetParent() && m_widthMax == -1) {
        auto x = GetParent()->GetSize().x;
        if (!use_content_width || x > szContent.x)
            szContent.x = x;
    }
    if (m_widthMax != -1) {
        szContent.x *= 1.1;
        if (szContent.x > m_widthMax)
            szContent.x = m_widthMax;
    }
    rowSize = szContent;
    if (limit_max_content_width) {
        wxSize parent_size = GetParent()->GetSize();
        if (rowSize.x > parent_size.x * 2) {
            rowSize.x = 2 * parent_size.x;
            szContent = rowSize;
        }
    }
    szContent.y *= std::min((size_t)15, texts.size());
    szContent.y += texts.size() > 15 ? rowSize.y / 2 : 0;
    wxWindow::SetSize(szContent);
#ifdef __WXGTK__
    // Gtk has a wrapper window for popup widget
    gtk_window_resize (GTK_WINDOW (m_widget), szContent.x, szContent.y);
#endif
    need_sync = false;
}

void ACCustomDropDown::autoPosition()
{
    messureSize();
    wxPoint pos = GetParent()->ClientToScreen(wxPoint(0, -6));
    wxPoint old = GetPosition();
    wxSize size = GetSize();
    Position(pos, {0, GetParent()->GetSize().y + 12});
    if (old != GetPosition()) {
        size = rowSize;
        size.y *= std::min((size_t)15, texts.size());
        size.y += texts.size() > 15 ? rowSize.y / 2 : 0;
        if (size != GetSize()) {
            wxWindow::SetSize(size);
            offset = wxPoint();
            Position(pos, {0, GetParent()->GetSize().y + 12});
        }
    }
    if (GetPosition().y > pos.y) {
        // may exceed
        auto drect = wxDisplay(GetParent()).GetGeometry();
        if (GetPosition().y + size.y + 10 > drect.GetBottom()) {
            if (use_content_width && texts.size() <= 15) size.x += 6;
            size.y = drect.GetBottom() - GetPosition().y - 10;
            wxWindow::SetSize(size);
            if (selection >= 0) {
                if (offset.y + rowSize.y * (selection + 1) > size.y)
                    offset.y = size.y - rowSize.y * (selection + 1);
                else if (offset.y + rowSize.y * selection < 0)
                    offset.y = -rowSize.y * selection;
            }
        }
    }
}

void ACCustomDropDown::mouseDown(wxMouseEvent& event)
{
    // Receivce unexcepted LEFT_DOWN on Mac after OnDismiss
    if (!IsShown())
        return;
    // force calc hover item again
    mouseMove(event);
    pressedDown = true;
    CaptureMouse();
    dragStart   = event.GetPosition();
}

void ACCustomDropDown::mouseReleased(wxMouseEvent& event)
{
    if (pressedDown) {
        dragStart = wxPoint();
        pressedDown = false;
        if (HasCapture())
            ReleaseMouse();
        if (hover_item >= 0 && !m_isRightBar) { // not moved
            sendDropDownEvent();
            DismissAndNotify();
        }
    }
}

void ACCustomDropDown::mouseCaptureLost(wxMouseCaptureLostEvent &event)
{
    /*wxMouseEvent evt;
    mouseReleased(evt);*/
    wxMouseEvent evt(wxEVT_LEFT_UP);
    event.SetEventObject(this);
    GetEventHandler()->ProcessEvent(evt);
}

void ACCustomDropDown::mouseMove(wxMouseEvent &event)
{
    wxPoint pt = event.GetPosition();
    if (pressedDown) {
        int     dir   = m_isRightBar ? -1 : 1;
        wxPoint delta = pt - dragStart;
        delta.x *= dir;
        delta.y *= dir;

        wxPoint pt2 = offset + delta;

        dragStart = pt;

        int contentHeight = rowSize.y * int(texts.size());
        int minOffset     = std::min(0, GetSize().y - contentHeight);
        if (pt2.y > 0)
            pt2.y = 0;
        else if (pt2.y < minOffset)
            pt2.y = minOffset;

        if (pt2.y != offset.y) {
            offset     = pt2;
            hover_item = -1;
        } else {
            return;
        }
    }


    if (!pressedDown || hover_item >= 0) {
        int hover = (pt.y - offset.y) / rowSize.y;
        calculateBarEvent(hover,pt, rowSize);
        if (hover >= (int) texts.size()) hover = -1;
        if (hover == hover_item) return;
        hover_item = hover;
        if (hover >= 0) SetToolTip(tips[hover]);
    }
    paintNow();
}

void ACCustomDropDown::mouseWheelMoved(wxMouseEvent &event)
{
    bool    isReturn = false;
    auto delta = event.GetWheelRotation();
    wxSize  size  = GetSize();
    wxPoint pt2   = offset + wxPoint{0, delta};
    if (pt2.y > 0)
        pt2.y = 0;
    else if (pt2.y + rowSize.y * int(texts.size()) < size.y)
        pt2.y = size.y - rowSize.y * int(texts.size());
    if (pt2.y != offset.y) {
        offset = pt2;
    } else {
        calculateBarEvent(0, wxPoint(0,0), rowSize, true);
        return;
    }
    wxPoint pt    = event.GetPosition();
    int     hover = (pt.y - offset.y) / rowSize.y;
    calculateBarEvent(hover,pt, rowSize,true);
    if (hover >= (int) texts.size()) hover = -1;
    if (hover != hover_item) {
        hover_item = hover;
        if (hover >= 0) SetToolTip(tips[hover]);
    }
    paintNow();
}

// currently unused events
void ACCustomDropDown::sendDropDownEvent()
{
    wxCommandEvent event(wxEVT_COMBOBOX, GetId());
    event.SetEventObject(this);
    event.SetInt(hover_item);
    event.SetString(texts[hover_item]);
    GetEventHandler()->ProcessEvent(event);
}

void ACCustomDropDown::OnDismiss()
{
    if (HasCapture())
        ReleaseMouse();
    this->Unbind(wxEVT_LEFT_UP, &ACCustomDropDown::mouseReleased, this);
    this->Unbind(wxEVT_LEFT_DOWN, &ACCustomDropDown::mouseDown, this);
    this->Unbind(wxEVT_MOUSE_CAPTURE_LOST, &ACCustomDropDown::mouseCaptureLost, this);
    this->Unbind(wxEVT_MOTION, &ACCustomDropDown::mouseMove, this);
    this->Unbind(wxEVT_MOUSEWHEEL, &ACCustomDropDown::mouseWheelMoved, this);
    this->Unbind(wxEVT_PAINT, &ACCustomDropDown::paintEvent, this);
    dismissTime = boost::posix_time::microsec_clock::universal_time();
    hover_item  = -1;
    wxCommandEvent e(EVT_CUSTOM_DISMISS);
    GetEventHandler()->ProcessEvent(e);
    m_isShowBar = false;
    m_isRightBar = false;
    m_timerIndex = -1;
}
