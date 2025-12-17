#include "printerImage.hpp"
#include "slic3r/GUI/Widgets/StateColor.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"
#include "slic3r/GUI/Widgets/StateHandler.hpp"
#include <plugins_sdk/event/detail/program_color.hxx>
#include <plugins_sdk/event/detail/plugin_custom_event.hxx>
#include <wx/mstream.h>
#include "slic3r/GUI/GUI.hpp"
#include <wx/dcgraph.h>
#include <wx/base64.h>
#include "slic3r/GUI/GUI_App.hpp"

BEGIN_EVENT_TABLE(PrinterImageObj, StaticBox)

EVT_PAINT(PrinterImageObj::paintEvent)

END_EVENT_TABLE()

PrinterImageObj::~PrinterImageObj() { m_printerImageProvider->UnreginsterCloudEvent(this); }

PrinterImageObj::PrinterImageObj(wxWindow* parent,
                                 wxString  imgBase64,
                                 wxString  modelName,
                                 wxString  printTimes,
                                 wxString  printFilamentLength,
                                 wxString  printFilamentWeight,
                                 wxSize    winSize,
                                 long      style)
    : m_parent(parent)
    , m_imgBase64(imgBase64)
    , m_printTimes(printTimes)
    , m_modelName(modelName)
    ,m_printFilamentLength(printFilamentLength)
    ,m_printFilamentWeight(printFilamentWeight)
    , m_winSize(winSize)
{
    init();
}

