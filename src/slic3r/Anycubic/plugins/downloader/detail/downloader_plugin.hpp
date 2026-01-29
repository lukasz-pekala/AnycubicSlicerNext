#pragma once
#include "plugins_manager/plugins_manager.hxx"

#include "protocol/protocol.hpp"

#include <boost/preprocessor/cat.hpp>

#include "plugins_base/plugins.hxx"
#include "plugins_base/plugins_base.hxx"

#include <slic3r/GUI/Downloader.hpp>

#include <atomic>

#define PLGUINS_NAME downloader
#define PLUGIN_NAME_STR BOOST_PP_STRINGIZE(PLGUINS_NAME)

class DownloaderPlugin : public Anycubic::Plugins::Plugin, public wxEvtHandler {
public:
  DownloaderPlugin(Anycubic::Plugins::PluginHost *host);
  virtual ~DownloaderPlugin();

public:
  bool is_test_env(void) const;
  bool is_china_env(void) const;

private:
  /**
   * @brief 开始下载
   *
   * @param url 下载url
   * @param callback 下载回调函数
   * @param ctx 回调上下文
   * @return size_t 下载id
   */
  size_t start_download(const wxString &url,
                        download_callback callback = nullptr,
                        void *ctx = nullptr);

public:
  bool start_download_impl(size_t id, Slic3r::GUI::Download *download);

  size_t get_next_id() { return ++m_next_id; }

private:
  // Anycubic::Plugins::Plugin
  const char *Name(void) override { return PLUGIN_NAME_STR; };
  bool Start(void) override { return true; };
  void Stop(void) override;
  bool AttachEvt(class wxEvtHandler *) override { return false; };
  bool DetachEvt(class wxEvtHandler *) override { return false; };
  bool BindEvt(class wxPanel *panel, class wxWindow *parent = nullptr,
               class wxString *bmp = nullptr) override;
  bool CreateWebview(class wxWebView *view, class wxWindow *parent = nullptr,
                     class wxString *bmp = nullptr) override {
    return false;
  }
  void Destroy(void) override { delete this; };

private:
  // download event handlers
  void on_progress(wxCommandEvent &event);
  void on_error(wxCommandEvent &event);
  void on_complete(wxCommandEvent &event);
  void on_name_change(wxCommandEvent &event);
  void on_paused(wxCommandEvent &event);
  void on_canceled(wxCommandEvent &event);
  // user action event handlers
  void on_cancel(wxCommandEvent &event);

private:
  Anycubic::Plugins::PluginHost *host_;
  std::vector<std::unique_ptr<Protocol>> protocols_;
  struct Downloader {
    size_t id{0};                                    ///< 下载id
    std::unique_ptr<Slic3r::GUI::Download> download; ///< 下载器对象
    wxFileName filename;                             ///< 下载文件名
    std::function<void(int32_t status,
                       wxString &filename)>
        callback; ///< 下载回调函数
  };
  std::unique_ptr<Downloader> download_;
  size_t m_next_id{0};
  bool has_error_{false};
  wxDialog *parent_{nullptr};
  wxStaticText *label_{nullptr};
  wxGauge *progress_{nullptr};
  wxButton *cancel_{nullptr};
};