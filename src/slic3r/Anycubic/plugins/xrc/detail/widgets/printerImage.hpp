#ifndef slic3r_GUI_Anycubic_RemotePrinting_PrinterImage_hpp_
#define slic3r_GUI_Anycubic_RemotePrinting_PrinterImage_hpp_

#include "slic3r/GUI/wxExtensions.hpp"
#include "slic3r/GUI/Widgets/StaticBox.hpp"
#include <wx/event.h>
#include <wx/wx.h>
#include <plugins_sdk/event/detail/idata_provider_manger.hxx>

class PrinterImageObj : public wxNavigationEnabled<StaticBox>
{

public:
    PrinterImageObj(wxWindow *parent,
                         wxString  imgBase64,
                         wxString  modelName,
                         wxString  printTimes,
                         wxString  printFilamentLength,
                         wxString    printFilamentWeight,
                         wxSize      winSize,
                         long      style      = 0);

    ~PrinterImageObj();

    void init();

    void    render(wxDC& dc);
    void    paintEvent(wxPaintEvent& evt);
    void    imageChangeEvent(const wxString& imgBase64);

    DECLARE_EVENT_TABLE()

private:
    wxWindow *m_parent;
    wxSize         m_winSize;
    wxString       m_imgBase64;
    wxString       m_modelName;
    wxString    m_printTimes;
    wxString    m_printFilamentLength;
    wxString    m_printFilamentWeight;
    wxBitmap       m_modeBgImage;
    wxBitmap       m_modeBgTextImage;
    wxBitmap       m_modeImage;
    wxSize         m_modeImageSize;
    wxSize         m_modeBgTextImageSize;
    wxBitmap       m_timeIco;
    wxSize         m_timeIcoSize;
    wxBitmap       m_filamentIco;
    wxSize         m_filamentIcoSize;
    wxString       m_showFilamenStr;
    wxString       m_showForStr;


private:
    IDataProvider_printerImage* m_printerImageProvider;

public:
    void IDataProvider_RegisterprinterImage(IDataProvider_printerImage* view) { m_printerImageProvider = view; }

    void IDataProvider_UnregisterprinterImage() { m_printerImageProvider = nullptr; }
   
};



#endif // !slic3r_GUI_PrinterImage_hpp_
