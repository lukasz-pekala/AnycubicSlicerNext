#pragma once

#include <plugins_manager/plugins_manager.hxx>

#include <string>

struct CheckResponse {
  std::string download_url;  ///< 下载链接
  std::string filename;      ///< 包名
  std::string md5;           ///< 文件md5值
  std::string update_log_en; ///< 英文更新日志
  std::string update_log_cn; ///< 中文更新日志
};

struct UpdateAppResponse {
  int32_t id;
  int32_t version_code; ///< 版本号121
  int32_t
      platform; ///< 平台：  1=Android，  2=IOS，  3=win平台workshop，
                ///<  4=mac平台workshop,  5=win平台acslicer，  6=mac平台acslicer
  int32_t is_force_update;          ///< 是否强制更新：0=否，1=是
  int32_t is_show;                  ///< 是否在app启动时显示，0=否，1=是
  int32_t create_time;              ///< 版本创建时间
  std::string size;                 ///<
  std::string download_url;         ///< 下载url
  std::string version_name;         ///< 版本名称eg v1.2.1
  std::string package_md5;          ///< md5
  std::string update_content_cn;    ///< 更新内容中文，切片软件用得到
  std::string update_content_us;    ///< 更新内容英文，切片软件用得到
  std::vector<std::string> content; ///< 具体的更新内容
};

class UpdateChecker {
public:
  bool check_app_update_available(UpdateAppResponse &response) const;
  bool check_preset_update_available(CheckResponse &response) const;
  bool
  check_plugin_update_available(std::vector<CheckResponse> &response) const;
  bool update_plugin_name_list(std::vector<std::string> &name_list) const;

  bool is_china_env(void) const { return is_china_env_; }

public:
  UpdateChecker(PMConfig *config);

private:
  int url_index(void) const;

private:
  PMConfig *config_;
  bool is_china_env_;
  bool is_test_env_;
  std::string pcid_;
};
