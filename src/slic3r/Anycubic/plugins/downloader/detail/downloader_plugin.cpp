#include "downloader_plugin.hpp"
#include "impl/impl.hpp"
#include "protocol/acnext.hpp"
#include "protocol/plain.hpp"

#include <plugins_base/funcation.hxx>
#include <plugins_sdk/constant/config.hxx>
#include <plugins_sdk/event/detail/plugin_custom_event.hxx>

#include <utility/utils/filesystem.hxx>

#include <slic3r/GUI/GUI_App.hpp>

namespace Slic3r::GUI {
void open_folder(const std::string &path);
}

template <typename T, typename Container, typename owner_type>
static inline int make_unique(Container &protocols, owner_type *owner) {
  protocols.emplace_back(new T(owner));
  return 0;
}
template <typename... Args, typename Container, typename owner_type>
static inline void create_protocol(Container &protocols, owner_type *owner) {
  (make_unique<Args>(protocols, owner), ...);
}

DownloaderPlugin::DownloaderPlugin(Anycubic::Plugins::PluginHost *host)
    : host_(host) {
  create_protocol<ACNextProtocol, PlainProtocol>(protocols_, this);
  auto router = host_->Router();
  assert(router != nullptr);
  router->REGISTER_FUNCATION(DownloaderPlugin, start_download);

  Bind(Slic3r::GUI::EVT_DWNLDR_FILE_COMPLETE, &DownloaderPlugin::on_complete,
       this);
  Bind(Slic3r::GUI::EVT_DWNLDR_FILE_PROGRESS, &DownloaderPlugin::on_progress,
       this);
  Bind(Slic3r::GUI::EVT_DWNLDR_FILE_ERROR, &DownloaderPlugin::on_error, this);
  Bind(Slic3r::GUI::EVT_DWNLDR_FILE_NAME_CHANGE,
       &DownloaderPlugin::on_name_change, this);
  Bind(Slic3r::GUI::EVT_DWNLDR_FILE_PAUSED, &DownloaderPlugin::on_paused, this);
  Bind(Slic3r::GUI::EVT_DWNLDR_FILE_CANCELED, &DownloaderPlugin::on_canceled,
       this);
}

DownloaderPlugin::~DownloaderPlugin() { Stop(); }

bool DownloaderPlugin::is_test_env(void) const { return ::is_test_env(host_); }

bool DownloaderPlugin::is_china_env(void) const {
  return ::is_china_env(host_);
}

size_t DownloaderPlugin::start_download(const wxString &url,
                                        download_callback callback, void *ctx) {
  LOG_INFO("start_download: {}", url.utf8_string());
  // 做个原子锁
  has_error_ = false;
  for (auto &p : protocols_) {
    if (!p->parse_url(url)) {
      continue;
    }
    if (!p->can_handle()) {
      continue;
    }
    // NOTE: 获取下载保存路径
    auto id = get_next_id();

    wxString download_path;
    host_->GetValue(CONFIG_DOWNLOAD_PATH, download_path);

    wxFileName full_path = wxFileName::DirName(download_path);
    full_path.SetName(p->get_filename());
    wxString filename = full_path.GetFullPath();
    if (callback != nullptr) {
      callback(ctx, id, DownloadStatus_Suggested, filename);
    }

    if (p->start(id, filename)) {
      assert(id == download_->id);
      download_->callback = std::bind(callback, ctx, id, std::placeholders::_1,
                                      std::placeholders::_2);
      download_->filename = full_path;
      // 先显示界面
      auto result =
          ::show_progress_dialog(host_, PLUGIN_NAME_STR, _L("download"));
      if (!result) {
        break;
      }
      download_->download->start();
      return id;
    }
  }
  wxString tmp;
  callback(ctx, 0, DownloadStatus_NotSupported, tmp);
  return 0;
}

bool DownloaderPlugin::start_download_impl(size_t id,
                                           Slic3r::GUI::Download *download) {
  download_ = std::make_unique<Downloader>();
  download_->id = id;
  download_->download = std::unique_ptr<Slic3r::GUI::Download>(download);
  return true;
}

void DownloaderPlugin::Stop(void) {
  protocols_.clear();
  download_.reset();
}

bool DownloaderPlugin::BindEvt(wxPanel *panel, wxWindow *parent,
                               wxString *bmp) {
  parent_ = wxStaticCast(parent, wxDialog);
  // label
  label_ = XRCCTRL(*panel, "label", wxStaticText);
  // progress
  progress_ = XRCCTRL(*panel, "progress", wxGauge);
  // cancel
  cancel_ = XRCCTRL(*panel, "cancel", wxButton);
  assert(label_ && progress_ && cancel_);

  cancel_->Bind(wxEVT_BUTTON, &DownloaderPlugin::on_cancel, this);
  parent_->Bind(EVT_DIALOG_CLOSE_EVENT, [this](wxCommandEvent &evt) {
    if (has_error_) {
      wxString tmp;
      download_->callback(DownloadStatus_Error, tmp);
    } else if (evt.GetInt() == wxID_CANCEL) {
      on_cancel(evt);
    }
    parent_ = nullptr; // 先处理界面指针，关闭后续逻辑
    label_ = nullptr;
    progress_ = nullptr;
    cancel_ = nullptr;
  });

  return true;
}

void DownloaderPlugin::on_progress(wxCommandEvent &event) {
  if (progress_) {
    long percent = 0;
    event.GetString().ToLong(&percent);
    progress_->SetValue(static_cast<int>(percent));
  }
}
void DownloaderPlugin::on_error(wxCommandEvent &event) {
  if (label_ && progress_) {
    auto msg = event.GetString();
    msg.Replace("\n", " ");
    label_->SetLabel(msg);
    progress_->Hide();
    cancel_->SetLabel(_L("Close"));
    parent_->SetTitle(_L("Download Failed"));
    has_error_ = true;
  }
}
void DownloaderPlugin::on_complete(wxCommandEvent &event) {
  assert(download_ != nullptr);
  wxString tmp = download_->filename.GetFullPath();
  parent_->EndModal(wxID_OK);
  download_->callback(DownloadStatus_Complete, tmp);
}

void DownloaderPlugin::on_name_change(wxCommandEvent &event) {
  // empty
}

void DownloaderPlugin::on_paused(wxCommandEvent &event) {
  // empty
}

void DownloaderPlugin::on_canceled(wxCommandEvent &event) {
  assert(download_ != nullptr);
  wxString tmp;
  download_->callback(DownloadStatus_Canceled, tmp);
}

void DownloaderPlugin::on_cancel(wxCommandEvent &event) {
  assert(download_ != nullptr);
  download_->download->cancel();
  if (parent_ != nullptr) {
    parent_->EndModal(wxID_NO);
  }
}