#include "ACCustomComboBox.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"

#include <plugins_sdk/event/detail/program_color.hxx>

#include <wx/dcgraph.h>

BEGIN_EVENT_TABLE(ACCustomComboBox, TextInput)

EVT_LEFT_DOWN(ACCustomComboBox::mouseDown)
EVT_LEFT_DCLICK(ACCustomComboBox::mouseDown)
//EVT_MOUSEWHEEL(ACCustomComboBox::mouseWheelMoved)
EVT_KEY_DOWN(ACCustomComboBox::keyDown)

// catch paint events
END_EVENT_TABLE()

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

static wxWindow *GetScrollParent(wxWindow *pWindow)
{
    wxWindow *pWin = pWindow;
    while (pWin->GetParent()) {
        auto pWin2 = pWin->GetParent();
        if (auto top = dynamic_cast<wxScrollHelper *>(pWin2))
            return dynamic_cast<wxWindow *>(pWin);
        pWin = pWin2;
    }
    return nullptr;
}

ACCustomComboBox::ACCustomComboBox(wxWindow *parent,
                   wxWindowID      id,
                   const wxString &value,
                   const wxPoint & pos,
                   const wxSize &  size,
                   int             n,
                   const wxString  choices[],
                   int textMaxWidth,
                   long            style)
    : drop(texts, tips, icons, textMaxWidth)
{
    if (style & wxCB_READONLY)
        style |= wxRIGHT;
    text_off = style & CB_NO_TEXT;
    TextInput::Create(parent, "", value, (style & CB_NO_DROP_ICON) ? "" : "drop_down", pos, size,
                      style | wxTE_PROCESS_ENTER);
    drop.Create(this, style & DD_STYLE_MASK);

    if (style & wxCB_READONLY) {
        GetTextCtrl()->Hide();
        TextInput::SetFont(Label::Body_14);
        TextInput::SetBorderColor(StateColor(
            std::make_pair(0xDBDBDB, (int) StateColor::Disabled),
            std::make_pair(0x437DFF, (int) StateColor::Hovered),
            std::make_pair(0xDBDBDB, (int) StateColor::Normal),
            std::make_pair(0xEFF5FF, (int) StateColor::Focused)
        ));
        TextInput::SetBackgroundColor(StateColor(
            std::make_pair(0xF0F0F1, (int) StateColor::Disabled),
            std::make_pair(0xEFF5FF, (int) StateColor::Focused),
            std::make_pair(0xFFFFFF, (int) StateColor::Normal),
            std::make_pair(0xFFFFFF, (int) StateColor::Pressed)
        ));
        TextInput::SetLabelColor(StateColor(
            std::make_pair(0x909090, (int) StateColor::Disabled),
            std::make_pair(0x262E30, (int) StateColor::Normal)
        ));
    }
    if (auto scroll = GetScrollParent(this))
        scroll->Bind(wxEVT_MOVE, &ACCustomComboBox::onMove, this);
    drop.Bind(wxEVT_COMBOBOX, [this](wxCommandEvent &e) {
        SetSelection(e.GetInt());
        e.SetEventObject(this);
        e.SetId(GetId());
        GetEventHandler()->ProcessEvent(e);
    });
    drop.Bind(EVT_CUSTOM_DISMISS, [this](auto&) {
        drop_down = false;
        wxCommandEvent e(wxEVT_COMBOBOX_CLOSEUP);
        GetEventHandler()->ProcessEvent(e);
        SetShowTextIco();
    });
    for (int i = 0; i < n; ++i) Append(choices[i]);
}

int ACCustomComboBox::GetSelection() const { return drop.GetSelection(); }

void ACCustomComboBox::SetStyleInfo(int style) 
{
    if (style == 0) {
        StateColor background_color = StateColor(std::make_pair(COLOR_Neutral_03, (int) StateColor::Disabled),
                                                 std::make_pair(COLOR_Neutral_03, (int) StateColor::Normal));
        StateColor text_color       = StateColor(std::make_pair(COLOR_FARM_TITLE_GRAY_TEXT, (int) StateColor::Disabled),
                                           std::make_pair(COLOR_FARM_TITLE_GRAY_TEXT, (int) StateColor::Normal));
        StateColor borderColor      = StateColor(std::make_pair(COLOR_Neutral_03, (int) StateColor::Disabled),
                                            std::make_pair(COLOR_Neutral_03, (int) StateColor::Normal));

        TextInput::SetBackgroundColor(background_color);
        TextInput::SetBorderColor(borderColor);
        TextInput::SetLabelColor(text_color);
        TextInput::SetBorderWidth(0);
    }

}


