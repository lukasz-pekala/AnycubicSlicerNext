#include "RemoteSlotColorPanel.hpp"
#include "slic3r/GUI/Widgets/StateColor.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"
#include "slic3r/GUI/Widgets/StateHandler.hpp"
#include <plugins_sdk/event/detail/util_tool.hxx>
#include <plugins_sdk/event/detail/program_color.hxx>
#include <wx/mstream.h>
#include "slic3r/GUI/GUI.hpp"
#include <wx/dcgraph.h>
#include <wx/base64.h>

BEGIN_EVENT_TABLE(RemoteSlotColorPanel, StaticBox)

EVT_PAINT(RemoteSlotColorPanel::paintEvent)
END_EVENT_TABLE()





RemoteSlotColorPanel::RemoteSlotColorPanel(wxWindow* parent, wxVector<AmsSlotObjInfo> sltInfo,
                                           wxSize winSize, int id, bool showSingle, long style)
    : m_parent(parent), m_sltInfo(sltInfo), m_winSize(winSize),m_id(id)
{
    init();
    BindEvent(showSingle);
}

RemoteSlotColorPanel::~RemoteSlotColorPanel() 
{
    m_colorPanelProvider->UnreginsterCloudEvent(this);
}

void RemoteSlotColorPanel::BindEvent(bool showSingle)
{
    Bind(wxEVT_ENTER_WINDOW, [this](wxMouseEvent& evt) { wxPostEvent(m_parent, evt); });
    Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent& evt) { wxPostEvent(m_parent, evt); });

    if (!showSingle) {
        Bind(EVT_AMS_SHOW_GUI_CHANGE_EVENT, [this](wxPluginEvent& evt) {
            auto* clientData = static_cast<AmsBoxObj*>(evt.GetSharedData());

            const auto& ids = clientData->m_ids;

            if (std::find(ids.begin(), ids.end(), m_id) != ids.end()) {
                if (clientData->id == m_id) {
                    if (!IsShown())
                        Show(true);
                    m_sltInfo = clientData->slotInfo;
                    Refresh();
                }
            } else {
                if (IsShown())
                    Show(false);
            }
                
            
        });
    }
}

void RemoteSlotColorPanel::init()
{
    StaticBox::Create(m_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    this->SetBackgroundColor(COLOR_PRINTER_AMS_BG);
    this->SetBorderWidth(0);

    wxWindow::SetMinSize(m_winSize);
    Refresh();
    m_colorPanelProvider->ReginsterCloudEvent(this);
}

void RemoteSlotColorPanel::paintEvent(wxPaintEvent& evt)
{
    wxPaintDC dc(this);
    render(dc);
}

void RemoteSlotColorPanel::render(wxDC& dc)
{
    // background
    StaticBox::render(dc);
    wxPaintDC paindc(this);
    wxSize             size = GetSize();
    wxGraphicsContext* gc   = wxGraphicsContext::Create(paindc);
    if (!gc)
        return;

    wxGCDC gcdc_dc(paindc);
    gcdc_dc.SetFont(Label::Head_13);

    wxRect   rc(0, 0, size.x, size.y);
    wxColour bgColor   = StateColor::darkModeColorFor(COLOR_PRINTER_AMS_BG);
    int      dipRadius = FromDIP(6);

    gc->SetBrush(bgColor);
    gc->SetPen(bgColor);
    gc->DrawRoundedRectangle(rc.x, rc.y, rc.width, rc.height, dipRadius);

    wxSize rectSize(FromDIP(17), FromDIP(20));
    int    start_y    = size.y / 2 - rectSize.y / 2;
    int    start_x    = 0;
    int    rectRadius = FromDIP(4);

    wxString questionSyt = "?";
    wxSize   textSize;
    gcdc_dc.GetTextExtent(questionSyt, &textSize.x, &textSize.y);

    for (int i = 0; i < m_sltInfo.size(); ++i) {
        const auto& obj    = m_sltInfo[i];
        int         rect_x = (i == 0) ? FromDIP(6) : start_x + (rectSize.x + FromDIP(8));
        start_x            = rect_x;

        wxRect slot_rc(rect_x, start_y, rectSize.x, rectSize.y);
        double rx = slot_rc.x, ry = slot_rc.y, rw = slot_rc.width, rh = slot_rc.height;
        int    showColorType = obj.iconType;
        bool   isShowSlider  = false;
        if (obj.filament_type == "?") {

            gcdc_dc.SetTextForeground(COLOR_Ams_Box_borderColor);
            wxPoint textPt(rx + (rw - textSize.x) / 2, ry + (rh - textSize.y) / 2);
            gcdc_dc.DrawText(questionSyt, textPt);
        } else if (obj.filament_type == "-") {
            wxRect minus_rc(rect_x + FromDIP(4), start_y + (rectSize.y / 2) - FromDIP(1), FromDIP(10), FromDIP(2));
            gc->SetBrush(COLOR_Neutral_05);
            gc->SetPen(COLOR_Neutral_05);
            gc->DrawRectangle(minus_rc.x, minus_rc.y, minus_rc.width, minus_rc.height);

        } else {
            isShowSlider = true;
            if (showColorType == 1 || showColorType == 2) {
                DrawRoundedRectangleColorEvent(gc, slot_rc, obj.skuColors, rectRadius, showColorType == 1 ? false : true,true);
            } else {
                gc->SetBrush(obj.filamentColorInfo);
                gc->SetPen(obj.filamentColorInfo);
                gc->DrawRoundedRectangle(slot_rc.x, slot_rc.y, slot_rc.width, slot_rc.height, rectRadius);
            }
        }

        wxGraphicsPath path = gc->CreatePath();
        path.AddRoundedRectangle(rx, ry, rw, rh, rectRadius);
        gc->SetBrush(*wxTRANSPARENT_BRUSH);
        gc->SetPen(showColorType == 3 && isShowSlider ? wxPen(obj.skuColors[1], 3) : wxPen(COLOR_Ams_Box_borderColor, 2));
        gc->StrokePath(path);
    }

    delete gc;
}
