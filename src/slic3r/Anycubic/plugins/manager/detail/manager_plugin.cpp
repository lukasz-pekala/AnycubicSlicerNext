#include "manager_plugin.hpp"
#include "app_update.hpp"
#include "package_update.hpp"
#include "preset_update.hpp"

#include <plugins_base/funcation.hxx>
#include <plugins_sdk/constant/config.hxx>

#include <assert.h>

struct Instace {
  virtual bool has_package(const wxString &name) = 0;
  virtual bool check_update(const wxString &name, int manual_check) = 0;
  virtual bool start_download(const wxString &name) = 0;
};

template <typename T> struct InstaceImpl : public Instace {
public:
  template <typename... Args>
  InstaceImpl(Args &&...args) : instance_(std::forward<Args>(args)...) {}

public:
  bool has_package(const wxString &name) override {
    return instance_.has_package(name);
  }
  bool check_update(const wxString &name, int manual_check) override {
    return instance_.check_update(name, manual_check);
  }
  bool start_download(const wxString &name) override {
    return instance_.start_download(name);
  }

private:
  T instance_;
};

template <typename... Ts>
std::vector<std::shared_ptr<Instace>> make_shared(const wxString &package_dir,
                                                  const wxString &plugins_dir) {
  return {std::make_shared<InstaceImpl<Ts>>(package_dir, plugins_dir)...};
}

ManagerPlugin::ManagerPlugin(Anycubic::Plugins::PluginHost *host)
    : host_(host) {
  assert(host_ != nullptr);
  auto router = host_->Router();
  assert(router != nullptr);
  router->REGISTER_FUNCATION(ManagerPlugin, has_package);
  router->REGISTER_FUNCATION(ManagerPlugin, check_update);
  router->REGISTER_FUNCATION(ManagerPlugin, start_download_plugin);
  wxString package_dir;
  wxString plugins_dir;
  host_->GetValue(CONFIG_PACKAGE_PATH, package_dir);
  host_->GetValue(CONFIG_PLUGINS_PATH, plugins_dir);
  assert(package_dir.IsEmpty() == false);
  assert(plugins_dir.IsEmpty() == false);
  instances_ = make_shared<AppUpdate, PresetUpdate, PackageUpdate>(package_dir,
                                                                   plugins_dir);
}

bool ManagerPlugin::Start(void) {
  check_update(wxEmptyString, 0);
  return true;
}

void ManagerPlugin::Stop(void) {}

bool ManagerPlugin::has_package(const wxString &name) {
  for (auto &instance : instances_) {
    if (instance->has_package(name)) {
      return true;
    }
  }
  return false;
}
bool ManagerPlugin::start_download_plugin(const wxString& name) { return false; }
bool ManagerPlugin::check_update(const wxString &name, int manual_check) {
  for (auto &instance : instances_) {
    if (instance->check_update(name, manual_check)) {
      return true;
    }
  }
  return false;
}
