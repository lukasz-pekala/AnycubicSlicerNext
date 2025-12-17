#include "ACButton.hpp"
#include "ACStateColor.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"
#include "slic3r/GUI/Widgets/StateHandler.hpp"

#include <wx/dcgraph.h>

#include "slic3r/GUI/Defines.h"

BEGIN_EVENT_TABLE(ACButton, ACStaticBox)

EVT_LEFT_DOWN(ACButton::mouseDown)
EVT_LEFT_UP(ACButton::mouseReleased)
EVT_MOUSE_CAPTURE_LOST(ACButton::mouseCaptureLost)
EVT_KEY_DOWN(ACButton::keyDownUp)
EVT_KEY_UP(ACButton::keyDownUp)

//// catch paint events
//EVT_PAINT(ACButton::paintEvent)

END_EVENT_TABLE()

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

//ACButton::ACButton()
//    : m_paddingSize(10, 10)
//    , m_minSize(-1, -1)
//{
//    //background_color = ACStateColor(
//    //    std::make_pair(0xF0F0F0, (int) ACStateColor::Disabled),
//    //    std::make_pair(0x37EE7C, (int) ACStateColor::Hovered | ACStateColor::Checked),
//    //    std::make_pair(0x00AE42, (int) ACStateColor::Checked),
//    //    std::make_pair(*wxLIGHT_GREY, (int) ACStateColor::Hovered), 
//    //    std::make_pair(*wxWHITE, (int) ACStateColor::Normal));
//    //text_color       = ACStateColor(
//    //    std::make_pair(*wxLIGHT_GREY, (int) ACStateColor::Disabled), 
//    //    std::make_pair(*wxBLACK, (int) ACStateColor::Normal));
//}
static int acButtonCounter = 0;

ACButton::~ACButton()
{
    //pressedDown = false;
    //state_handler.clearAttach();
    //state_handler.update_binds();
    //printf("ACButton Desdroyed.... %d \n", --acButtonCounter);
}


ACButton::ACButton(wxWindow* parent, wxString text, wxString icon, wxString hover_icon, wxString dis_icon, long style, wxSize iconSize)
    : m_paddingSize(10, 10)
    , m_minSize(-1, -1)
{
    acButtonCounter++;
    Create(parent, text, icon,hover_icon, dis_icon, style, iconSize);
}

bool ACButton::Create(wxWindow* parent, wxString text, wxString icon, wxString hover_icon, wxString dis_icon, long style, wxSize iconSize)
{
    ACStaticBox::Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);

    m_iconSize = iconSize;
    if ((style & (int)AC_ALIGN_LEFT) == AC_ALIGN_LEFT)
        m_alignCenter = false;

    if ((style & (int)wxVERTICAL) == wxVERTICAL)
        m_isHorizontal = false;

    state_handler.attach({&text_color});
    state_handler.update_binds();

    //BBS set default font
    SetFont(Label::Head_14);

    wxWindow::SetLabel(text);

    SetIcon(icon);
    SetInactiveIcon(dis_icon.IsEmpty() ? icon : dis_icon);
    SetHoverIcon(hover_icon.IsEmpty() ? icon : hover_icon);

    //setupCheckImg();

    m_sizeValid = false;
    messureSize();
    Refresh();
    return true;
}

void ACButton::setupCheckImg()
{
    m_ckBoxIconSize = wxSize(16,16);

    m_checkOnImg        = ScalableBitmap(this, "checkbox-on-nor", m_ckBoxIconSize.y);
    m_checkOnImgHover   = ScalableBitmap(this,"checkbox-on-focused", m_ckBoxIconSize.y );
    m_checkOnImgDis     = ScalableBitmap(this, "checkbox-on-disable" , m_ckBoxIconSize.y);
    m_checkOffImg       = ScalableBitmap(this, "checkbox-off-nor" , m_ckBoxIconSize.y);
    m_checkOffImgHover  = ScalableBitmap(this, "checkbox-off-focused", m_ckBoxIconSize.y);
    m_checkOffImgDis    = ScalableBitmap(this,"checkbox-off-disable", m_ckBoxIconSize.y);
    m_checkHalfImg      = ScalableBitmap(this, "checkbox-half_on-nor" , m_ckBoxIconSize.y);
    m_checkHalfImgHover = ScalableBitmap(this, "checkbox-half_on-focused", m_ckBoxIconSize.y);
    m_checkHalfImgDis   = ScalableBitmap(this, "checkbox-half_on-disable", m_ckBoxIconSize.y );
}

