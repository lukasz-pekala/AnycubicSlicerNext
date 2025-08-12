#include "tabbook_plugin.hpp"

#include <plugins_base/funcation.hxx>

#include <assert.h>

#include <wx/string.h>
#include <wx/xrc/xmlres.h>

#include <slic3r/GUI/Tabbook.hpp>

TabBookPlugin::TabBookPlugin(Anycubic::Plugins::PluginHost *host)
    : host_(host) {
  assert(host_ != nullptr);
  host_->Router()->REGISTER_FUNCATION(TabBookPlugin, CreateTab);
  host_->Router()->REGISTER_FUNCATION(TabBookPlugin, RemoveTab);
}
wxString RandomString(int length) {
  wxString str;
  for (int i = 0; i < length; ++i) {
    str += (wxChar)('a' + rand() % 26);
  }
  return str;
}
TabBookPlugin::~TabBookPlugin() {}

int32_t TabBookPlugin::CreateTab(int idx, const wxString &title,
                                 const wxString &icon, const wxString &xrcName,
                                 const wxString &xrc) {

  Tabbook *tabbook = dynamic_cast<Tabbook *>(host_->GetWindow("tabbook"));
  if (tabbook == nullptr) {
    return -1;
  }

  wxString name = RandomString(16) + wxASCII_STR(".xrc");
  host_->AddFS(name, xrc);
  auto pXRC = wxXmlResource::Get();
  pXRC->Load(wxASCII_STR("memory://") + name);
  host_->DelFS(name);

  wxWindow *panel = pXRC->LoadPanel(tabbook, xrcName);
  if (panel == nullptr) {
    return -1;
  }
  auto ret =
      tabbook->InsertNewPage(size_t(idx), panel, title, icon.utf8_string());
  if (ret)
    return 0;
  return -1;
}

int32_t TabBookPlugin::RemoveTab(int idx) {
  Tabbook *tabbook = dynamic_cast<Tabbook *>(host_->GetWindow("tabbook"));
  if (tabbook == nullptr) {
    return -1;
  }
  tabbook->RemovePage(size_t(idx));
  return 0;
}

bool TabBookPlugin::AttachEvt(wxEvtHandler *) { return false; }

bool TabBookPlugin::DetachEvt(wxEvtHandler *) { return false; }

bool TabBookPlugin::CreateDialog(wxDialog *dlg, wxWindow *parent,
                                 wxString *bmp) {
  return false;
}

bool TabBookPlugin::CreateWebview(wxWebView *view, wxWindow *parent,
                                  wxString *bmp) {
  return false;
}

void TabBookPlugin::Destroy(void) { delete this; }
