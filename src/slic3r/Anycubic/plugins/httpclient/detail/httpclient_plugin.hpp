#pragma once

#include "plugins_manager/plugins_manager.hxx"

#include <boost/preprocessor/cat.hpp>

#include "plugins_base/plugins.hxx"
#include "plugins_base/plugins_base.hxx"

#define PLGUINS_NAME httpclient
#define PLUGIN_NAME_STR BOOST_PP_STRINGIZE(PLGUINS_NAME)

class HttpClientPlugin : public Anycubic::Plugins::Plugin {
public:
  HttpClientPlugin(Anycubic::Plugins::PluginHost *host);
  virtual ~HttpClientPlugin();

private:
  /**
   * @brief 下载文件
   *
   * @param url 文件URL
   * @param path 本地路径--包含文件名
   * @return true 下载成功
   * @return false 下载失败
   */
  bool Download(const wxString &url, const wxString &path);

  /**
   * @brief 获取文件内容
   *
   * @param url 文件URL
   * @param content 文件内容
   * @return true 获取成功
   * @return false 获取失败
   */
  bool Get(const wxString &url, wxString *content);
  /**
   * @brief 发送POST请求
   *
   * @param url 请求URL
   * @return wxString 响应内容
   */
  bool Post(const wxString &url, wxString *content);

private:
  // Anycubic::Plugins::Plugin interface
  const char *Name(void) override;
  bool Start(void) override;
  bool AttachEvt(wxEvtHandler *evt) override { return false; }
  bool DetachEvt(wxEvtHandler *evt) override { return false; }
  bool BindEvt(class wxPanel* panel, wxWindow* parent = nullptr,
                   wxString *bmp = nullptr) override {
    return false;
  }
  bool CreateWebview(wxWebView *view, wxWindow *parent = nullptr,
                     wxString *bmp = nullptr) override {
    return false;
  }
  void Destroy(void) override;

private:
};
