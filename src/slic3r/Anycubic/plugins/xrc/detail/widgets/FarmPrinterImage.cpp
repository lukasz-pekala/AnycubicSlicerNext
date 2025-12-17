#include "FarmPrinterImage.hpp"
#include "slic3r/GUI/Widgets/StateColor.hpp"
#include "slic3r/GUI/Widgets/Label.hpp"
#include "slic3r/GUI/Widgets/StateHandler.hpp"
#include <plugins_sdk/event/detail/program_color.hxx>
#include <wx/mstream.h>
#include "slic3r/GUI/GUI.hpp"
#include <wx/dcgraph.h>
#include <wx/base64.h>
#include "slic3r/GUI/GUI_App.hpp"
#include <plugins_sdk/event/detail/plugin_custom_event.hxx>
#include <plugins_sdk/event/detail/custom_struct_manger.hxx>
#include <plugins_sdk/event/detail/idata_provider_manger.hxx>

BEGIN_EVENT_TABLE(FarmPrinterImageObj, StaticBox)

EVT_PAINT(FarmPrinterImageObj::paintEvent)

END_EVENT_TABLE()

FarmPrinterImageObj::~FarmPrinterImageObj() 
{ 
    m_printerImageProvider->UnreginsterCloudEvent(this);

}

FarmPrinterImageObj::FarmPrinterImageObj(wxWindow* parent,
                                         wxString  imgBase64,
                                         wxSize    winSize,
                                         long      style)
    : m_parent(parent)
    , m_imgBase64(imgBase64)
    , m_winSize(winSize)
{
    init();
}

void FarmPrinterImageObj::init()
{
    StaticBox::Create(m_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    this->SetBackgroundColor(COLOR_Neutral_01);
    imageChangeEvent(m_imgBase64);
    wxWindow::SetMinSize(m_winSize);
    Refresh();
    this->Bind(EVT_DOWN_MODEL_IMG_FINISH_EVENT, [this](wxPluginEvent& evt) {
        wxString baseImg64 = evt.GetString();
        imageChangeEvent(baseImg64);
        Refresh();
    });
    m_printerImageProvider->ReginsterCloudEvent(this);
}


void FarmPrinterImageObj::imageChangeEvent(const wxString& imgBase64) 
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


void FarmPrinterImageObj::paintEvent(wxPaintEvent& evt)
{
    wxPaintDC dc(this);
    render(dc);
}

void FarmPrinterImageObj::render(wxDC& dc)
{
    StaticBox::render(dc);
    dc.SetFont(Label::Head_12);
    wxSize size = GetSize();

    dc.SetPen(StateColor::darkModeColorFor(COLOR_Neutral_03));
    dc.SetBrush(StateColor::darkModeColorFor(COLOR_Neutral_03));
    dc.DrawRoundedRectangle(wxPoint(0, 0), size,FromDIP(6));

    wxPoint pt((size.x - m_modeImageSize.x) / 2, (size.y - m_modeImageSize.y) / 2);
    dc.DrawBitmap(m_modeImage, pt);
}