void ACButton::SetLabel(const wxString& label)
{
    wxWindow::SetLabel(label);
    m_sizeValid = false;
    Refresh();
}

void ACButton::SetIcon(const wxString& iconName)
{
    if (!iconName.IsEmpty()) {
        //BBS set button icon default size to 20
        this->active_icon = ScalableBitmap(this, iconName.ToStdString(), m_iconSize.y);
    }
    else
    {
        this->active_icon = ScalableBitmap();
    }
    m_sizeValid = false;
    Refresh();
}

void ACButton::SetInactiveIcon(const wxString &iconName)
{
    if (!iconName.IsEmpty()) {
        // BBS set button icon default size to 20
        this->inactive_icon = ScalableBitmap(this, iconName.ToStdString(), m_iconSize.y);
    } else {
        this->inactive_icon = ScalableBitmap();
    }
    m_sizeValid = false;
    Refresh();
}

void ACButton::SetHoverIcon(const wxString &iconName)
{
    if (!iconName.IsEmpty()) {
        // BBS set button icon default size to 20
        this->hover_icon = ScalableBitmap(this, iconName.ToStdString(), m_iconSize.y);
    } else {
        this->hover_icon = ScalableBitmap();
    }
    m_sizeValid = false;
    Refresh();
}

void ACButton::SetStdStringIcon(const std::string& iconName)
{
	if (!iconName.empty())
	{
		//BBS set button icon default size to 20
		this->active_icon = ScalableBitmap(this, iconName, m_iconSize.y);
	}
	else
	{
		this->active_icon = ScalableBitmap();
	}
	m_sizeValid = false;
	Refresh();
}

void ACButton::SetStdStringInactiveIcon(const std::string& iconName)
{
	if (!iconName.empty())
	{
		// BBS set button icon default size to 20
		this->inactive_icon = ScalableBitmap(this, iconName, m_iconSize.y);
	}
	else
	{
		this->inactive_icon = ScalableBitmap();
	}
	m_sizeValid = false;
	Refresh();
}

void ACButton::SetStdStringHoverIcon(const std::string& iconName)
{
	if (!iconName.empty())
	{
		// BBS set button icon default size to 20
		this->hover_icon = ScalableBitmap(this, iconName, m_iconSize.y);
	}
	else
	{
		this->hover_icon = ScalableBitmap();
	}
	m_sizeValid = false;
	Refresh();
}


void ACButton::setDropDownStyle(bool index, int type)
{
    m_isDropDownIndex = index;
    m_dropDownScalableBitmapList.clear();
    if (m_isDropDownIndex) {
        if (type == 0) {
            wxString iconName = "icon_dropDwomn_white_up";
            m_dropDownScalableBitmapList.push_back(ScalableBitmap(this, iconName.ToStdString(), m_iconSize.y));
            iconName = "icon_dropDwomn_white_down";
            m_dropDownScalableBitmapList.push_back(ScalableBitmap(this, iconName.ToStdString(), m_iconSize.y));
        } else {
            wxString iconName = "icon_dropDwomn_black_up";
            m_dropDownScalableBitmapList.push_back(ScalableBitmap(this, iconName.ToStdString(), m_iconSize.y));
            iconName = "icon_dropDwomn_black_down";
            m_dropDownScalableBitmapList.push_back(ScalableBitmap(this, iconName.ToStdString(), m_iconSize.y));
        }

    } 
}

void ACButton::SetMinSize(const wxSize& size)
{
    m_minSize = size;
    m_sizeValid = false;
    Refresh();
}

void ACButton::SetPaddingSize(const wxSize& size)
{
    m_paddingSize = size;
    m_sizeValid = false;
    Refresh();
}
    
void ACButton::SetSpacing(const int& value)
{
    m_spacing = value;
    m_sizeValid = false;
    Refresh();
}