void ACCustomComboBox::SetShowTextIco(bool show) 
{

    wxString icon_path = show ? "drop_down_white" : "drop_down";
    auto        bitmap       = ScalableBitmap(this, icon_path.ToStdString(), 12).bmp();
    TextInput::SetIcon(bitmap);

    if (show) {
        TextInput::SetBorderColor(0x3986FF);
        //TextInput::SetBackgroundColor(0x254030);
    } else {
        TextInput::SetBorderColor( StateColor(std::make_pair(0xDBDBDB, (int) StateColor::Disabled), std::make_pair(0x437DFF, (int) StateColor::Hovered),
                       std::make_pair(0xDBDBDB, (int) StateColor::Normal), std::make_pair(0xEFF5FF, (int) StateColor::Focused)));
        /*TextInput::SetBackgroundColor(
            StateColor(std::make_pair(0xF0F0F1, (int) StateColor::Disabled), std::make_pair(0xEFF5FF, (int) StateColor::Focused),
                       std::make_pair(0xFFFFFF, (int) StateColor::Normal), std::make_pair(0xFFFFFF, (int) StateColor::Pressed)));*/
    }
    






    TextInput::Refresh();
}

void ACCustomComboBox::SetSelection(int n)
{
    if (n == drop.selection)
        return;
    drop.SetSelection(n);
    SetLabel(drop.GetValue());
    if (drop.selection >= 0 && drop.iconSize.y > 0)
        SetIcon(icons[drop.selection].IsNull() ? create_scaled_bitmap("drop_down", nullptr, 16): icons[drop.selection]); // ORCA fix combo boxes without arrows
}
void ACCustomComboBox::SelectAndNotify(int n) { 
    SetSelection(n);
    sendComboBoxEvent();
}

void ACCustomComboBox::Rescale()
{
    TextInput::Rescale();
    drop.Rescale();
}

wxString ACCustomComboBox::GetValue() const
{
    return drop.GetSelection() >= 0 ? drop.GetValue() : GetLabel();
}

void ACCustomComboBox::SetValue(const wxString &value)
{
    drop.SetValue(value);
    SetLabel(value);
    if (drop.selection >= 0 && drop.iconSize.y > 0)
        SetIcon(icons[drop.selection].IsNull() ? create_scaled_bitmap("drop_down", nullptr, 16): icons[drop.selection]); // ORCA fix combo boxes without arrows
}

void ACCustomComboBox::SetLabel(const wxString &value)
{
    if (GetTextCtrl()->IsShown() || text_off)
        GetTextCtrl()->SetValue(value);
    else
        TextInput::SetLabel(value);
}

wxString ACCustomComboBox::GetLabel() const
{
    if (GetTextCtrl()->IsShown() || text_off)
        return GetTextCtrl()->GetValue();
    else
        return TextInput::GetLabel();
}

void ACCustomComboBox::SetTextLabel(const wxString& label)
{
    TextInput::SetLabel(label);
}

wxString ACCustomComboBox::GetTextLabel() const
{
    return TextInput::GetLabel();
}

bool ACCustomComboBox::SetFont(wxFont const& font)
{
    if (GetTextCtrl() && GetTextCtrl()->IsShown())
        return GetTextCtrl()->SetFont(font);
    else
        return TextInput::SetFont(font);
}

int ACCustomComboBox::Append(const wxString &item, const wxBitmap &bitmap)
{
    return Append(item, bitmap, nullptr);
}

int ACCustomComboBox::Append(const wxString &item,
                     const wxBitmap &bitmap,
                     void *          clientData)
{
    texts.push_back(item);
    tips.push_back(wxString{});
    icons.push_back(bitmap);
    datas.push_back(clientData);
    types.push_back(wxClientData_None);
    drop.Invalidate();
    return texts.size() - 1;
}

void ACCustomComboBox::DoClear()
{
    SetIcon("drop_down");
    texts.clear();
    tips.clear();
    icons.clear();
    datas.clear();
    types.clear();
    drop.Invalidate(true);
}

void ACCustomComboBox::DoDeleteOneItem(unsigned int pos)
{
    if (pos >= texts.size()) return;
    texts.erase(texts.begin() + pos);
    tips.erase(tips.begin() + pos);
    icons.erase(icons.begin() + pos);
    datas.erase(datas.begin() + pos);
    types.erase(types.begin() + pos);
    drop.Invalidate(true);
}

unsigned int ACCustomComboBox::GetCount() const { return texts.size(); }

wxString ACCustomComboBox::GetString(unsigned int n) const
{
    return n < texts.size() ? texts[n] : wxString{};
}

void ACCustomComboBox::SetString(unsigned int n, wxString const &value)
{
    if (n >= texts.size()) return;
    texts[n]  = value;
    drop.Invalidate();
    if (n == drop.GetSelection()) SetLabel(value);
}

