#include "check_protocol.hpp"
#include "os.hpp"
#include <slic3r/Anycubic/detail/anonymous.hpp>

#include <plugins_base/funcation.hxx>

#include <utility/json/reader.hxx>
#include <utility/utils/range.hxx>

#include <iguana/detail/string_stream.hpp>
#include <iguana/detail/traits.hpp>
#include <iguana/json.hpp>
#include <utility/json/reader.hxx>

#include <libslic3r/SemVer.hpp>
#include <slic3r/Utils/Http.hpp>

#include <boost/log/trivial.hpp>

#include <wx/filename.h>

#include <algorithm>
#include <future>

#define CONFIG_PLUGIN_INFO_KEY "plugin.info"
#define CONFIG_PLUGIN_NAME_KEY "plugin.name"

#define UNIVERSE_DOMAIN "https://cloud-universe.anycubic.com"
#define UNIVERSE_TEST_DOMAIN "https://cloud-universe-test.anycubic.com"
#define CHINA_DOMAIN "https://cloud-platform.anycubic.com"
#define CHINA_TEST_DOMAIN "https://cloud-platform-test.anycubicloud.com"

#define TEST_URI_PREFIX "/p/t/workbench"
#define PRODUCTION_URI_PREFIX "/p/p/workbench"

#define PLUGIN_CHECK_UPDATE_URI "/api/v2/network_plugin/info"
#define PLUGIN_NAME_LIST_URI "/api/v2/network_plugin/getPluginNameList"
#define CHECK_APP_UPDATE_URI "/api/portal/index/get_version"

#define MAKE_URL(domain, prefix, uri) (domain prefix uri)

static const char *check_plugin_update_url[] = {
    MAKE_URL(UNIVERSE_DOMAIN, PRODUCTION_URI_PREFIX, PLUGIN_CHECK_UPDATE_URI),
    MAKE_URL(UNIVERSE_TEST_DOMAIN, TEST_URI_PREFIX, PLUGIN_CHECK_UPDATE_URI),
    MAKE_URL(CHINA_DOMAIN, PRODUCTION_URI_PREFIX, PLUGIN_CHECK_UPDATE_URI),
    MAKE_URL(CHINA_TEST_DOMAIN, TEST_URI_PREFIX, PLUGIN_CHECK_UPDATE_URI)};
static const char *get_name_list_url[] = {
    MAKE_URL(UNIVERSE_DOMAIN, PRODUCTION_URI_PREFIX, PLUGIN_NAME_LIST_URI),
    MAKE_URL(UNIVERSE_TEST_DOMAIN, TEST_URI_PREFIX, PLUGIN_NAME_LIST_URI),
    MAKE_URL(CHINA_DOMAIN, PRODUCTION_URI_PREFIX, PLUGIN_NAME_LIST_URI),
    MAKE_URL(CHINA_TEST_DOMAIN, TEST_URI_PREFIX, PLUGIN_NAME_LIST_URI)};
static const char *check_app_update_url[] = {
    MAKE_URL(UNIVERSE_DOMAIN, PRODUCTION_URI_PREFIX, CHECK_APP_UPDATE_URI),
    MAKE_URL(UNIVERSE_TEST_DOMAIN, TEST_URI_PREFIX, CHECK_APP_UPDATE_URI),
    MAKE_URL(CHINA_DOMAIN, PRODUCTION_URI_PREFIX, CHECK_APP_UPDATE_URI),
    MAKE_URL(CHINA_TEST_DOMAIN, TEST_URI_PREFIX, CHECK_APP_UPDATE_URI)};

struct PluginBase {
  std::string name;    ///< 插件名称
  std::string version; ///< 插件版本
};

struct CheckParams {
  std::string main_version; ///< 主程序版本
  std::string os_name;      ///< 操作系统名称 -- 不要填写
  std::string os_version;   ///< 操作系统版本 -- 不要填写
  std::string architecture; ///< 架构取值可能为“x86_64”，“arm64”等 -- 不要填写
  std::vector<PluginBase> plugins; ///< 插件列表
};
struct AppCheckParams {
  int32_t platform;     ///< 平台
  int32_t version_code; ///< 版本号
};

template <typename T> struct ResponseT {
  int32_t code;    ///< 状态码
  std::string msg; ///< 状态描述
  T data;          ///< 更新信息
};

struct PluginInfo {
  std::string name;     ///< 插件名称
  std::string version;  ///< 插件版本
  std::string md5;      ///< 插件md5
  std::string filename; ///< 插件文件名
};
struct Plugin {
  std::string plugin_key;
  std::vector<std::string>
      plugin_versions; ///< 名称列表(列表中的版本都不允许加载)
};
struct NameListResponse {
  std::vector<Plugin> plugins;
};