void ACButton::SetButtonType(ACButton::AC_BUTTON_TYPE type)
{
    StateColor   bgColor;
    StateColor   bdColor;
    StateColor   fgColor;

    switch (type)
    {
    case ACButton::AC_BUTTON_BULE:
        ACStaticBox::SetBorderWidth(0);
        bgColor.append(COLOR_Blue_05, ACStateColor::Disabled);
        bgColor.append(COLOR_Blue_03, ACStateColor::Pressed);
        bgColor.append(COLOR_Blue_08, ACStateColor::Hovered);
        bgColor.append(COLOR_Blue_03, ACStateColor::Normal);

        bdColor.append(COLOR_Neutral_01, ACStateColor::Disabled);
        bdColor.append(COLOR_Neutral_01, ACStateColor::Pressed);
        bdColor.append(COLOR_Neutral_01, ACStateColor::Hovered);
        bdColor.append(COLOR_Neutral_01, ACStateColor::Normal);


        fgColor.append(COLOR_BLUE_BUTTON_TEXT, ACStateColor::Disabled);
        fgColor.append(COLOR_BLUE_BUTTON_TEXT, ACStateColor::Normal);
        break;
    case ACButton::AC_BUTTON_ICO:
        ACStaticBox::SetBorderWidth(0);
        //bgColor.append(wxColour(0,0,0,0), ACStateColor::Disabled);
        //bgColor.append(wxColour(0, 0, 0, 0), ACStateColor::Normal);

        background_color.clear();

        bdColor.append(COLOR_Neutral_01, ACStateColor::Disabled);
        bdColor.append(COLOR_Neutral_01, ACStateColor::Pressed);
        bdColor.append(COLOR_Neutral_01, ACStateColor::Hovered);
        bdColor.append(COLOR_Neutral_01, ACStateColor::Normal);

        break;
    case ACButton::AC_BUTTON_BLACK:

        bgColor.append(COLOR_Neutral_01, ACStateColor::Disabled);
        bgColor.append(COLOR_Neutral_01, ACStateColor::Pressed);
        bgColor.append(COLOR_Btn_Bg_Hover, ACStateColor::Hovered);
        bgColor.append(COLOR_Neutral_01, ACStateColor::Normal);
        
        
        bdColor.append(COLOR_Neutral_05, ACStateColor::Disabled);
        bdColor.append(COLOR_Btn_Bd_Normal, ACStateColor::Normal);


        fgColor.append(COLOR_Neutral_05, ACStateColor::Disabled);
        fgColor.append(COLOR_Btn_Bd_Normal, ACStateColor::Normal);
        break;
    case ACButton::AC_BUTTON_NOBG_BLUE:
        ACStaticBox::SetBorderWidth(0);
        bgColor.append(COLOR_Blue_01, ACStateColor::Disabled);
        bgColor.append(COLOR_Blue_01, ACStateColor::Pressed);
        bgColor.append(COLOR_Blue_01, ACStateColor::Hovered);
        bgColor.append(COLOR_Blue_01, ACStateColor::Normal);

        bdColor.append(COLOR_Neutral_01, ACStateColor::Disabled);
        bdColor.append(COLOR_Neutral_01, ACStateColor::Pressed);
        bdColor.append(COLOR_Neutral_01, ACStateColor::Hovered);
        bdColor.append(COLOR_Neutral_01, ACStateColor::Normal);

        fgColor.append(COLOR_Blue_05, ACStateColor::Disabled);
        fgColor.append(COLOR_Blue_HOVER, ACStateColor::Hovered);
        fgColor.append(COLOR_Blue_06, ACStateColor::Pressed);
        fgColor.append(COLOR_Blue_06, ACStateColor::Normal);
        
        break;
    default:
        break;
    }
    bgColor.setTakeFocusedAsHovered(false);
    fgColor.setTakeFocusedAsHovered(false);
    bdColor.setTakeFocusedAsHovered(false);
    SetBackgroundColor(bgColor);
    SetBorderColor(bdColor);
    SetTextColor(fgColor);

    Refresh();
}

void ACButton::SetTextColor(StateColor const& color)
{
    text_color = color;
    state_handler.update_binds();
    Refresh();
}

void ACButton::SetTextColorNormal(wxColor const &color)
{
    text_color.setColorForStates(color, 0);
    Refresh();
}

void ACButton::SetCheckStyle(AC_BUTTON_CHECK_STYLE checkStyle) 
{
    m_checkStyle = checkStyle; 
    m_sizeValid = false;
    Refresh(); 
}

