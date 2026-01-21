#include "manager_plugin.hpp"
#include "app_update.hpp"
#include "package_update.hpp"
#include "preset_update.hpp"

#include <plugins_base/funcation.hxx>
#include <plugins_sdk/constant/config.hxx>

#include <utility/utils/string.hxx>

#include <boost/scope_exit.hpp>

#include <assert.h>

#include <functional>
#include <thread>

const char *CONFIG_PLUGIN_NAME_LIST = "plugins.names";
const char *CONFIG_PLUGIN_PACKAGES = "plugins.packages";

ManagerPlugin::ManagerPlugin(Anycubic::Plugins::PluginHost *host)
    : host_(host), running_(false) {
  assert(host_ != nullptr);
  auto router = host_->Router();
  assert(router != nullptr);
  router->REGISTER_FUNCATION(ManagerPlugin, has_package);
  router->REGISTER_FUNCATION(ManagerPlugin, check_update);
  router->REGISTER_FUNCATION(ManagerPlugin, start_download_plugin);
}

bool ManagerPlugin::Start(void) {
  std::thread(std::bind(&ManagerPlugin::check_update_impl, this, 0)).detach();
  return true;
}

void ManagerPlugin::Stop(void) {}

bool ManagerPlugin::has_package(const wxString &name) { return false; }
bool ManagerPlugin::start_download_plugin(const wxString &name) {
  return false;
}
bool ManagerPlugin::check_update(const wxString &name, int manual_check) {
  return false;
}

void ManagerPlugin::check_update_impl(int manual_check) {
  bool value = false;
  while (running_.compare_exchange_weak(value, true)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  auto running = &running_;
  BOOST_SCOPE_EXIT(running) { running->store(false); }
  BOOST_SCOPE_EXIT_END;
  // TODO:
  // 1、 从服务端获取插件名单

  // 2、 名单写入配置

  // 3、 读取已启用插件包信息

  // 先移除名单中不存在的插件包
  // 先保存记录--防止下次再次加载

  // 4、 根据名单检查插件是否需要更新
  // 5、 更新插件信息显示
}