wxString ACCustomComboBox::GetItemTooltip(unsigned int n) const
{
    if (n >= texts.size()) return wxString();
    return tips[n];
}

void ACCustomComboBox::SetItemTooltip(unsigned int n, wxString const &value) {
    if (n >= texts.size()) return;
    tips[n] = value;
    if (n == drop.GetSelection()) drop.SetToolTip(value);
}

wxBitmap ACCustomComboBox::GetItemBitmap(unsigned int n) { return icons[n]; }

void ACCustomComboBox::SetItemBitmap(unsigned int n, wxBitmap const &bitmap)
{
    if (n >= texts.size()) return;
    icons[n] = bitmap;
    drop.Invalidate();
}

int ACCustomComboBox::DoInsertItems(const wxArrayStringsAdapter &items,
                            unsigned int                 pos,
                            void **                      clientData,
                            wxClientDataType             type)
{
    if (pos > texts.size()) return -1;
    for (int i = 0; i < items.GetCount(); ++i) {
        texts.insert(texts.begin() + pos, items[i]);
        tips.insert(tips.begin() + pos, wxString{});
        icons.insert(icons.begin() + pos, wxNullBitmap);
        datas.insert(datas.begin() + pos, clientData ? clientData[i] : NULL);
        types.insert(types.begin() + pos, type);
        ++pos;
    }
    drop.Invalidate(true);
    return pos - 1;
}

void *ACCustomComboBox::DoGetItemClientData(unsigned int n) const { return n < texts.size() ? datas[n] : NULL; }

void ACCustomComboBox::DoSetItemClientData(unsigned int n, void *data)
{
    if (n < texts.size())
        datas[n] = data;
}

void ACCustomComboBox::mouseDown(wxMouseEvent &event)
{
    SetFocus();
    if (drop_down) {
        drop.Hide();
    } else if (drop.HasDismissLongTime()) {
        if (GetCount() > 0)   {// [AC:1019270 @2024-06-20] FIX：空的下拉框，闪退
            drop.autoPosition();
            drop_down = true;
            drop.Popup(&drop);
            SetShowTextIco(true);
            wxCommandEvent e(wxEVT_COMBOBOX_DROPDOWN);
            GetEventHandler()->ProcessEvent(e);
        }
    }
}

void ACCustomComboBox::mouseWheelMoved(wxMouseEvent &event)
{
    event.Skip();
    if (drop_down) return;
    auto delta = event.GetWheelRotation() < 0 ? 1 : -1;
    unsigned int n = GetSelection() + delta;
    if (n < GetCount()) {
        SetSelection((int) n);
        sendComboBoxEvent();
    }
}

void ACCustomComboBox::keyDown(wxKeyEvent& event)
{
    switch (event.GetKeyCode()) {
        case WXK_RETURN:
        case WXK_SPACE:
            if (drop_down) {
                drop.DismissAndNotify();
            } else if (drop.HasDismissLongTime()) {
                drop.autoPosition();
                drop_down = true;
                drop.Popup();
                SetShowTextIco(true);
                wxCommandEvent e(wxEVT_COMBOBOX_DROPDOWN);
                GetEventHandler()->ProcessEvent(e);
            }
            break;
        case WXK_UP:
        case WXK_DOWN:
        case WXK_LEFT:
        case WXK_RIGHT:
            if ((event.GetKeyCode() == WXK_UP || event.GetKeyCode() == WXK_LEFT) && GetSelection() > 0) {
                SetSelection(GetSelection() - 1);
            } else if ((event.GetKeyCode() == WXK_DOWN || event.GetKeyCode() == WXK_RIGHT) && GetSelection() + 1 < texts.size()) {
                SetSelection(GetSelection() + 1);
            } else {
                break;
            }
            sendComboBoxEvent();
            break;
        case WXK_TAB:
            HandleAsNavigationKey(event);
            break;
        default:
            event.Skip();
            break;
    }
}

void ACCustomComboBox::onMove(wxMoveEvent &event)
{
    event.Skip();
    drop.Hide();
}

void ACCustomComboBox::OnEdit()
{
    auto value = GetTextCtrl()->GetValue();
    SetValue(value);
}

#ifdef __WIN32__

WXLRESULT ACCustomComboBox::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
    if (nMsg == WM_GETDLGCODE) {
        return DLGC_WANTALLKEYS;
    }
    return TextInput::MSWWindowProc(nMsg, wParam, lParam);
}

#endif

void ACCustomComboBox::sendComboBoxEvent()
{
    wxCommandEvent event(wxEVT_COMBOBOX, GetId());
    event.SetEventObject(this);
    event.SetInt(drop.GetSelection());
    event.SetString(drop.GetValue());
    GetEventHandler()->ProcessEvent(event);
}
