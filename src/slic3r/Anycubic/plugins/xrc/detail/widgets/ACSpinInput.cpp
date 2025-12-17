#include "ACSpinInput.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"
#include "slic3r/GUI/Widgets/Button.hpp"
#include "slic3r/GUI/Widgets/TextCtrl.h"

#include <wx/dcgraph.h>

BEGIN_EVENT_TABLE(ACSpinInput, wxPanel)

EVT_KEY_DOWN(ACSpinInput::keyPressed)
//EVT_MOUSEWHEEL(ACSpinInput::mouseWheelMoved)

EVT_PAINT(ACSpinInput::paintEvent)

END_EVENT_TABLE()

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

ACSpinInput::ACSpinInput()
    : label_color(std::make_pair(0x909090, (int) StateColor::Disabled), std::make_pair(0x6B6B6B, (int) StateColor::Normal))
    , text_color(std::make_pair(0x909090, (int) StateColor::Disabled), std::make_pair(0x262E30, (int) StateColor::Normal))
{
    radius = 0;
    border_width     = 1;
    border_color     = StateColor(std::make_pair(0xDBDBDB, (int) StateColor::Disabled), std::make_pair(0x3986FF, (int) StateColor::Hovered),
                              std::make_pair(0xDBDBDB, (int) StateColor::Normal));
    background_color = StateColor(std::make_pair(0xF0F0F1, (int) StateColor::Disabled), std::make_pair(*wxWHITE, (int) StateColor::Normal));
}


ACSpinInput::ACSpinInput(wxWindow *parent,
                     wxString       text,
                     wxString       label,
                     const wxPoint &pos,
                     const wxSize & size,
                     long           style,
                     int min, int max, int initial)
    : ACSpinInput()
{
    Create(parent, text, label, pos, size, style, min, max, initial);
}

void ACSpinInput::Create(wxWindow *parent, 
                     wxString       text,
                     wxString       label,
                     const wxPoint &pos,
                     const wxSize & size,
                     long           style,
                     int min, int max, int initial)
{
    StaticBox::Create(parent, wxID_ANY, pos, size);
    SetFont(Label::Body_12);
    wxWindow::SetLabel(label);
    state_handler.attach({&label_color, &text_color});
    state_handler.update_binds();
    text_ctrl = new TextCtrl(this, wxID_ANY, text, {20, 4}, wxDefaultSize, style | wxBORDER_NONE | wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_DIGITS));
    text_ctrl->SetFont(Label::Body_14);
    text_ctrl->SetBackgroundColour(background_color.colorForStates(state_handler.states()));
    text_ctrl->SetForegroundColour(text_color.colorForStates(state_handler.states()));
    text_ctrl->SetInitialSize(text_ctrl->GetBestSize());
    state_handler.attach_child(text_ctrl);
    text_ctrl->Bind(wxEVT_KILL_FOCUS, &ACSpinInput::onTextLostFocus, this);
    text_ctrl->Bind(wxEVT_TEXT_ENTER, &ACSpinInput::onTextEnter, this);
    text_ctrl->Bind(wxEVT_KEY_DOWN, &ACSpinInput::keyPressed, this);
    text_ctrl->Bind(wxEVT_RIGHT_DOWN, [this](auto &e) {}); // disable context menu
    button_inc = createButton(true);
    button_dec = createButton(false);
    delta      = 0;
    timer.Bind(wxEVT_TIMER, &ACSpinInput::onTimer, this);

    long initialFromText;
    if (text.ToLong(&initialFromText)) initial = initialFromText;
    SetRange(min, max);
    SetValue(initial);
    messureSize();
}

void ACSpinInput::SetCornerRadius(double radius)
{
    this->radius = radius;
    Refresh();
}

void ACSpinInput::SetLabel(const wxString &label)
{
    wxWindow::SetLabel(label);
    messureSize();
    Refresh();
}

