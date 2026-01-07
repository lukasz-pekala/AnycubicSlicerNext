#include "WebView.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/Utils/MacDarkMode.hpp"

#include <boost/log/trivial.hpp>

#include <webview/webviewarchivehandler.h>
#include <webview/webviewfshandler.h>
#if USE_WEBVIEW_EDGE
#include <webview/msw/webview_edge.h>
#elif defined(__WXMAC__)
#include <webview/osx/webview_webkit.h>
#endif
#include <wx/uri.h>
#if defined(__WXMSW__) || defined(__WXMAC__)
#include "webview/private/jsscriptwrapper.h"
#endif

#ifdef __WXMSW__
#include <Shellapi.h>
#include <slic3r/Utils/Http.hpp>
#elif defined __linux__
#include <gtk/gtk.h>
#define WEBKIT_API
struct WebKitWebView;
struct WebKitJavascriptResult;
extern "C" {
WEBKIT_API void
webkit_web_view_run_javascript                       (WebKitWebView             *web_view,
                                                      const gchar               *script,
                                                      GCancellable              *cancellable,
                                                      GAsyncReadyCallback       callback,
                                                      gpointer                  user_data);
WEBKIT_API WebKitJavascriptResult *
webkit_web_view_run_javascript_finish                (WebKitWebView             *web_view,
                                                      GAsyncResult              *result,
						      GError                    **error);
WEBKIT_API void
webkit_javascript_result_unref              (WebKitJavascriptResult *js_result);
}
#endif

#ifdef __WXMSW__
// Run Download and Install in another thread so we don't block the UI thread
DWORD DownloadAndInstallWV2RT() {

  int returnCode = 2; // Download failed
  // Use fwlink to download WebView2 Bootstrapper at runtime and invoke installation
  // Broken/Invalid Https Certificate will fail to download
  // Use of the download link below is governed by the below terms. You may acquire the link
  // for your use at https://developer.microsoft.com/microsoft-edge/webview2/. Microsoft owns
  // all legal right, title, and interest in and to the WebView2 Runtime Bootstrapper
  // ("Software") and related documentation, including any intellectual property in the
  // Software. You must acquire all code, including any code obtained from a Microsoft URL,
  // under a separate license directly from Microsoft, including a Microsoft download site
  // (e.g., https://developer.microsoft.com/microsoft-edge/webview2/).
  // HRESULT hr = URLDownloadToFileW(NULL, L"https://go.microsoft.com/fwlink/p/?LinkId=2124703",
  //                               L".\\plugin\\MicrosoftEdgeWebview2Setup.exe", 0, 0);
  fs::path target_file_path = (fs::temp_directory_path() / "MicrosoftEdgeWebview2Setup.exe");
  bool downloaded = false;
  Slic3r::Http::get("https://go.microsoft.com/fwlink/p/?LinkId=2124703")
      .on_error([](std::string body, std::string error, unsigned http_status) {

      })
      .on_complete([&downloaded, target_file_path](std::string body, unsigned http_status) {
        fs::fstream file(target_file_path, std::ios::out | std::ios::binary | std::ios::trunc);
        file.write(body.c_str(), body.size());
        file.flush();
        file.close();

        downloaded = true;
      })
      .perform_sync();
  // Sleep for 1 second to wait for the buffer writen into disk
  std::this_thread::sleep_for(1000ms);
  if (downloaded) {
    // Either Package the WebView2 Bootstrapper with your app or download it using fwlink
    // Then invoke install at Runtime.
    SHELLEXECUTEINFOW shExInfo = {0};
    shExInfo.cbSize = sizeof(shExInfo);
    shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExInfo.hwnd = 0;
    shExInfo.lpVerb = L"runas";
    shExInfo.lpFile = target_file_path.generic_wstring().c_str();
    shExInfo.lpParameters = L" /install";
    shExInfo.lpDirectory = 0;
    shExInfo.nShow = 0;
    shExInfo.hInstApp = 0;

    if (ShellExecuteExW(&shExInfo)) {
      WaitForSingleObject(shExInfo.hProcess, INFINITE);
      returnCode = 0; // Install successfull
    } else {
      returnCode = 1; // Install failed
    }
  }
  return returnCode;
}
#endif // __WXMSW__




static std::vector<wxWebView*> g_webviews;
static std::vector<wxWebView*> g_delay_webviews;

