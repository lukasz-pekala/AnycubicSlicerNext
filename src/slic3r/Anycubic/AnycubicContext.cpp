
#include "AnycubicContext.hpp"
#include "detail/anonymous.hpp"
#include "plugins/plugins_list.hpp"
#include "protocol/check_protocol.hpp"

#include <utility/codec/base64.hxx>
#include <utility/encrypt/aes.hxx>
#include <utility/utils/filesystem.hxx>
#include <utility/utils/range.hxx>

#include <plugins_base/funcation.hxx>
#include <plugins_sdk/constant/config.hxx>

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
static const char *CONFIG_IGNORE_VERSION = "app/skip_version";
extern const char *CONFIG_PLUGIN_NAME_LIST;
namespace Slic3r {
namespace GUI {

#if 0
// 定义白名单
const std::set<wxString> SECTION_NAME_WHITELIST = {
    "anycubic_remote_printing", "anycubic_presets", "anycubic_cloud", "app", "user", "web"
    // 可以根据需要添加更多允许的section_name
};

// 检查section_name是否在白名单中
bool is_section_name_allowed(const wxString &section_name) {
  return SECTION_NAME_WHITELIST.find(section_name) !=
         SECTION_NAME_WHITELIST.end();
}
#else
// 好像存在的意义很小，先关闭以后再说
bool is_section_name_allowed(const wxString &section_name) { return true; }
#endif

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
  void SetPackageVisitor(PluginsVisitor_t visitor,
                         void *ctx = nullptr) override {}
};

class AnycubicContextPrivate : public PMConfig {
  friend class AnycubicContext;
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
  bool Visitor(const wxString &package) {
    // 如果有名单，就要在名单内才允许加载
    static wxString name_list;
    static bool init = false;
    if (init || !GetEncryptValue(CONFIG_PLUGIN_NAME_LIST, name_list) ||
        name_list.IsEmpty()) {
      return true;
    }
    init = true;

    wxString filename;
    wxFileName::SplitPath(package, nullptr, &filename, nullptr);
    if (filename == name_list) {
      return true;
    }
    wxStringTokenizer tokenizer(name_list, ";");
    while (tokenizer.HasMoreTokens()) {
      auto token = tokenizer.GetNextToken();
      if (token == filename) {
        return true;
      }
    }
    return false;
  }
  void SetPM(PluginsManager *pm) {
    assert(pm != nullptr && pm_ == nullptr);
    pm_ = pm;
    pm_->SetConfig(this);
  }
  PluginsManager *GetPM() { return pm_; }
  bool PluginsIsLoaded() const { return pm_ != nullptr; }
  inline void EmitEvent(EventType event) {
    if (pm_ != nullptr) {
      pm_->EmitEvent(event);
    }
  }
  static void downloader_callback(void *ctx, int32_t download_id,
                                  int32_t status, wxString &filename) {
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
  bool
  StartDownload(const wxString &url, void *ctx = nullptr,
                void (*callback)(void *ctx, int32_t download_id, int32_t status,
                                 wxString &filename) = downloader_callback) {
    if (ctx == nullptr) {
      ctx = this;
    }
    return Anycubic::Plugins::dispatch_call<size_t>(
        pm_, "downloader", "start_download", url, callback, ctx);
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
      return value.IsEmpty() == false;
    }

    wxStringTokenizer tokenizer(key, ",./ ");
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
      if (val.empty()) {
        return false;
      }
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

    return value.IsEmpty() == false;
  }
  bool SetValue(const class wxString &key, const class wxString &value,
                bool persistent = true) override {
    if (persistent) {
      wxStringTokenizer tokenizer(key, ",./ ");
      switch (tokenizer.CountTokens()) {
      case 2: {
        auto section_name = tokenizer.GetNextToken();
        // 检查section_name是否在白名单中
        if (!is_section_name_allowed(section_name)) {
          return false; // 不在白名单中，丢弃处理
        }
        auto config_name = tokenizer.GetNextToken();
        if (value.IsEmpty()) {
          app_config_->erase(section_name.utf8_string(),
                             config_name.utf8_string());
        } else {
          app_config_->set_str(section_name.utf8_string(),
                               config_name.utf8_string(), value.utf8_string());
        }
        break;
      }
      case 1: {
        if (value.IsEmpty()) {
          app_config_->erase("app", key.utf8_string());
        } else {
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
    if (value.IsEmpty()) {
      return SetValue(key, value, persistent);
    } else {
      auto tmp = Encrypt(value);
      return SetValue(key, tmp, persistent);
    }
  }
  wxString Decrypt(const wxString &value) {
    auto decoded_data = ::base64Decode(value.utf8_string());
    auto ret = ::aesDecrypt(std::string_view((char *)decoded_data.data(),
                                             decoded_data.size()),GetPCID(app_config_).utf8_string());
    return wxString::FromUTF8(ret);
  }
  wxString Encrypt(const wxString &value) {
    auto ret =
        ::aesEncrypt( value.utf8_string(),GetPCID(app_config_).utf8_string());
    return wxString::FromUTF8(::base64Encode(ret));
  }

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

bool AnycubicContext::HasPlugin() const {
  // TODO: 检查是否存在插件
  return false;
}

bool AnycubicContext::StartDownloadPlugins(bool is_auto_update /*= false*/) {
  // TODO: 下载插件
  return false;
}

void AnycubicContext::UpdatePluginConfig(void) {
  // 更新插件列表。在必要时和用于关闭问题插件
  UpdateChecker checker(impl_);
  std::vector<std::string> name_list;
  checker.update_plugin_name_list(name_list);
}

void AnycubicContext::UpdatePreset(void) {
  // TODO: 更新打印配置
}
int version_comparison(const char *verison,
                       const char *verison2 = SoftFever_VERSION) {
  assert(verison != nullptr);
  Semver v2 = *(Semver::parse(verison2));
  Semver v1 = *(Semver::parse(verison));
  assert(v1.valid() && v2.valid());
  if (v1 > v2) {
    return 1;
  } else if (v1 < v2) {
    return -1;
  } else {
    return 0;
  }
}
void AnycubicContext::UpdateApp(bool is_auto_update /*= false*/) {
  // 更新主程序
  UpdateChecker checker(impl_);
  UpdateAppResponse response;
  if (!checker.check_app_update_available(response)) {
    // 更新检查失败
    if (!is_auto_update) {
      ShowUpdateVersionDialog(_L("Check update failed"), _L(""), false);
    }
    return;
  }
  if (response.version_code == 0) {
    if (!is_auto_update) {
      // 没有更新弹框
      ShowUpdateVersionDialog(_L("It's already the latest"), _L(""), false);
    }
    return;
  }
  if (is_auto_update) {
    wxString skip_version;
    auto result = impl_->GetEncryptValue(CONFIG_IGNORE_VERSION, skip_version);
    if (result && version_comparison(skip_version.utf8_string().c_str(),
                                     response.version_name.c_str()) >= 0) {
      // 自动更新，跳过版本
      return;
    }
  }

  // 弹框提示更新

  wxString language;
  impl_->GetValue("language", language);

  auto buttonID = ShowUpdateVersionDialog(
      wxString::FromUTF8(language != "zh-CN" ? response.update_content_us
                                             : response.update_content_cn),
      response.version_name);
  if (buttonID != wxID_YES) {
    if (buttonID == wxID_IGNORE) {
      // 忽略版本更新
      impl_->SetEncryptValue(CONFIG_IGNORE_VERSION,
                             wxString::FromUTF8(response.version_name));
    } else {
      impl_->SetEncryptValue(CONFIG_IGNORE_VERSION, wxEmptyString);
    }
    return;
  }
  impl_->SetEncryptValue(CONFIG_IGNORE_VERSION, wxEmptyString);

  // : 下载更新包
  impl_->StartDownload(
      response.download_url, this,
      [](void *ctx, int32_t download_id, int32_t status, wxString &filename) {
        assert(ctx != nullptr);
        AnycubicContext *context = static_cast<AnycubicContext *>(ctx);
        if (status == 0) {
// 下载更新包完成
// 1. 运行更新程序
// 2. 退出程序
#ifdef _WIN32
          HINSTANCE hInstance = ShellExecuteW(NULL, L"open", filename.wc_str(),
                                              NULL, NULL, SW_SHOWNORMAL);
          if (reinterpret_cast<intptr_t>(hInstance) <= 32) {
            BOOST_LOG_TRIVIAL(error) << "create process failed! Error code: "
                                     << reinterpret_cast<intptr_t>(hInstance)
                                     << ":" << GetLastError();
          }
#elif defined(__APPLE__)
          system(wxString::Format("hdiutil attach \"%s\" -noverify -autoopen",
                                  filename)
                     .utf8_string()
                     .c_str());
#else
          // 构造命令，在用户按回车键后重启程序
          auto command = wxString::Format(
              "gnome-terminal -- bash -c \"pkexec sh -c 'apt update && apt "
              "install --only-upgrade anycubicslicernext'; read -p '%s';\"",
              _L("Update complete. Please restart to apply changes.Press "
                 "[Enter] to exit."));
          int result = wxExecute(command, wxEXEC_ASYNC);
          if (result == -1) {
            BOOST_LOG_TRIVIAL(error) << "Failed to execute system command";
          }
#endif

          Slic3r::GUI::wxGetApp().CallAfter(
              []() { Slic3r::GUI::wxGetApp().GetTopWindow()->Close(); });
        } else if (status == 1) {
          // 用户取消下载
        } else if (status == 3) {
          // 更新文件名
        }
      });
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
    pm->SetPackageVisitor(
        [](void *ctx, const wxString &package) {
          auto pthis = static_cast<AnycubicContextPrivate *>(ctx);
          assert(pthis != nullptr);
          return pthis->Visitor(package);
        },
        impl_);
    impl_->SetPM(pm);
  } else {
    LOG_ERROR("SetupPM failed");
    return;
  }
  // 保证所有地方读取都是一致的
  impl_->SetValue(CONFIG_PACKAGE_PATH, package, false);
  impl_->SetValue(CONFIG_PLUGINS_PATH, current_dir, false);

  std::vector<create_library_t> &plugins = Anycubic::Plugins::GetPluginsList();
  (*impl_)->AddStaticPlugins(plugins.data(), plugins.size());
  // 这行只能最后一行
  impl_->EmitEvent(EventType::kEventInitByApp);
}
void AnycubicContext::OnInitByGui() {
  if (PluginsIsLoaded()) {
    impl_->EmitEvent(EventType::kEventInitByGUI);
  }
}
void AnycubicContext::OnFinishedByGui() {
  if (PluginsIsLoaded()) {
    impl_->EmitEvent(EventType::kEventFinishedByGUI);
  }
  // 检查是否有新的版本
  UpdateApp(true);
  UpdatePreset();
}
void AnycubicContext::OnExitByGui() {
  if (PluginsIsLoaded()) {
    impl_->EmitEvent(EventType::kEventExitByGUI);
  }
}
void AnycubicContext::OnExitByApp() {
  if (PluginsIsLoaded()) {
    impl_->EmitEvent(EventType::kEventExitByApp);
  }
}
int AnycubicContext::ShowUpdateVersionDialog(const wxString &extmsg,
                                             const wxString &version_str,
                                             bool is_skip_version) {

  UpdateVersionDialog dialog(wxGetApp().GetTopWindow());
  dialog.update_version_info(extmsg, version_str);
  // dialog.update_version_info(version_info.description);
  if (is_skip_version) {
    dialog.m_button_skip_version->Hide();
  }
  return dialog.ShowModal();
}
} // namespace GUI
} // namespace Slic3r