void PrinterImageObj::init()
{
    StaticBox::Create(m_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    this->SetBackgroundColor(COLOR_Neutral_01);
    wxString bgName            = "ico_print_objec_bg";
    wxString bgTextName = "ico_print_objec_bg_text";
    m_modeBgTextImageSize      = FromDIP(wxSize(124, 60));
    m_modeBgTextImage          = ScalableBitmap(this, bgTextName.ToStdString(),60).bmp();
    m_modeBgImage              = ScalableBitmap(this, bgName.ToStdString(), m_winSize.y).bmp();

    wxImage bgImage         = m_modeBgImage.ConvertToImage();
    bgImage.Rescale(m_winSize.x, m_winSize.y, wxIMAGE_QUALITY_HIGH);
    m_modeBgImage            = wxBitmap(bgImage);

    m_showFilamenStr           = wxString::Format("%s/%s", m_printFilamentWeight, m_printFilamentLength);
    
    imageChangeEvent(m_imgBase64);
    m_showForStr             = "for";
    wxString filamentIcoName = "ico_print_filament";
    m_timeIcoSize            = FromDIP(wxSize(24, 24));
    m_filamentIcoSize        = FromDIP(wxSize(24, 24));
    wxString timeName = "icon_print_time";
    m_timeIco                = ScalableBitmap(this, timeName.ToStdString(), 24).bmp();
    m_filamentIco            = ScalableBitmap(this, filamentIcoName.ToStdString(),24).bmp();
    wxWindow::SetMinSize(m_winSize);
    Refresh();
    this->Bind(EVT_DOWN_MODEL_IMG_FINISH_EVENT, [this](wxPluginEvent& evt) {
        wxString baseImg64 = evt.GetString();
        imageChangeEvent(baseImg64);
        Refresh();
    });
    m_printerImageProvider->ReginsterCloudEvent(this);
}

void PrinterImageObj::imageChangeEvent(const wxString& imgBase64) 
{
    if (imgBase64.length() > 0) {
        wxMemoryBuffer      buffer = wxBase64Decode(imgBase64);
        wxMemoryInputStream stream(buffer.GetData(), buffer.GetDataLen());
        wxImage             image(stream, wxBITMAP_TYPE_ANY);
        wxSize              modeImageSize = image.GetSize();
        m_modeImageSize                   = modeImageSize;

        if (modeImageSize.x > m_winSize.x || modeImageSize.y > m_winSize.y) {
            double aspectRatio = static_cast<double>(modeImageSize.x) / static_cast<double>(modeImageSize.y);

            if (aspectRatio > 1.0) {
                m_modeImageSize.x = m_winSize.x;
                m_modeImageSize.y = static_cast<int>(m_winSize.x / aspectRatio);
            } else {
                m_modeImageSize.y = m_winSize.y;
                m_modeImageSize.x = static_cast<int>(m_winSize.y * aspectRatio);
            }
            image.Rescale(m_modeImageSize.x, m_modeImageSize.y, wxIMAGE_QUALITY_HIGH);
        }

        wxBitmap bitmap(image);
        m_modeImage = bitmap;
    }
}



void PrinterImageObj::paintEvent(wxPaintEvent& evt)
{
    wxPaintDC dc(this);
    render(dc);
}

void PrinterImageObj::render(wxDC& dc)
{
    // background
    StaticBox::render(dc);
    dc.SetFont(Label::Head_12);
    wxSize size = GetSize();

    wxPoint pt((size.x - m_modeImageSize.x) / 2, (size.y - m_modeImageSize.y) / 2);
    wxPoint bgPt(0, 0);
    dc.DrawBitmap(m_modeBgImage, bgPt);
    dc.DrawBitmap(m_modeImage, pt);

    wxPoint textForPt = wxPoint(FromDIP(16), FromDIP(10));

    wxSize  forTextSize = dc.GetTextExtent(m_showForStr);
    wxPoint textModelPt = wxPoint(FromDIP(6) + textForPt.x + forTextSize.x, textForPt.y);


    wxColour bgColor;
    dc.GetPixel(textForPt.x, textForPt.y, &bgColor);

    wxColour simulated = wxColour(167,175,189);
    wxColour simulated_2 = wxColour(221,224,230);

    dc.SetPen(simulated);
    dc.SetTextForeground(simulated);
    dc.DrawText(m_showForStr, textForPt);
    dc.SetPen(simulated_2);
    dc.SetTextForeground(simulated_2);
    dc.SetFont(Label::Body_12);
    dc.DrawText(m_modelName, textModelPt);



    wxSize  filamentTextSize = dc.GetTextExtent(m_showFilamenStr);
    wxSize  timeTextSize     = dc.GetTextExtent(m_printTimes);

    wxPoint textBgPt         = wxPoint(FromDIP(2), size.y - m_modeBgTextImageSize.y - FromDIP(2));
    wxSize  time_icoSize     = m_timeIcoSize;
    wxSize  filament_icoSize = m_filamentIcoSize;
    int     icoSum_y         = time_icoSize.y + filament_icoSize.y + FromDIP(4);

    int center_y = (m_modeBgTextImageSize.y - icoSum_y) / 2;




    wxPoint timeIcoPt = wxPoint(textBgPt.x + FromDIP(4), textBgPt.y + center_y);

    
    wxPoint filamentIcoPt = wxPoint(timeIcoPt.x, size.y - FromDIP(4) - filament_icoSize.y);


    wxPoint timeTextpt   = wxPoint(timeIcoPt.x + time_icoSize.x + FromDIP(4), timeIcoPt.y + (time_icoSize.y - timeTextSize.y) / 2);

    
    wxPoint filamentTextpt   = wxPoint(filamentIcoPt.x + filament_icoSize.x + FromDIP(4),
                                     filamentIcoPt.y + (filament_icoSize.y - filamentTextSize.y) / 2);

    
    
    dc.DrawBitmap(m_modeBgTextImage, textBgPt);
    

    dc.SetPen(COLOR_Neutral_08);
    dc.SetTextForeground(COLOR_Neutral_08);
    dc.DrawText(m_showFilamenStr, filamentTextpt);
    dc.DrawText(m_printTimes, timeTextpt);
    dc.DrawBitmap(m_timeIco, timeIcoPt);
    dc.DrawBitmap(m_filamentIco, filamentIcoPt);
}


