#include "downloader_plugin.hpp"
#include "impl/impl.hpp"
#include "protocol/acnext.hpp"

#include <plugins_base/funcation.hxx>
#include <plugins_sdk/constant/config.hxx>
#include <utility/utils/filesystem.hxx>

#include <slic3r/GUI/GUI_App.hpp>
#include <slic3r/GUI/NotificationManager.hpp>

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
  create_protocol<ACNextProtocol>(protocols_, this);
  auto router = host_->Router();
  assert(router != nullptr);
  router->REGISTER_FUNCATION(DownloaderPlugin, start_download);
  router->REGISTER_FUNCATION(DownloaderPlugin, stop_download);
  router->REGISTER_FUNCATION(DownloaderPlugin, pause_download);
  router->REGISTER_FUNCATION(DownloaderPlugin, resume_download);

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

  ntf_mngr_ = Slic3r::GUI::wxGetApp().notification_manager();
  assert(ntf_mngr_ != nullptr);
}

DownloaderPlugin::~DownloaderPlugin() { protocols_.clear(); }

bool DownloaderPlugin::is_test_env(void) const { return ::is_test_env(host_); }

bool DownloaderPlugin::is_china_env(void) const {
  return ::is_china_env(host_);
}

size_t DownloaderPlugin::start_download(const wxString &url,
                                        download_callback callback, void *ctx) {
  LOG_INFO("start_download: {}", url.utf8_string());
  for (auto &p : protocols_) {
    if (!p->parse_url(url)) {
      continue;
    }
    if (!p->can_handle()) {
      continue;
    }
    // NOTE: 获取下载保存路径
    wxString download_path;
    host_->GetValue(CONFIG_DOWNLOAD_PATH, download_path);
    if (download_path.IsEmpty()) {
      continue;
    }
    auto id = get_next_id();
    if (p->start(id, download_path)) {
      auto &d = m_downloads.back();
      assert(id == d->id);
      d->callback = [ctx, callback](size_t download_id, int32_t status,
                                    const wxString &filename) {
        callback(ctx, download_id, status, filename);
      };
      d->download->start();
      return id;
    }
  }
  return 0;
}

bool DownloaderPlugin::stop_download(int32_t download_id) {
  return user_action_callback(Slic3r::GUI::DownloadUserCanceled, download_id);
}
bool DownloaderPlugin::pause_download(int32_t download_id) {
  return user_action_callback(Slic3r::GUI::DownloadUserPaused, download_id);
}

bool DownloaderPlugin::resume_download(int32_t download_id) {
  return user_action_callback(Slic3r::GUI::DownloadUserContinued, download_id);
}

bool DownloaderPlugin::start_download_impl(size_t id,
                                           Slic3r::GUI::Download *download) {
  auto downloader = std::make_unique<Downloader>();
  downloader->id = id;
  downloader->download = std::unique_ptr<Slic3r::GUI::Download>(download);
  m_downloads.emplace_back(std::move(downloader));
  ntf_mngr_->push_download_URL_progress_notification(
      id, download->get_filename(),
      std::bind(&DownloaderPlugin::user_action_callback, this,
                std::placeholders::_1, std::placeholders::_2));
  return true;
}

void DownloaderPlugin::on_progress(wxCommandEvent &event) {
  size_t id = event.GetInt();
  double percent = .0;
  event.GetString().ToDouble(&percent);
  ntf_mngr_->set_download_URL_progress(id, percent / 100.0);
}
void DownloaderPlugin::on_error(wxCommandEvent &event) {
  BOOST_LOG_TRIVIAL(error) << "Download error: " << event.GetString();

  int id = event.GetInt();
  ntf_mngr_->set_download_URL_error(id, event.GetString().utf8_string());
  set_download_state(id, Slic3r::GUI::DownloadState::DownloadError);
}
void DownloaderPlugin::on_complete(wxCommandEvent &event) {
  set_download_state(event.GetInt(), Slic3r::GUI::DownloadState::DownloadDone);
}
bool DownloaderPlugin::user_action_callback(
    Slic3r::GUI::DownloaderUserAction action, int id) {
  for (auto &d : m_downloads) {
    if (d->id == id) {
      switch (action) {
      case Slic3r::GUI::DownloadUserCanceled:
        d->download->cancel();
        return true;
      case Slic3r::GUI::DownloadUserPaused:
        d->download->pause();
        return true;
      case Slic3r::GUI::DownloadUserContinued:
        d->download->resume();
        return true;
      case Slic3r::GUI::DownloadUserOpenedFolder: {
        using namespace Slic3r::GUI;
        // NOTE: 打开下载文件夹
        wxString download_path;
        host_->GetValue(CONFIG_DOWNLOAD_PATH, download_path);
        if (!download_path.IsEmpty()) {
          open_folder(download_path.utf8_string());
        }
        return true;
      }
      default:
        return false;
      }
    }
  }
  return false;
}

void DownloaderPlugin::on_name_change(wxCommandEvent &event) {
  // empty
}

void DownloaderPlugin::on_paused(wxCommandEvent &event) {
  size_t id = event.GetInt();
  ntf_mngr_->set_download_URL_paused(id);
}

void DownloaderPlugin::on_canceled(wxCommandEvent &event) {
  int id = event.GetInt();
  ntf_mngr_->set_download_URL_canceled(id);
  set_download_state(id, Slic3r::GUI::DownloadState::DownloadStopped);
}

bool DownloaderPlugin::set_download_state(int id,
                                          Slic3r::GUI::DownloadState state) {
  auto itr = std::find_if(m_downloads.begin(), m_downloads.end(),
                          [id](const auto &d) { return d->id == id; });
  if (itr == m_downloads.end()) {
    return false;
  }
  auto &d = *itr;
  d->download->set_state(state);

  if (state == Slic3r::GUI::DownloadState::DownloadDone ||
      state == Slic3r::GUI::DownloadState::DownloadError ||
      state == Slic3r::GUI::DownloadState::DownloadStopped) {
    auto filename = d->download->get_dest_folder();
    if (filename.back() != '/') {
      filename += '/';
    }
    filename += d->download->get_filename();
    d->callback(d->id, state, filename);
    m_downloads.erase(itr);
  }
  return true;
}
