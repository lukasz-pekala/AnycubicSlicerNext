#pragma once
#include <easy_log/log.hxx>

class wxString;
class wxWindow;

namespace Slic3r {
class AppConfig;
namespace GUI {

class AnycubicContextPrivate;

using PluginUpdateCallback =
    std::function<void(const wxString &name, const wxString &url,
                       const wxString &cnlog, const wxString &enlog)>;
class AnycubicContext {
public:
  explicit AnycubicContext(AppConfig *app_config);
  ~AnycubicContext();
  bool PluginsIsLoaded() const;
  bool AddWindow(const wxString &position, wxWindow *window);
  bool
  CheckUpdatePlugins(const PluginUpdateCallback &callback); ///< 检查插件更新
  bool HasPlugin() const;                                   ///< 是否存在插件
  void OnInitByApp();     ///< 在 new MainFrame 之前调用
  void OnInitByGui();     ///< 在 new MainFrame 构造时调用
  void OnFinishedByGui(); ///< GUI初始化完成后调用
  void OnExitByGui();     ///< 在 MainFrame::shutdown() 之前
  void OnExitByApp();     ///< 在 delete MainFrame 销毁之后

private:
  AnycubicContextPrivate *impl_;
};

} // namespace GUI
} // namespace Slic3r