class WebViewRef : public wxObjectRefData
{
public:
    WebViewRef(wxWebView *webView) : m_webView(webView) {}
    ~WebViewRef() {
        auto iter = std::find(g_webviews.begin(), g_webviews.end(), m_webView);
        assert(iter != g_webviews.end());
        if (iter != g_webviews.end())
            g_webviews.erase(iter);
    }
    wxWebView *m_webView;
};
static wxString CustomUserAgent()
{
#if defined(__WXMAC__)
    static const wxString webUserAgent = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/18.5 Safari/605.1.15";
#elif defined(__WXGTK__)
    static const wxString webUserAgent = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/605.1.15 (KHTML, like Gecko) Chrome/140.0.0.0 Safari/537.36";
#else
    static const wxString webUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/605.1.15 (KHTML, like Gecko) Chrome/140.0.0.0 Safari/537.36 Edg/140.0.0.0";
#endif
    wxString dark_light = Slic3r::GUI::wxGetApp().dark_mode() ? "dark" : "light";
    return wxString::Format(SLIC3R_APP_NAME "/V" SLIC3R_VERSION " (%s) %s",  dark_light, webUserAgent);
}
wxWebView* WebView::CreateWebView(wxWindow * parent, wxString const & url,wxWebViewConfiguration*conf,const std::function<void(wxWebView*)>& visitor)
{
#ifdef __WXMSW__
    wxWebViewConfiguration confg = wxWebView::NewConfiguration(wxWebViewBackendDefault);
    if (conf==nullptr) {
        conf = &confg;
    }
    // WebView2 Runtime Cache Dir
    auto cacheDir = wxFileName::DirName(wxStandardPaths::Get().GetTempDir());
    cacheDir.AppendDir(SLIC3R_APP_KEY);
    cacheDir.AppendDir(SoftFever_VERSION);
    if(!cacheDir.DirExists()){
        cacheDir.Mkdir();
    }
    conf->SetDataPath(cacheDir.GetFullPath()); 
#endif //__WXMSW__

    wxWebView* webView = conf==nullptr?wxWebView::New():wxWebView::New(*conf);
    if (webView) {
        if(visitor){
            visitor(webView);
        }
        webView->SetBackgroundColour(StateColor::darkModeColorFor(*wxWHITE));
        webView->Create(parent, wxID_ANY, url, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
        webView->SetUserAgent(CustomUserAgent());
#ifdef __WXMAC__
        WKWebView * wkWebView = (WKWebView *) webView->GetNativeBackend();
        Slic3r::GUI::WKWebView_setTransparentBackground(wkWebView);
#endif
        auto addScriptMessageHandler = [] (wxWebView *webView) {
            BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << ": begin to add script message handler for wx.";
            Slic3r::GUI::wxGetApp().set_adding_script_handler(true);
            if (!webView->AddScriptMessageHandler("wx"))
                wxLogError("Could not add script message handler");
            Slic3r::GUI::wxGetApp().set_adding_script_handler(false);
            BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << ": finished add script message handler for wx.";
        };
#ifndef __WXMSW__
        webView->CallAfter([webView, addScriptMessageHandler] {
#endif
            if (Slic3r::GUI::wxGetApp().is_adding_script_handler()) {
                g_delay_webviews.push_back(webView);
            } else {
                addScriptMessageHandler(webView);
                while (!g_delay_webviews.empty()) {
                    auto views = std::move(g_delay_webviews);
                    for (auto wv : views)
                        addScriptMessageHandler(wv);
                }
            }
#ifndef __WXMSW__
        });
#endif

    } 
    webView->SetRefData(new WebViewRef(webView));
    g_webviews.push_back(webView);
    return webView;
}
#if USE_WEBVIEW_EDGE
bool WebView::CheckWebViewRuntime()
{
    wxWebViewFactoryEdge factory;
    auto wxVersion = factory.GetVersionInfo();
    return wxVersion.GetMajor() != 0;
}

bool WebView::DownloadAndInstallWebViewRuntime()
{
    return DownloadAndInstallWV2RT() == 0;
}
#endif
void WebView::LoadUrl(wxWebView * webView, wxString const &url)
{
    auto url2  = url;
#ifdef __WIN32__
    url2.Replace("\\", "/");
#endif
    if (!url2.empty()) { url2 = wxURI(url2).BuildURI(); }
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << url2.ToUTF8();
    webView->LoadURL(url2);
}

bool WebView::RunScript(wxWebView *webView, wxString const &javascript)
{
    if (Slic3r::GUI::wxGetApp().app_config->get("internal_developer_mode") == "true"
            && javascript.find("studio_userlogin") == wxString::npos)
        wxLogMessage("Running JavaScript:\n%s\n", javascript);
    try {
        webView->RunScriptAsync(javascript);
        return true;
    } catch (std::exception &) {
        return false;
    }
}

void WebView::RecreateAll()
{
    auto dark = Slic3r::GUI::wxGetApp().dark_mode();
    for (auto webView : g_webviews) {
        webView->SetUserAgent(CustomUserAgent());
        webView->Reload();
    }
}