void ACSpinInput::SetLabelColor(StateColor const &color)
{
    label_color = color;
    state_handler.update_binds();
}

void ACSpinInput::SetTextColor(StateColor const &color)
{
    text_color = color;
    state_handler.update_binds();
}

void ACSpinInput::SetSize(wxSize const &size)
{
    StaticBox::SetSize(size);
    Rescale();
}

void ACSpinInput::SetValue(const wxString &text)
{
    long value;
    if ( text.ToLong(&value) )
        SetValue(value);
}

void ACSpinInput::SetNowIntValue(int delta) 
{
    wxString now_val = text_ctrl->GetValue();
    if (now_val.IsNumber()) {
        int nowHeat;
        nowHeat = std::stoi(now_val.ToStdString());
        val     = nowHeat;
    }
    SetValue(val + delta);
}

void ACSpinInput::SetValue(int value)
{
    if (value < min) value = min;
    else if (value > max) value = max;
    this->val = value;
    text_ctrl->SetValue(wxString::FromDouble(value));
}

int ACSpinInput::GetValue()const
{
    return val;
}

void ACSpinInput::SetRange(int min, int max)
{
    this->min = min;
    this->max = max;
}

void ACSpinInput::DoSetToolTipText(wxString const &tip)
{ 
    wxWindow::DoSetToolTipText(tip);
    text_ctrl->SetToolTip(tip);
}

void ACSpinInput::Rescale()
{
    button_inc->Rescale();
    button_dec->Rescale();
    messureSize();
}

bool ACSpinInput::Enable(bool enable)
{
    bool result = text_ctrl->Enable(enable) && wxWindow::Enable(enable);
    if (result) {
        wxCommandEvent e(EVT_ENABLE_CHANGED);
        e.SetEventObject(this);
        GetEventHandler()->ProcessEvent(e);
        text_ctrl->SetBackgroundColour(background_color.colorForStates(state_handler.states()));
        text_ctrl->SetForegroundColour(text_color.colorForStates(state_handler.states()));
        button_inc->Enable(enable);
        button_dec->Enable(enable);
    }
    return result;
}

void ACSpinInput::paintEvent(wxPaintEvent& evt)
{
    // depending on your system you may need to look at double-buffered dcs
    wxPaintDC dc(this);
    render(dc);
}

