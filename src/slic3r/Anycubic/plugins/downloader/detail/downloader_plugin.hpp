#pragma once
#include "plugins_manager/plugins_manager.hxx"

#include "protocol/protocol.hpp"

#include <boost/preprocessor/cat.hpp>

#include "plugins_base/plugins.hxx"
#include "plugins_base/plugins_base.hxx"

#include <slic3r/GUI/Downloader.hpp>

#define PLGUINS_NAME downloader
#define PLUGIN_NAME_STR BOOST_PP_STRINGIZE(PLGUINS_NAME)

namespace Slic3r::GUI {
class NotificationManager;
} // namespace Slic3r::GUI
using Slic3r::GUI::NotificationManager;
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

  /**
   * @brief 停止下载
   *
   * @param download_id 下载id
   * @return true 成功
   * @return false 失败
   */
  bool stop_download(int32_t download_id);

  /**
   * @brief 暂停下载
   *
   * @param download_id 下载id
   * @return true 成功
   * @return false 失败
   */
  bool pause_download(int32_t download_id);

  /**
   * @brief 恢复下载
   *
   * @param download_id 下载id
   * @return true 成功
   * @return false 失败
   */
  bool resume_download(int32_t download_id);

public:
  bool start_download_impl(size_t id, Slic3r::GUI::Download *download);

  size_t get_next_id() { return ++m_next_id; }

private:
  // Anycubic::Plugins::Plugin
  const char *Name(void) override { return PLUGIN_NAME_STR; };
  bool Start(void) override { return true; };
  bool AttachEvt(class wxEvtHandler *) override { return false; };
  bool DetachEvt(class wxEvtHandler *) override { return false; };
  bool BindEvt(class wxPanel* panel, class wxWindow* parent = nullptr,
                   class wxString *bmp = nullptr) override {
    return false;
  }
  bool CreateWebview(class wxWebView *view, class wxWindow *parent = nullptr,
                     class wxString *bmp = nullptr) override {
    return false;
  }
  void Destroy(void) override { delete this; };

private:
  // cancel = false -> just pause
  bool user_action_callback(Slic3r::GUI::DownloaderUserAction action, int id);

  // download event handlers
  void on_progress(wxCommandEvent &event);
  void on_error(wxCommandEvent &event);
  void on_complete(wxCommandEvent &event);
  void on_name_change(wxCommandEvent &event);
  void on_paused(wxCommandEvent &event);
  void on_canceled(wxCommandEvent &event);
  bool set_download_state(int id, Slic3r::GUI::DownloadState state);

private:
  Anycubic::Plugins::PluginHost *host_;
  NotificationManager *ntf_mngr_;
  std::vector<std::unique_ptr<Protocol>> protocols_;
  struct Downloader {
    size_t id{0};                                    ///< 下载id
    std::unique_ptr<Slic3r::GUI::Download> download; ///< 下载器对象
    std::function<void(size_t download_id, int32_t status,
                       const wxString &filename)>
        callback; ///< 下载回调函数
  };
  std::vector<std::unique_ptr<Downloader>> m_downloads;
  size_t m_next_id{0};
};