void ACButton::SetChecked(AC_BUTTON_CHECK_STATE state, bool sendEvents) 
{
    if (m_checkState == state)
        return;

    m_checkState = state;

    wxCommandEvent ckEvent(wxEVT_CHECKBOX, GetId());
    ckEvent.SetEventObject(this);
    ckEvent.SetInt(m_checkState);
    /*if (sendEvents)
        GetEventHandler()->ProcessEvent(ckEvent);
    else
        this->state_handler.changed(ckEvent);*/

    Refresh();
}

void ACButton::SetChecked(bool selected, bool sendEvents) 
{ 
    AC_BUTTON_CHECK_STATE state = selected ? CHECKSTATE_ON : CHECKSTATE_OFF;

    SetChecked(state, sendEvents);
}
void ACButton::SetHalfChecked(bool sendEvents) 
{ 
    SetChecked(CHECKSTATE_ON_HALF, sendEvents);
}

void ACButton::SetTextAtLeft(bool atLeft)
{
    m_textAtLeft = atLeft;
    Refresh();
}


bool ACButton::Enable(bool enable)
{
    bool result = wxWindow::Enable(enable);
    if (result) {
        wxCommandEvent e(EVT_ENABLE_CHANGED);
        e.SetEventObject(this);
        e.SetInt(enable);
        GetEventHandler()->ProcessEvent(e);
    }
    return result;
}

bool ACButton::SetEnable(bool enable) 
{
    return Enable(enable);
}

void ACButton::SetCanFocus(bool canFocus) { this->canFocus = canFocus; }

void ACButton::Rescale()
{
    if (this->active_icon.bmp().IsOk())
        this->active_icon.msw_rescale();

    if (this->inactive_icon.bmp().IsOk())
        this->inactive_icon.msw_rescale();

    if (this->hover_icon.bmp().IsOk())
        this->hover_icon.msw_rescale();

    m_sizeValid = false;

    Refresh();
}

void ACButton::clearColor()
{
    text_color.clear();
    background_color.clear();
    background_color2.clear();
    border_color.clear();

    state_handler.update_binds();
    Refresh();
}
void ACButton::setTakeFocusedAsHovered(bool as)
{
    text_color       .setTakeFocusedAsHovered(as);
    background_color .setTakeFocusedAsHovered(as);
    background_color2.setTakeFocusedAsHovered(as);
    border_color     .setTakeFocusedAsHovered(as);
    Refresh();
}

void ACButton::sys_color_changed()
{
    Rescale();
}

void ACButton::setDrawCircle(bool canShow) 
{ 
    m_draw_circle = canShow; 
    m_sizeValid = false;
    Refresh();
}

void ACButton::setCircleVisiable(bool show) 
{ 
    m_circle_visiable = show; 
    m_sizeValid = false;
    Refresh();
}

void ACButton::setSizeValid(bool valid, bool calcNow )
{
    m_sizeValid = valid;
    if (calcNow)
        messureSize();
}


//void ACButton::paintEvent(wxPaintEvent& evt)
//{
//    // depending on your system you may need to look at double-buffered dcs
//    wxPaintDC dc(this);
//    render(dc);
//}

