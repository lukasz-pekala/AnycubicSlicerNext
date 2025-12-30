
#include "AnycubicContext.hpp"
#include "detail/anonymous.hpp"
#include "plugins/plugins_list.hpp"

#include <utility/encrypt/aes.hxx>
#include <utility/utils/filesystem.hxx>
#include <utility/utils/range.hxx>

#include <plugins_base/funcation.hxx>

#include <slic3r/GUI/GUI_App.hpp>
#include <slic3r/GUI/MainFrame.hpp>
#include <slic3r/GUI/Plater.hpp>
#include <slic3r/GUI/Widgets/WebView.hpp>

#include <wx/base64.h>
#include <wx/filename.h>
#include <wx/tokenzr.h>

#include <libslic3r/AppConfig.hpp>
#include <libslic3r/Utils.hpp>

#include <set>

#ifdef __WXMSW__
#include <Windows.h>
static int setenv(const char *name, const char *value, int overwrite) {
  if (!overwrite) {
    DWORD size = GetEnvironmentVariableA(name, NULL, 0);
    if (size > 0) {
      return 0;
    }
  }
  return SetEnvironmentVariableA(name, value) ? 0 : -1;
}
#endif // __WXMSW__

namespace Slic3r {
namespace GUI {
// 定义白名单
const std::set<wxString> SECTION_NAME_WHITELIST = {
    "anycubic_remote_printing", "anycubic_presets", "anycubic_cloud", "app"
    // 可以根据需要添加更多允许的section_name
};

// 检查section_name是否在白名单中
bool is_section_name_allowed(const wxString &section_name) {
  return SECTION_NAME_WHITELIST.find(section_name) !=
         SECTION_NAME_WHITELIST.end();
}

struct EmptyPM : public PluginsManager {
  bool AddWidget(const wxString &position, wxWindow *widget) override {
    return false;
  }
  bool AddStaticPlugins(create_library_t *create, size_t count) override {
    return false;
  }
  bool SetConfig(class PMConfig *config) override { return false; }
  void EmitEvent(EventType event) override { return; }
  size_t Plugins(void) const override { return 0; }
  size_t Package(void) const override { return 0; };
  bool PackagePath(size_t index, wxString &path) override { return false; }
  bool ExecuteFunction(const char *plugin, const char *fname,
                       Anycubic::Plugins::IStream *data,
                       Anycubic::Plugins::OStream *result) override {
    return false;
  }
};

class AnycubicContextPrivate : public PMConfig {

#define URL_SWITCH(name, cn, cn_test, en, en_test)                             \
  static const char *name[] = {cn, cn_test, en, en_test};

public:
  explicit AnycubicContextPrivate(AppConfig *app_config)
      : app_config_(app_config) {
    assert(app_config_ != nullptr);
  }
  ~AnycubicContextPrivate() override {
    if (pm_ != nullptr) {
      ::ShutdownPM(pm_);
      pm_ = nullptr;
    }
  }
  void SetPM(PluginsManager *pm) {
    assert(pm != nullptr && pm_ == nullptr);
    pm_ = pm;
    pm_->SetConfig(this);
  }
  PluginsManager *GetPM() { return pm_; }
  bool PluginsIsLoaded() const { return pm_ != nullptr; }

  bool StartDownload(const wxString &url) {
    static auto call = [](void *ctx, int32_t download_id, int32_t status,
                          const wxString &filename) {
      if (status == 0) {
        // 下载成功
        LOG_INFO("download success: {}", filename.utf8_string());
        // NOTE: 加载模型文件
        wxArrayString filenames;
        filenames.Add(filename);
        Slic3r::GUI::wxGetApp().plater()->load_files(filenames);
        Slic3r::GUI::wxGetApp().mainframe->update_title();
      }
    };
    return Anycubic::Plugins::dispatch_call<bool>(
        pm_, "downloader", "start_download", url, &call, nullptr);
  }

public:
  static bool append_env(const std::string &env_name, const std::string &path) {
    const char *env = ::getenv(env_name.c_str());
    if (env == nullptr) {
      ::setenv(env_name.c_str(), path.c_str(), 1);
      return true;
    }
    std::string env_path = env;
    if (env_path.find(path) != std::string::npos) {
      return false;
    }
    env_path += ":" + path;
    ::setenv(env_name.c_str(), env_path.c_str(), 1);
    return true;
  }

