#include "app_plugin.hpp"

#include <slic3r/GUI/GUI_App.hpp>
#include <slic3r/GUI/MainFrame.hpp>
#include <slic3r/GUI/Plater.hpp>

#include <plugins_base/funcation.hxx>

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

  bool res = Slic3r::GUI::wxGetApp().plater()->load_files(filenames);
  Slic3r::GUI::wxGetApp().mainframe->update_title();
  return res ? 0 : -1;
}

bool AppPlugin::current_gcode_file(wxString *path) {
  assert(path != nullptr);
  *path = Slic3r::GUI::wxGetApp().plater()->current_gcode_file();
  return path->IsEmpty() == false;
}

wxString AppPlugin::recent_projects(void) {  
  boost::property_tree::wptree data;
  wxGetApp().mainframe->get_recent_projects(data, INT_MAX);
  std::wostringstream oss;
  boost::property_tree::write_json(oss, data, false);
  return oss.str(); 
}
