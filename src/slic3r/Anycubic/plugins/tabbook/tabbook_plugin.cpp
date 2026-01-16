#include "tabbook_plugin.hpp"

#include <plugins_base/funcation.hxx>

#include <assert.h>

#include <wx/string.h>
#include <wx/xrc/xmlres.h>

#include <slic3r/GUI/Notebook.hpp>

TabBookPlugin::TabBookPlugin(Anycubic::Plugins::PluginHost *host)
    : host_(host) {
  assert(host_ != nullptr);
  auto router = host_->Router();
  assert(router != nullptr);
  router->REGISTER_FUNCATION(TabBookPlugin, CreateTab);
  router->REGISTER_FUNCATION(TabBookPlugin, RemoveTab);
  router->REGISTER_FUNCATION(TabBookPlugin, GetTabTitle);
  router->REGISTER_FUNCATION(TabBookPlugin, GetTabCount);
}
wxString RandomString(int length) {
  wxString str;
  for (int i = 0; i < length; ++i) {
    str += (wxChar)('a' + rand() % 26);
  }
  return str;
}
TabBookPlugin::~TabBookPlugin() {}

int32_t TabBookPlugin::CreateTab(int idx, const std::string &title,
                                 const std::string &icon, wxWindow *panel) {

  Notebook *tabbook = dynamic_cast<Notebook *>(host_->GetWindow("tabpanel"));
  if (tabbook == nullptr) {
    return -1;
  }
  if (idx == -1) {
    idx = static_cast<int>(tabbook->GetPageCount());
  }
  auto ret = tabbook->InsertPage(size_t(idx), panel, wxString::FromUTF8(title),
                                 icon, icon);
  if (ret)
    return 0;
  return -1;
}

int32_t TabBookPlugin::RemoveTab(int idx) {
  Notebook *tabbook = dynamic_cast<Notebook *>(host_->GetWindow("tabpanel"));
  if (tabbook == nullptr) {
    return -1;
  }
  if (idx == -1) {
    idx = static_cast<int>(tabbook->GetPageCount()) - 1;
  }
  auto win = tabbook->GetPage(size_t(idx));
  if (tabbook->RemovePage(size_t(idx)) && win) {
    win->Destroy();
  }
  return 0;
}
int32_t TabBookPlugin::GetTabCount(void) const {
  Notebook *tabbook = dynamic_cast<Notebook *>(host_->GetWindow("tabpanel"));
  if (tabbook == nullptr) {
    return -1;
  }
  return static_cast<int32_t>(tabbook->GetPageCount());
}
std::string TabBookPlugin::GetTabTitle(int idx) const {
  Notebook *tabbook = dynamic_cast<Notebook *>(host_->GetWindow("tabpanel"));
  if (tabbook == nullptr) {
    return std::string();
  }
  assert(idx >= 0 && idx < static_cast<int>(tabbook->GetPageCount()));
  auto title = tabbook->GetPageText(size_t(idx));

  return title.Trim().Trim(false).utf8_string();
}
bool TabBookPlugin::AttachEvt(wxEvtHandler *) { return false; }

bool TabBookPlugin::DetachEvt(wxEvtHandler *) { return false; }

bool TabBookPlugin::BindEvt(wxPanel *panel, wxWindow *parent, wxString *bmp) {
  return false;
}

bool TabBookPlugin::CreateWebview(wxWebView *view, wxWindow *parent,
                                  wxString *bmp) {
  return false;
}

void TabBookPlugin::Destroy(void) { delete this; }