  PluginsManager *operator->() {
    if (pm_ != nullptr)
      return pm_;
    else {
      static EmptyPM empty_pm;
      return &empty_pm;
    }
  }

private:
  bool GetValue(const class wxString &key, class wxString &value) override {
    if (auto itr = config_.find(key); itr != config_.end()) {
      value = itr->second;
      return true;
    }

    wxStringTokenizer tokenizer(key, ",. ");
    auto tokens = tokenizer.CountTokens();
    switch (tokens) {
    case 2: {
      auto section_name = tokenizer.GetNextToken();
      // 检查section_name是否在白名单中
      if (!is_section_name_allowed(section_name)) {
        return false; // 不在白名单中，丢弃处理
      }
      auto config_name = tokenizer.GetNextToken();
      auto val = app_config_->get(section_name.utf8_string(),
                                  config_name.utf8_string());
      value = wxString::FromUTF8(val);
      break;
    }
    case 1: {
      auto val = app_config_->get(key.utf8_string());
      if (val.empty()) {
        return false;
      }
      value = wxString::FromUTF8(val);
      break;
    }
    default:
      return false;
      break;
    }

    return true;
  }
  bool SetValue(const class wxString &key, const class wxString &value,
                bool persistent = true) override {
    if (persistent) {
      wxStringTokenizer tokenizer(key, ",. ");
      switch (tokenizer.CountTokens()) {
      case 2: {
        auto section_name = tokenizer.GetNextToken();
        // 检查section_name是否在白名单中
        if (!is_section_name_allowed(section_name)) {
          return false; // 不在白名单中，丢弃处理
        }
        auto config_name = tokenizer.GetNextToken();
        if(value.IsEmpty()){
          app_config_->erase(section_name.utf8_string(),
                             config_name.utf8_string());
        }else{
          app_config_->set_str(section_name.utf8_string(),
                             config_name.utf8_string(), value.utf8_string());
        }
        break;
      }
      case 1: {
        if(value.IsEmpty()){
          app_config_->erase("app", key.utf8_string());
        }else{
          app_config_->set(key.utf8_string(), value.utf8_string());
        }
        break;
      }
      default:
        return false;
        break;
      }
    } else {
      config_[key] = value;
    }
    return true;
  }
  bool GetEncryptValue(const class wxString &key,
                       class wxString &value) override {
    if (auto ret = GetValue(key, value); ret) {
      value = Decrypt(value);
      return true;
    }
    return false;
  }

  bool SetEncryptValue(const class wxString &key, const class wxString &value,
                       bool persistent = true) override {
    auto tmp = Encrypt(value);
    return SetValue(key, tmp, persistent);
  }

  wxString Decrypt(const wxString &value) {
    wxMemoryBuffer decoded_data = ::wxBase64Decode(value);
    auto ret = ::aesDecrypt(GetPCID(app_config_).utf8_string(),
                            std::string_view((char *)decoded_data.GetData(),
                                             decoded_data.GetDataLen()));
    return wxString::FromUTF8(ret);
  }

  wxString Encrypt(const wxString &value) {
    auto ret =
        ::aesEncrypt(GetPCID(app_config_).utf8_string(), value.utf8_string());
    return ::wxBase64Encode(static_cast<const void *>(ret.data()), ret.size());
  }

public:
  void CheckUpdatePlugin(int64_t version, const char *name,
                         const PluginUpdateCallback &callback) {

    // 检查插件更新
  }
  std::string GetPluginUpdateUrl(const wxString &name, int64_t version) {

    return std::string();
  }