#define DECLARE_RESPONSE(tname, type)                                          \
  using tname = ResponseT<type>;                                               \
  REFLECTION(tname, code, msg, data)

DECLARE_RESPONSE(NameListData, NameListResponse);
DECLARE_RESPONSE(PluginUpdateData, std::vector<CheckResponse>);
DECLARE_RESPONSE(AppUpdateData, UpdateAppResponse);

REFLECTION(AppCheckParams, platform, version_code);
REFLECTION(UpdateAppResponse, id, version_code, platform, size, download_url,
           package_md5, version_name, update_content_cn, update_content_us,
           content);
REFLECTION(NameListResponse, plugins);
REFLECTION(Plugin, plugin_key, plugin_versions);
REFLECTION(CheckResponse, download_url, filename, md5, update_log_en,
           update_log_cn);
REFLECTION(CheckParams, main_version, os_name, os_version, architecture);
REFLECTION(PluginInfo, name, version, md5, filename);

template <typename T>
constexpr bool is_reflection_v = iguana::is_reflection<T>::value;

template <typename T> std::string to_json(const T &t) {
  iguana::string_stream ss;
  iguana::json::to_json(ss, t);
  return ss.str();
}

template <typename T> bool from_json(T &t, const std::string &body) {
  return Anycubic::utility::json::load_from_json(t, body.data(), body.size());
}

bool get_params(CheckParams &tmp) {
  tmp.main_version = SoftFever_VERSION;
  operating_system_info *os_info = nullptr;
  if (getOpratingSystemInfo(&os_info) && os_info != nullptr) {
    tmp.os_version = os_info->kernel_version;
    tmp.architecture = os_info->arch;
    tmp.os_name = os_info->name;
  } else {
    tmp.architecture = "x86_64";
#if defined(_WIN32)
    tmp.os_version = "10.0.0.0";
    tmp.os_name = "windows";
#elif defined(__linux__)
    tmp.os_version = "24.04";
    tmp.os_name = "ubuntu";
#elif defined(__APPLE__)
    tmp.os_version = "13.7.8";
    tmp.os_name = "darwin";
    if (isArmArch()) {
      tmp.architecture = "arm64";
    }
#else
#error "Unsupported platform"
#endif
  }
  return true;
}

template <typename BodyType, typename ResponseType>
std::enable_if_t<is_reflection_v<BodyType>, bool>
send_request(const char *url, const BodyType &params,
             decltype(ResponseType::data) &response) {
  Slic3r::Http client = Slic3r::Http::post(url);

  auto body = to_json(params);

  bool result = false; // 默认就是失败了
  client.set_post_body(body)
      .header("Content-Type", "application/json")
      .on_complete(
          [&response, &result](std::string body, unsigned http_status) {
            ResponseType data;
            result =
                (http_status == 200 && from_json(data, body) && data.code == 1);
            if (result) {
              response = data.data;
            }
          })
      .on_error([](std::string body, std::string url, unsigned http_status) {
        // 打印错误信息
        body += "\nurl: " + url;
      })
      .perform_sync();
  return result;
}

bool load_plugin_info(PMConfig *config, std::vector<PluginInfo> &plugins) {
  wxString body;
  bool result =
      config->GetEncryptValue(CONFIG_PLUGIN_INFO_KEY, body) && !body.IsEmpty();
  if (result) {
    result = from_json(plugins, body.utf8_string());
  }
  return result;
}
bool save_plugin_info(PMConfig *config, std::vector<PluginInfo> &plugins) {
  auto body = to_json(plugins);

  return config->SetEncryptValue(CONFIG_PLUGIN_INFO_KEY,
                                 wxString::FromUTF8(body));
}

int UpdateChecker::url_index(void) const {
  int index = 0;
  if (is_china_env_) {
    index += 2;
  }
  if (is_test_env_) {
    index += 1;
  }

  return index;
}

bool UpdateChecker::check_app_update_available(
    UpdateAppResponse &response) const {
  AppCheckParams params;
  params.version_code = VERSION_CODE;
#if defined(_WIN32)
  params.platform = 7; // win平台FDM
#elif defined(__linux__)
  params.platform = 10; // linux FDM
#elif defined(__APPLE__)
  if (isArmArch()) {
    params.platform = 9; // mac平台FDM M系列芯片
  } else {
    params.platform = 8; // mac平台FDM intel芯片
  }
#else
#error "Unsupported platform"
#endif
  // 发送请求检查应用更新
  auto url = check_app_update_url[url_index()];
  auto feature = std::async(std::launch::async, [url, &params, &response] {
    return send_request<AppCheckParams, AppUpdateData>(url, params, response);
  });
  do {
    wxYield();
  } while (feature.wait_for(std::chrono::milliseconds(100)) !=
           std::future_status::ready);
  return feature.get();
}

