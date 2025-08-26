#include "utility_plugin.hpp"
#include "slic3r/Anycubic/detail/anonymous.hpp"

#include <plugins_base/funcation.hxx>

UtilityPlugin::UtilityPlugin(Anycubic::Plugins::PluginHost *host)
    : host_(host) {
  assert(host_ != nullptr);
  host_->Router()->REGISTER_FUNCATION(UtilityPlugin, pcid);
}

UtilityPlugin::~UtilityPlugin() {}

std::string UtilityPlugin::pcid(void) const {
  return Slic3r::GetPCID(nullptr).ToStdString();
}

bool UtilityPlugin::AttachEvt(wxEvtHandler *) { return false; }

bool UtilityPlugin::DetachEvt(wxEvtHandler *) { return false; }

bool UtilityPlugin::CreateDialog(wxDialog *dlg, wxWindow *parent,
                                 wxString *bmp) {
  return false;
}

bool UtilityPlugin::CreateWebview(wxWebView *view, wxWindow *parent,
                                  wxString *bmp) {
  return false;
}

void UtilityPlugin::Destroy(void) { delete this; }