/*
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void ACSpinInput::render(wxDC& dc)
{
    StaticBox::render(dc);
    int    states = state_handler.states();
    wxSize size = GetSize();
    // draw seperator of buttons
    wxPoint pt = button_inc->GetPosition();
    pt.y = size.y / 2;
    dc.SetPen(wxPen(border_color.defaultColor()));
    dc.DrawLine(pt, pt + wxSize{button_inc->GetSize().x - 2, 0});
    // draw label
    auto label = GetLabel();
    if (!label.IsEmpty()) {
        pt.x = size.x - labelSize.x - 5;
        pt.y = (size.y - labelSize.y) / 2;
        dc.SetFont(GetFont());
        dc.SetTextForeground(label_color.colorForStates(states));
        dc.DrawText(label, pt);
    }
}

void ACSpinInput::messureSize()
{
    wxSize size = GetSize();
    wxSize textSize = text_ctrl->GetSize();
    int h = textSize.y + 8;
    if (size.y < h) {
        size.y = h;
    }
    wxSize minSize = size;
    minSize.x      = GetMinWidth();
    StaticBox::SetSize(size);
    SetMinSize(size);
    wxSize btnSize = {14, (size.y - 4) / 2};
    btnSize.x = btnSize.x * btnSize.y / 10;
    wxClientDC dc(this);
    labelSize  = dc.GetMultiLineTextExtent(GetLabel());
    textSize.x = size.x - labelSize.x - btnSize.x - 16;
    text_ctrl->SetSize(textSize);
    text_ctrl->SetPosition({6, (size.y - textSize.y) / 2});
    button_inc->SetSize(btnSize);
    button_dec->SetSize(btnSize);
    button_inc->SetPosition({minSize.x - btnSize.x - 3, size.y / 2 - btnSize.y - 1});
    button_dec->SetPosition({minSize.x - btnSize.x - 3, size.y / 2 + 1});
}

Button *ACSpinInput::createButton(bool inc)
{
    auto btn = new Button(this, "", inc ? "spin_inc" : "spin_dec", wxBORDER_NONE, 6);
    btn->SetCornerRadius(0);
    btn->DisableFocusFromKeyboard();
    btn->Bind(wxEVT_LEFT_DOWN, [=](auto &e) {
        delta = inc ? 1 : -1;
        if (m_isLoadNow) {
            SetNowIntValue(delta);
        } else {
            SetValue(val + delta);
        }
        text_ctrl->SetFocus();
        if (!btn->HasCapture())
            btn->CaptureMouse();
        delta *= 8;
        timer.Start(100);
        sendSpinEvent();
    });
    btn->Bind(wxEVT_LEFT_DCLICK, [=](auto &e) {
        delta = inc ? 1 : -1;
        if (!btn->HasCapture())
            btn->CaptureMouse();
        if (m_isLoadNow) {
            SetNowIntValue(delta);
        } else {
            SetValue(val + delta);
        }
        sendSpinEvent();
    });
    btn->Bind(wxEVT_LEFT_UP, [=](auto &e) {
        if (btn->HasCapture())
            btn->ReleaseMouse();
        timer.Stop();
        text_ctrl->SelectAll();
        delta = 0;
    });
    return btn;
}

void ACSpinInput::onTimer(wxTimerEvent &evnet) {
    if (delta < -1 || delta > 1) {
        delta /= 2;
        return;
    }
    if (m_isLoadNow) {
        SetNowIntValue(delta);
    } else {
        SetValue(val + delta);
    }
    sendSpinEvent();
}

void ACSpinInput::onTextLostFocus(wxEvent &event)
{
    timer.Stop();
    for (auto * child : GetChildren())
        if (auto btn = dynamic_cast<Button*>(child))
            if (btn->HasCapture())
                btn->ReleaseMouse();
    wxCommandEvent e;
    onTextEnter(e);
    // pass to outer
    event.SetId(GetId());
    ProcessEventLocally(event);
    e.Skip();
}

void ACSpinInput::onTextEnter(wxCommandEvent &event)
{
    long value;
    if (!text_ctrl->GetValue().ToLong(&value)) { value = val; }
    if (value != val) {
        if (m_isLoadNow) {
            SetNowIntValue(delta);
        } else {
            SetValue(val + delta);
        }
        sendSpinEvent();
    }
    event.SetId(GetId());
    ProcessEventLocally(event);
}

void ACSpinInput::mouseWheelMoved(wxMouseEvent &event)
{
    auto delta = event.GetWheelRotation() < 0 ? 1 : -1;
    if (m_isLoadNow) {
        SetNowIntValue(delta);
    } else {
        SetValue(val + delta);
    }
    sendSpinEvent();
    text_ctrl->SetFocus();
}

void ACSpinInput::keyPressed(wxKeyEvent &event)
{
    switch (event.GetKeyCode()) {
    case WXK_UP:
    case WXK_DOWN:
        long value;
        if (!text_ctrl->GetValue().ToLong(&value)) { value = val; }
        if (event.GetKeyCode() == WXK_DOWN && value > min) {
            --value;
        } else if (event.GetKeyCode() == WXK_UP && value + 1 < max) {
            ++value;
        }
        if (value != val) {
            SetValue(value);
            sendSpinEvent();
        }
        break;
    default: event.Skip(); break;
    }
}

void ACSpinInput::sendSpinEvent()
{
    wxCommandEvent event(wxEVT_SPINCTRL, GetId());
    event.SetEventObject(this);
    GetEventHandler()->ProcessEvent(event); 
}
