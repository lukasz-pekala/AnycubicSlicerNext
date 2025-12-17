#ifndef slic3r_GUI_Anycubic_RemotePrinting_FarmPrinterImage_hpp_
#define slic3r_GUI_Anycubic_RemotePrinting_FarmPrinterImage_hpp_

#include "slic3r/GUI/wxExtensions.hpp"
#include "slic3r/GUI/Widgets/StaticBox.hpp"
#include <wx/event.h>
#include <wx/wx.h>
#include <plugins_sdk/event/detail/idata_provider_manger.hxx>

class FarmPrinterImageObj : public wxNavigationEnabled<StaticBox>
{

public:
    FarmPrinterImageObj(wxWindow* parent,
                         wxString  imgBase64,
                         wxSize    winSize,
                         long      style      = 0);

    ~FarmPrinterImageObj();

    void init();

    void    render(wxDC& dc);
    void    paintEvent(wxPaintEvent& evt);
    void    imageChangeEvent(const wxString& imgBase64);

    DECLARE_EVENT_TABLE()

private:
    wxWindow *m_parent;
    wxSize         m_winSize;
    wxString       m_imgBase64;
    wxBitmap       m_modeImage;
    wxSize         m_modeImageSize;

private:
    IDataProvider_printerImage* m_printerImageProvider;

public:
    void IDataProvider_RegisterprinterImage(IDataProvider_printerImage* view) { m_printerImageProvider = view; }

    void IDataProvider_UnregisterprinterImage() { m_printerImageProvider = nullptr; }
};



#endif // !slic3r_GUI_FarmPrinterImage_hpp_
