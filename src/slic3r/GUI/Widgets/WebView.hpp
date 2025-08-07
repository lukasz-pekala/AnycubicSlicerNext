#ifndef slic3r_GUI_WebView_hpp_
#define slic3r_GUI_WebView_hpp_
#ifdef PLUGINS
#include "webview.h"
#else
#include <webview/webview.h>
#endif 

#include <functional>
class WebView
{
public:
    static wxWebView *CreateWebView(wxWindow *parent, wxString const &url, wxWebViewConfiguration*conf=nullptr,const std::function<void(wxWebView*)>& visitor=nullptr);
#if USE_WEBVIEW_EDGE
    static bool CheckWebViewRuntime();
    static bool DownloadAndInstallWebViewRuntime();
#endif
    static void LoadUrl(wxWebView * webView, wxString const &url);

    static bool RunScript(wxWebView * webView, wxString const & msg);

    static void RecreateAll();
};

#endif // !slic3r_GUI_WebView_hpp_