/*
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void ACButton::render(wxDC& old_dc)
{
    wxPaintDC dc(this);
    if (m_sizeValid == false)
        messureSize();

    auto text = GetLabel();
    // background
    ACStaticBox::render(old_dc);

    // draw icon and text
    int states = state_handler.states();
    wxSize size = GetSize();
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    // calc content size
    // text
    
    wxSize dipIconSize = FromDIP(m_iconSize);
    wxSize dipPadding = FromDIP(m_paddingSize);
    wxSize dipCkBoxSize = FromDIP(m_ckBoxIconSize);
    float dipSpacing = FromDIP(m_spacing);

    const ScalableBitmap* icon = &active_icon;
    if ((states & (int)ACStateColor::State::Disabled) != 0) {
        icon = &inactive_icon;
    } else if ((states & (int)ACStateColor::State::Hovered) != 0) {
        icon = &hover_icon;
    }

    bool hasIcon = icon->bmp().IsOk();
    bool hasCkBox = checkable() && m_isHorizontal && (m_checkStyle == CHECKSTYLE_ON_BOX || m_checkStyle == CHECKSTYLE_ON_HALF);

    wxRect rcContent = { {0, 0}, size };
    
    if (m_alignCenter) {
		
		if (hasIcon || m_checkStyle == ACButton::CHECKSTYLE_ON_BOX) {
            rcContent.Deflate((size - m_szContent) / 2);
        } else {
            rcContent.Deflate((size - m_szText) / 2);
        }
    } else {
        rcContent.Deflate(dipPadding);
    }
    // start draw
    if (hasCkBox && !m_textAtLeft) {
        rcContent.x += dipCkBoxSize.GetX() + dipSpacing;
    }
    // icon
    wxPoint pt = rcContent.GetLeftTop();
    if (hasIcon) {
        wxBitmap bmp = icon->bmp();

        if (m_isHorizontal) {
            if (m_icoAtLeft)
                pt.x = dipSpacing;
            pt.y += (rcContent.height - dipIconSize.y) / 2;
            dc.DrawBitmap(bmp, pt);

            pt.x += dipIconSize.x + dipSpacing;
            pt.y = rcContent.GetTop();
        } else {
            pt.x += (rcContent.width - dipIconSize.x) / 2;
            dc.DrawBitmap(bmp, pt);

            pt.x = rcContent.GetLeft();
            pt.y = rcContent.GetTop() + dipIconSize.y + dipSpacing;
        }
    }

    if (!text.IsEmpty()) {
        if (m_isHorizontal) {
            if (m_icoAtLeft) {
                pt.x = pt.x + (size.x - pt.x - dc.GetTextExtent(text).x) / 2;
            }
            pt.y += (rcContent.height - m_szText.y) / 2;
        }
        else { 
            pt.x += (rcContent.width - m_szText.x) / 2;
        }

        int textValidSpace = rcContent.GetWidth() - ((hasIcon&&m_isHorizontal) ? (dipIconSize.GetX()+dipSpacing) : 0) - (hasCkBox ? (dipCkBoxSize.GetX() + dipSpacing) : 0);
        if (m_szText.x > textValidSpace)
            text = wxControl::Ellipsize(text, dc, wxELLIPSIZE_END, textValidSpace);

        dc.SetFont(GetFont());
        dc.SetTextForeground(text_color.colorForStates(states));
        if (!firstPart.empty() && !secondPart.empty()) {
            dc.DrawText(firstPart, pt);
            pt.y +=  m_szText.y;
            int secondWidth = dc.GetTextExtent(secondPart).x;
            pt.x            += (dc.GetTextExtent(firstPart).x  - secondWidth) / 2;
            dc.DrawText(secondPart, pt);
        } else {
            dc.DrawText(text, pt);
        }
    }
    if (m_draw_circle && m_circle_visiable) {
        int dipCircleSize = FromDIP(m_circleSize);
        wxPoint circle_pos(size.x - dipPadding.x - dipCircleSize / 2, size.y / 2);
        old_dc.SetBrush(text_color.colorForStates(states));
        old_dc.DrawCircle(circle_pos, dipCircleSize / 2);
    }

    if (checkable()) {
        if (m_checkStyle == CHECKSTYLE_ON_MARK) {
            if (GetChecked()) // show mark on selected
            {
                // mark
                ScalableBitmap* sbmp = (states & (int)ACStateColor::State::Hovered) ? &m_checkedMarkImgHover : &m_checkedMarkImg; 
                if (sbmp->bmp().IsOk()) {
                    wxSize  chMarkSize = sbmp->GetBmpSize();
                    if (m_isConfigWizardIndex)
                        chMarkSize = m_ckMarkIconSize;
                    wxPoint pt(size.x - chMarkSize.x - 1, 1);
                    dc.DrawBitmap(sbmp->bmp(), pt);
                }
            }
        } else if (m_checkStyle == CHECKSTYLE_ON_BOX || m_checkStyle == CHECKSTYLE_ON_HALF){
            if (m_isHorizontal) 
            {
                ScalableBitmap* sbmp = nullptr;

                switch (m_checkState)
                {
                case CHECKSTATE_ON:
                    sbmp = (states & (int)ACStateColor::State::Disabled) ?  &m_checkOnImgDis : (states & (int)ACStateColor::State::Hovered) ? &m_checkOnImgHover : &m_checkOnImg;
                    break;
                case CHECKSTATE_ON_HALF:
                    sbmp = (states & (int)ACStateColor::State::Disabled) ?  &m_checkHalfImgDis : (states & (int)ACStateColor::State::Hovered) ? &m_checkHalfImgHover : &m_checkHalfImg;
                    break;
                default: // no check
                    sbmp = (states & (int)ACStateColor::State::Disabled) ?  &m_checkOffImgDis : (states & (int)ACStateColor::State::Hovered) ? &m_checkOffImgHover : &m_checkOffImg;
                    break;
                }

                if (sbmp->bmp().IsOk()) {

                    if (m_textAtLeft) {
                        pt.x = size.x - dipPadding.x - dipCkBoxSize.x;
                    } else {
                        pt.x = dipPadding.x;
                    }
                    pt.y = (size.y- dipCkBoxSize.y)/2;

                    dc.DrawBitmap(sbmp->bmp(), pt);
                }         
            }
        }



    }

    if (m_isDropDownIndex && m_dropDownScalableBitmapList.size() > 0) 
    {
        wxBitmap bmp;
        wxSize   dropDwonSize(FromDIP(20), FromDIP(14));
        if (m_isDropIndex) {
            bmp = m_dropDownScalableBitmapList[0].bmp();
        } else {
            bmp = m_dropDownScalableBitmapList[1].bmp();
        }
        pt.x = size.x - FromDIP(8) - dropDwonSize.x;
        pt.y = size.y - FromDIP(8) - dropDwonSize.y;
        dc.DrawBitmap(bmp, pt);
    }
    
    if (left_topNum != -1) {
        dc.SetFont(GetFont());
        
        wxPoint circlePoint(FromDIP(18), FromDIP(15));

        wxString left_topText = wxString::FromDouble(left_topNum);
        wxSize   textSize     = dc.GetTextExtent(left_topText);
        wxPoint  start_pos(circlePoint.x - textSize.x / 2, circlePoint.y - textSize.y/2);
        dc.SetPen(left_topColor);
        dc.DrawText(left_topText, start_pos);
        dc.DrawCircle(circlePoint, FromDIP(10));
    }
}

void ACButton::messureSize()
{
    wxClientDC dc(this);

    wxSize dipPadding = FromDIP(m_paddingSize);
    float dipSpacing = FromDIP(m_spacing);

    int states = state_handler.states();
    const ScalableBitmap* icon = &active_icon;
    if ((states & (int)ACStateColor::State::Disabled) != 0) {
        icon = &inactive_icon;
    } else if ((states & (int)ACStateColor::State::Hovered) != 0) {
        icon = &hover_icon;
    }

    wxString text = GetLabel();
    if (!text.IsEmpty()) {
        wxSize textSize; 
        if (m_fixedWidth > 0 && text.find("\n") != wxString::npos) {
            size_t secondSpacePos = text.find('\n');
            firstPart  = text.substr(0, secondSpacePos);
            if (dc.GetTextExtent(firstPart).x > textSize.x)
                textSize = dc.GetTextExtent(firstPart);
            secondPart = text.substr(secondSpacePos + 1);
            if (dc.GetTextExtent(secondPart).x > textSize.x)
                textSize = dc.GetTextExtent(secondPart);
        } else {
            textSize = dc.GetTextExtent(text);
        }
        m_szText = textSize;
    } else {
        m_szText = wxSize(0,0);
    }

    wxSize szIcon(0,0);
    if (icon->bmp().IsOk()) {
        wxSize dipIconSize = FromDIP(m_iconSize);
        szIcon = dipIconSize;
    }

    m_szContent = szIcon;

    if (m_isHorizontal) {
        if (m_szText.x > 0) {
            m_szContent.x = m_szContent.x + dipSpacing + m_szText.x;
        }

        if (m_szText.y > m_szContent.y)
            m_szContent.y = m_szText.y;

        if (m_checkStyle == CHECKSTYLE_ON_BOX || m_checkStyle == CHECKSTYLE_ON_HALF) {
            wxSize dipSize = FromDIP(m_ckBoxIconSize);
            m_szContent.x = m_szContent.x + dipSpacing + dipSize.x;
            m_szContent.y = std::max(m_szContent.y, dipSize.y);
        }
    } else {
        if (m_szText.y > 0) {
            m_szContent.y = m_szContent.y + dipSpacing + m_szText.y;
        }

        if (m_szText.x > m_szContent.x)
            m_szContent.x = m_szText.x;
    }

    if (m_draw_circle) {
        m_szContent.x += dipSpacing + FromDIP(m_circleSize);
    }
    if (m_fixedWidth > 0 && m_szContent.x > 0) {
        m_szContent.x = m_fixedWidth;
    }
    wxSize minSize = m_szContent + dipPadding * 2;

    minSize.x = std::max(minSize.x, FromDIP(m_minSize.x));
    minSize.y = std::max(minSize.y, FromDIP(m_minSize.y));

    wxSize curSize = wxWindow::GetSize();
    wxWindow::SetMinSize(minSize);

    if (curSize.x < minSize.x || curSize.y < minSize.y) {
        wxWindow::SetSize(std::max(curSize.x, minSize.x), std::max(curSize.y, minSize.y));
    }

    m_sizeValid = true;
}

void ACButton::mouseDown(wxMouseEvent& event)
{
    event.Skip();
    pressedDown = true;
    if (canFocus)
        SetFocus();
    CaptureMouse();
}

void ACButton::mouseReleased(wxMouseEvent& event)
{
    event.Skip();
    if (pressedDown) 
    {
        pressedDown = false;
        if (HasCapture())
            ReleaseMouse();
        if (wxRect({0, 0}, GetSize()).Contains(event.GetPosition()))
            sendButtonEvent();
    }
}

void ACButton::mouseCaptureLost(wxMouseCaptureLostEvent &event)
{
    /*wxMouseEvent evt;
    mouseReleased(evt);*/
    wxMouseEvent evt(wxEVT_LEFT_UP);
    event.SetEventObject(this);
    GetEventHandler()->ProcessEvent(evt);
}