  int get_url_index() { return 0; }

private:
  std::map<wxString, wxString> config_;
  AppConfig *app_config_{nullptr};
  PluginsManager *pm_{nullptr};
};

///////////////////////////////////////////////////////////////////////////////
//////////             AnycubicContext  implement /////////////////////////
///////////////////////////////////////////////////////////////////////////////

AnycubicContext::AnycubicContext(AppConfig *app_config)
    : impl_(new AnycubicContextPrivate(app_config)) {}
AnycubicContext::~AnycubicContext() { delete impl_; }

bool AnycubicContext::AddWindow(const wxString &position, wxWindow *window) {
  assert(window != nullptr);
  return (*impl_)->AddWidget(position, window);
}

bool AnycubicContext::PluginsIsLoaded() const {
  return impl_->PluginsIsLoaded();
}

bool AnycubicContext::CheckUpdatePlugins(const PluginUpdateCallback &callback) {
  assert(callback != nullptr);
  using Anycubic::utility::make_range;
  wxString path;
  for (auto index : make_range(static_cast<int>(impl_->GetPM()->Package()))) {
    if (impl_->GetPM()->PackagePath(index, path)) {
      PluginsPackageInfo info = {0};
      if (GetPluginsPackageInfo(path.utf8_string().c_str(), &info)) {
        impl_->CheckUpdatePlugin(info.version, info.name, callback);
        FreePluginsPackageInfo(&info);
      }
    }
  }

  return false;
}

bool AnycubicContext::HasPlugin() const {
  auto pm = impl_->GetPM();
  if (pm == nullptr) {
    return false;
  }
  return pm->Package() > 0;
}
bool AnycubicContext::StartDownload(const wxString &url) {
  return impl_->StartDownload(url);
}
void AnycubicContext::OnInitByApp() {
  assert(impl_ != nullptr);
  wxString current_dir = wxString::FromUTF8(Slic3r::data_dir());
  auto package =
      Anycubic::utility::JoinPath(current_dir, "cache"); // 插件缓存目录
  current_dir = Anycubic::utility::JoinPath(current_dir, "plugins"); // 插件目录
#ifndef NDEBUG
  if (auto dir = std::getenv("PLUGINS_DEBUG_DIR"); dir != nullptr) {
    current_dir = wxString::FromUTF8(dir);
    LOG_INFO("PLUGINS_DEBUG_DIR: {}", current_dir.utf8_string());
  }
#else
  if (!::wxFileExists(current_dir)) {
    LOG_WARN(
        "Plugins package does not exist, skipping loading plugins. Path: {}",
        package.utf8_string());
    return;
  }
#endif // NDEBUG

#ifdef __APPLE__
  AnycubicContextPrivate::append_env("DYLD_LIBRARY_PATH",
                                     current_dir.utf8_string());
  LOG_INFO("DYLD_LIBRARY_PATH after append: {}", getenv("DYLD_LIBRARY_PATH"));
#elif defined(__linux__)
  AnycubicContextPrivate::append_env("LD_LIBRARY_PATH", current_dir);
  LOG_INFO("LD_LIBRARY_PATH after append: {}", getenv("LD_LIBRARY_PATH"));
#endif
  assert(current_dir.IsEmpty() == false);
  auto pm = ::SetupPM(package.utf8_string().c_str(), WebView::CreateWebView,
                      SLIC3R_APP_KEY, current_dir.utf8_string().c_str());
  if (pm != nullptr) {
    impl_->SetPM(pm);
  } else {
    LOG_ERROR("SetupPM failed");
    return;
  }
  std::vector<create_library_t> &plugins = Anycubic::Plugins::GetPluginsList();
  (*impl_)->AddStaticPlugins(plugins.data(), plugins.size());
  // 这行只能最后一行
  (*impl_)->EmitEvent(EventType::kEventInitByApp);
}
void AnycubicContext::OnInitByGui() {
  if (PluginsIsLoaded()) {
    (*impl_)->EmitEvent(EventType::kEventInitByGUI);
  }
}
void AnycubicContext::OnFinishedByGui() {
  if (PluginsIsLoaded()) {
    (*impl_)->EmitEvent(EventType::kEventFinishedByGUI);
  }
}
void AnycubicContext::OnExitByGui() {
  if (PluginsIsLoaded()) {
    (*impl_)->EmitEvent(EventType::kEventExitByGUI);
  }
}
void AnycubicContext::OnExitByApp() {
  if (PluginsIsLoaded()) {
    (*impl_)->EmitEvent(EventType::kEventExitByApp);
  }
}
} // namespace GUI
} // namespace Slic3r