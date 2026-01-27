#pragma once
#include <easy_log/log.hxx>

class wxString;
class wxWindow;

namespace Slic3r {
class AppConfig;
namespace GUI {

class AnycubicContextPrivate;

class AnycubicContext : public wxEvtHandler {
public:
  explicit AnycubicContext(AppConfig *app_config);
  ~AnycubicContext();
  bool PluginsIsLoaded() const;
  bool AddWindow(const wxString &position, wxWindow *window);
  bool HasPlugin() const;                                 ///< 是否存在插件
  bool StartDownloadPlugins(bool is_auto_update = false); ///< 启动下载插件
  /**
   * @brief 更新插件配置
   *
   */
  void UpdatePluginConfig(void);

  /**
   * @brief 更新打印配置
   *
   */
  void UpdatePreset(void);

  /**
   * @brief 更新主程序
   *
   */
  void UpdateApp(bool is_auto_update = false); ///< 更新主程序

  bool StartDownload(const wxString &url); ///< 启动下载
  void OnInitByApp();                      ///< 在 new MainFrame 之前调用
  void OnInitByGui();                      ///< 在 new MainFrame 构造时调用
  void OnFinishedByGui();                  ///< GUI初始化完成后调用
  void OnExitByGui();                      ///< 在 MainFrame::shutdown() 之前
  void OnExitByApp();                      ///< 在 delete MainFrame 销毁之后
private:
  int ShowUpdateVersionDialog(const wxString &extmsg,
                               const wxString &version_str,
                               bool is_skip_version = false);

private:
  AnycubicContextPrivate *impl_;
};

} // namespace GUI
} // namespace Slic3r
