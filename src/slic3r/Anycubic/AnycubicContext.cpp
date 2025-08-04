
#include "AnycubicContext.hpp"
#include "detail/anonymous.hpp"

#include <plugins_manager/plugins_manager.hxx>

#include <common/utils/aes.hpp>

#include <wx/base64.h>

#include <libslic3r/AppConfig.hpp>
#include <libslic3r/Utils.hpp>
namespace Slic3r {

namespace GUI {
class AnycubicContextPrivate : public PMConfig {
public:
  AnycubicContextPrivate(AppConfig *app_config) : app_config_(app_config) {
    assert(app_config_ != nullptr);
  }
  virtual ~AnycubicContextPrivate() {
    if (pm_ != nullptr) {
      ShutdownPM(pm_);
      pm_ = nullptr;
    }
  }
  void SetPM(PluginsManager *pm) {
    assert(pm != nullptr && pm_ == nullptr);
    pm_ = pm;
    pm_->SetConfig(this);
  }

public:
  static bool append_env(const std::string &env_name, const std::string &path) {
    char *env = getenv(env_name.c_str());
    if (env == nullptr) {
      setenv(env_name.c_str(), path.c_str(), 1);
      return true;
    }
    std::string env_path = env;
    if (env_path.find(path) != std::string::npos) {
      return false;
    }
    env_path += ":" + path;
    setenv(env_name.c_str(), env_path.c_str(), 1);
    return true;
  }

  PluginsManager *operator->() {
    assert(pm_ != nullptr);
    return pm_;
  }

private:
  bool GetValue(const class wxString &key, class wxString &value) override {
    if (auto itr = config_.find(key); itr != config_.end()) {
      value = itr->second;
      return true;
    }

    return false;
  }
  bool SetValue(const class wxString &key, const class wxString &value,
                bool persistent = true) override {
    if (persistent) {

    } else {
      config_[key] = value;
    }
    return false;
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
    wxMemoryBuffer decoded_data = wxBase64Decode(value);
    auto ret = aesDecrypt(
        GetPCID().utf8_string(),
        std::string((char *)decoded_data.GetData(), decoded_data.GetDataLen()));
    return wxString::FromUTF8(ret);
  }

  wxString Encrypt(const wxString &value) {
    auto ret = aesEncrypt(GetPCID().utf8_string(), value.utf8_string());
    return wxBase64Encode(static_cast<const void *>(ret.data()), ret.size());
  }

private:
  std::map<wxString, wxString> config_;
  AppConfig *app_config_{nullptr};
  PluginsManager *pm_{nullptr};
};

///////////////////////////////////////////////////////////////////////////////
//////////             AnycubicContext  implement     /////////////////////////
///////////////////////////////////////////////////////////////////////////////

AnycubicContext::AnycubicContext(AppConfig *app_config)
    : impl_(new AnycubicContextPrivate(app_config)) {}
AnycubicContext::~AnycubicContext() { delete impl_; }

bool AnycubicContext::AddWindow(const wxString &position, wxWindow *window) {
  assert(window != nullptr);
  return (*impl_)->AddWidget(position, window);
}

void AnycubicContext::OnInitByApp() {
  assert(impl_ != nullptr);

  std::string current_dir = Slic3r::data_dir();
  current_dir += "/plugins";

#ifdef __APPLE__
  AnycubicContextPrivate::append_env("DYLD_LIBRARY_PATH", current_dir);
  LOG_INFO("DYLD_LIBRARY_PATH after append: {}", getenv("DYLD_LIBRARY_PATH"));
#elif defined(__linux__)
  AnycubicContextPrivate::append_env("LD_LIBRARY_PATH", current_dir);
  LOG_INFO("LD_LIBRARY_PATH after append: {}", getenv("LD_LIBRARY_PATH"));
#endif
  auto package = Slic3r::data_dir() + "/cache/plugins.zip";
  PluginsPackageInfo info = {0};
  if (!GetPluginsPackageInfo(package.c_str(), &info)) {
    LOG_ERROR("GetPluginsPackageInfo failed");
  }

  auto pm = SetupPM(package.c_str(), current_dir.c_str());
  if (pm != nullptr) {
    impl_->SetPM(pm);
  } else {
    LOG_ERROR("SetupPM failed");
  }
  std::vector<CreatePlugin> plugins{};
  (*impl_)->AppendPlugins(plugins.data(), plugins.size());
  // 这行只能最后一行
  (*impl_)->EmitEvent(EventType::kEventInitByApp);
}
void AnycubicContext::OnInitByGui() {
  (*impl_)->EmitEvent(EventType::kEventInitByGUI);
}
void AnycubicContext::OnFinishedByGui() {
  (*impl_)->EmitEvent(EventType::kEventFinishedByGUI);
}
void AnycubicContext::OnExitByGui() {
  (*impl_)->EmitEvent(EventType::kEventExitByGUI);
}
void AnycubicContext::OnExitByApp() {
  (*impl_)->EmitEvent(EventType::kEventExitByApp);
}
} // namespace GUI
} // namespace Slic3r