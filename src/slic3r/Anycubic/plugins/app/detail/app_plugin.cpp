#include "app_plugin.hpp"

#include <slic3r/GUI/GUI_App.hpp>
#include <slic3r/GUI/MainFrame.hpp>
#include <slic3r/GUI/Notebook.hpp>
#include <slic3r/GUI/Plater.hpp>

#include <plugins_base/funcation.hxx>
#include <plugins_sdk/webview/detail/script_format.hxx>

#include <wx/filename.h>
#include <wx/tokenzr.h>

#include <limits.h>

AppPlugin::AppPlugin(Anycubic::Plugins::PluginHost *host) : host_(host) {
  assert(host_ != nullptr);
  auto router = host_->Router();
  assert(router != nullptr);
  router->REGISTER_FUNCATION(AppPlugin, import);
  router->REGISTER_FUNCATION(AppPlugin, current_gcode_file);
  router->REGISTER_FUNCATION(AppPlugin, recent_projects);
  router->REGISTER_FUNCATION(AppPlugin, handler_web_request);
}

AppPlugin::~AppPlugin() {}

int32_t AppPlugin::import(const wxString *paths) {
  assert(paths != nullptr);

  wxArrayString filenames;

  // 使用 wxStringTokenizer 来分割 paths 字符串
  wxStringTokenizer tokenizer(*paths, wxASCII_STR("?"));
  while (tokenizer.HasMoreTokens()) {
    wxString path = tokenizer.GetNextToken();
    if (!path.IsEmpty() && wxFileName::FileExists(path)) {
      filenames.Add(path);
    }
  }
  // NOTE: 切换TAB页
  // 切换到打印页
  if (auto tab = Slic3r::GUI::wxGetApp().mainframe->m_tabpanel;
      tab != nullptr) {
    auto size = static_cast<int32_t>(tab->GetPageCount());
    for (auto idx = 0; idx < size; ++idx) {
      if (auto txt = tab->GetPageText(idx); txt == _("Prepare")) {
        tab->SetSelection(idx);
        break;
      }
    }
  }
  bool res = Slic3r::GUI::wxGetApp().plater()->load_files(filenames);
  Slic3r::GUI::wxGetApp().mainframe->update_title();
  return res ? 0 : -1;
}

bool AppPlugin::current_gcode_file(wxString *path) {
  assert(path != nullptr);
  *path = Slic3r::GUI::wxGetApp().plater()->current_gcode_file();
  return path->IsEmpty() == false;
}

bool AppPlugin::recent_projects(wxString *json) {
  assert(json != nullptr);
  boost::property_tree::wptree data;
  wxGetApp().mainframe->get_recent_projects(data, INT_MAX);
  std::wostringstream oss;
  boost::property_tree::write_json(oss, data, false);
  *json = wxString(oss.str());
  return json->IsEmpty() == false;
}

void AppPlugin::handler_web_request(wxWebView *view, const wxString *cmd) {
  if (cmd == nullptr || cmd->IsEmpty()) {
    return;
  }
  auto response =
      Slic3r::GUI::wxGetApp().handle_web_request(cmd->utf8_string());
  if (response.empty() || view == nullptr) {
    return;
  }

  response.erase(std::remove(response.begin(), response.end(), '\n'),
                 response.end());
  if (!response.empty()) {
    auto jsresponse = ScriptFormat("window.postMessage", response);
    RunScript(view, jsresponse);
  }
}