void ACButton::keyDownUp(wxKeyEvent &event)
{
    if (event.GetKeyCode() == WXK_SPACE || event.GetKeyCode() == WXK_RETURN) {
        wxMouseEvent evt(event.GetEventType() == wxEVT_KEY_UP ? wxEVT_LEFT_UP : wxEVT_LEFT_DOWN);
        event.SetEventObject(this);
        GetEventHandler()->ProcessEvent(evt);
        return;
    }
    if (event.GetEventType() == wxEVT_KEY_DOWN &&
        (event.GetKeyCode() == WXK_TAB || event.GetKeyCode() == WXK_LEFT || event.GetKeyCode() == WXK_RIGHT 
        || event.GetKeyCode() == WXK_UP || event.GetKeyCode() == WXK_DOWN))
        HandleAsNavigationKey(event);
    else
        event.Skip();
}

void ACButton::sendButtonEvent()
{
    if (checkable()) {    
        AC_BUTTON_CHECK_STATE checkState = AC_BUTTON_CHECK_STATE(m_checkStyle == CHECKSTYLE_ON_HALF ? (m_checkState+1)%3 : (m_checkState+1)%2);
        SetChecked(checkState, true);
    }

    Refresh();
    wxCommandEvent event(wxEVT_BUTTON, GetId());
    event.SetEventObject(this);
    GetEventHandler()->ProcessEvent(event);
}