bool UpdateChecker::check_preset_update_available(
    CheckResponse &response) const {
  return false;
}

bool UpdateChecker::check_plugin_update_available(
    std::vector<CheckResponse> &response) const {
  // 1. 从配置中读取已保存的白名单插件名称
  wxString name_json;
  config_->GetEncryptValue(CONFIG_PLUGIN_NAME_KEY, name_json);
  if (name_json.IsEmpty()) {
    return false; // 没有白名单，无法检查更新
  }

  // 2. 解析白名单JSON为字符串列表
  std::vector<std::string> name_list;
  from_json(name_list, name_json.utf8_string());

  // 3. 加载本地已安装的插件信息
  std::vector<PluginInfo> plugins;
  load_plugin_info(config_, plugins);

  // 4. 构建检查更新的请求参数
  CheckParams params;
  get_params(params); // 获取系统参数（版本、操作系统等）

  // 5. 准备插件列表参数，预分配空间优化性能
  std::vector<PluginBase> &paramsp = params.plugins;
  paramsp.reserve(name_list.size());

  // 6. 添加插件名称到请求参数（初始版本为空）
  for (auto &name : name_list) {
    paramsp.push_back({name, ""});
  }

  // 7. 为每个插件查找对应的本地版本信息
  for (auto &p : paramsp) {
    auto itr = std::find_if(
        plugins.begin(), plugins.end(),
        [&p](const PluginInfo &item) { return item.name == p.name; });
    if (itr != plugins.end()) {
      p.version = itr->version; // 设置插件版本
    }
  }

  // 8. 发送更新检查请求到服务器
  return ::send_request<CheckParams, PluginUpdateData>(
      check_plugin_update_url[url_index()], params, response);
}

bool UpdateChecker::update_plugin_name_list(
    std::vector<std::string> &name_list) const {
  name_list.clear();
  NameListResponse nameListData;
  CheckParams params;
  get_params(params);
  if (!::send_request<CheckParams, NameListData>(get_name_list_url[url_index()],
                                                 params, nameListData)) {
    return false;
  }
  auto &nameList = nameListData.plugins;
  name_list.resize(nameList.size()); /// 预分配空间
  std::transform(nameList.begin(), nameList.end(), name_list.begin(),
                 [](const auto &item) { return item.plugin_key; });
  auto name_json = to_json(name_list);
  config_->SetEncryptValue(CONFIG_PLUGIN_NAME_KEY,
                           wxString::FromUTF8(name_json));
  name_list.clear();

  // 先备好名单
  std::vector<PluginInfo> plugins;
  load_plugin_info(config_, plugins); // 加载成功与否并不是很重要

  std::vector<PluginInfo> enable_plugins;

  name_list.reserve(nameList.size()); /// 预分配空间
  for (const auto &nameItem : nameList) {
    assert(!nameItem.plugin_key.empty() && "Plugin key should not be empty");

    if (nameItem.plugin_versions.empty()) {
      // 没有禁用版本，直接加入白名单
      name_list.push_back(nameItem.plugin_key);
      continue;
    }

    // 检查本地是否有该插件，且版本是否在禁用列表中
    bool should_include = true;
    for (const auto &plugin : plugins) {
      if (plugin.name == nameItem.plugin_key) {
        Slic3r::Semver currentVersion(plugin.version);
        for (const auto &banned_version : nameItem.plugin_versions) {
          Slic3r::Semver ver(banned_version);
          if (ver == currentVersion) {
            should_include = false; // 版本被禁用，不加入白名单
            // 删除对应包的文件
            wxRemoveFile(wxString::FromUTF8(plugin.filename));
            break;
          }
        }
        // 如果本地有开放的插件，保存其信息
        if (should_include) {
          enable_plugins.push_back(plugin);
        }
        break; // 找到对应插件后退出
      }
    }

    if (should_include) {
      name_list.push_back(nameItem.plugin_key);
    }
  }
  // 保存有效的插件信息
  save_plugin_info(config_, enable_plugins);

  return name_list.empty() == false;
}

UpdateChecker::UpdateChecker(PMConfig *config) : config_(config) {
  assert(config_ != nullptr);
  wxString region;
  config_->GetValue("region", region);
  is_china_env_ = region.utf8_string() != "Global";

  wxString developer_mode;
  config_->GetValue("developer_mode", developer_mode);
  is_test_env_ = developer_mode == "1" || developer_mode == "true";
  pcid_ = Slic3r::GetPCID(nullptr).ToStdString();
}