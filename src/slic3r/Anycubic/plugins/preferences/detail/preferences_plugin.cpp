#include "preferences_plugin.hpp"

#include <plugins_base/funcation.hxx>

#include <slic3r/GUI/Preferences.hpp>

PreferencesPlugin::PreferencesPlugin(Anycubic::Plugins::PluginHost *host)
    : host_(host) {
  auto router = host_->Router();
  assert(router != nullptr);

  router->REGISTER_FUNCATION(PreferencesPlugin, register_tab);

  host_->WatchWindow("Preferences", this, [](void *ctx, wxWindow *wnd) {
    assert(wxIsMainThread());
    PreferencesPlugin *p = reinterpret_cast<PreferencesPlugin *>(ctx);
    assert(p != nullptr && wnd != nullptr);
    p->CreateTabs(wnd);

    return false; // 标记不存储处理
  });
}

void PreferencesPlugin::CreateTabs(wxWindow *parent) {
  assert(parent != nullptr);
  auto dialog = dynamic_cast<Slic3r::GUI::PreferencesDialog *>(parent);
  assert(dialog != nullptr);
  for (auto &tab : tabs_) {
    auto panel = host_->CreatePanel(dialog, tab.xrcName, tab.xrc);
    assert(panel != nullptr);
    auto p = host_->GetPlugin(tab.pluginName.utf8_string().c_str());
    assert(p != nullptr);
    p->CreatePanel(panel);
    // 写入tab到parent
    auto result = dialog->InsertPanel(tab.title, panel, -1, tab.iconName);

    LOG_INFO("InsertPanel :{}, result={}", tab.title.utf8_string(), result);
  }
}

bool PreferencesPlugin::register_tab(const wxString &plugin_name,
                                     const wxString &title,
                                     const wxString &icon,
                                     const wxString &xrcName,
                                     const wxString *xrc) {
  assert(xrc != nullptr && xrc->IsEmpty() == false);
  assert(plugin_name.IsEmpty() == false);
  if (host_->HasPlugin(plugin_name.utf8_string().c_str())) {
    return false;
  }
  tabs_.push_back({plugin_name, title, icon, xrcName, *xrc});
  return false;
}