#ifdef __WIN32__

WXLRESULT ACButton::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
    if (nMsg == WM_GETDLGCODE) { return DLGC_WANTMESSAGE; }
    if (nMsg == WM_KEYDOWN) {
        wxKeyEvent event(CreateKeyEvent(wxEVT_KEY_DOWN, wParam, lParam));
        switch (wParam) {
        case WXK_RETURN: { // WXK_RETURN key is handled by default button
            GetEventHandler()->ProcessEvent(event);
            return 0;
        }
        }
    }
    return wxWindow::MSWWindowProc(nMsg, wParam, lParam);
}

#endif

bool ACButton::AcceptsFocus() const { return canFocus; }


void ACButton::setCheckedMarkImg(const wxString& imgNameCheckedOn, const wxString& imgNameCheckedHover, int imgSize)
{
    m_ckMarkIconSize = wxSize(imgSize, imgSize);
    if (!imgNameCheckedOn.IsEmpty()) {
        m_checkedMarkImg = ScalableBitmap(this, imgNameCheckedOn.ToStdString(), imgSize);
    } else {
        m_checkedMarkImg = ScalableBitmap();
    }
    if (!imgNameCheckedHover.IsEmpty()) {
        m_checkedMarkImgHover = ScalableBitmap(this, imgNameCheckedHover.ToStdString(), imgSize);
    } else {
        m_checkedMarkImgHover = ScalableBitmap();
    }
    m_sizeValid = true;
    Refresh();
}

