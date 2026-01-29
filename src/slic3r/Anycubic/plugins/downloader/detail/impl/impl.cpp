#include "impl.hpp"

#include <embed_all.h>

#include <plugins_base/funcation.hxx>

#include <wx/gdicmn.h>

#define DEFAULT_DIALOG_SIZE(name) wxSize name(500, 140)

bool is_test_env(Anycubic::Plugins::PluginHost *host) {
  return dispatch_call<bool>(host, "utility", "is_test_env");
}

bool is_china_env(Anycubic::Plugins::PluginHost *host) {
  return dispatch_call<bool>(host, "utility", "is_china_env");
}

bool show_progress_dialog(Anycubic::Plugins::PluginHost *host, const char *name,
                          const wxString &title) {
  wxString xrc = wxString::FromUTF8(
      reinterpret_cast<const char *>(PROGRESS_DIALOG_XRC__DATA()),
      PROGRESS_DIALOG_XRC__SIZE());
  DEFAULT_DIALOG_SIZE(size);
  return dispatch_call<bool>(host, "home_dialog", "show_dialog", name, title,
                             "", "DownloadProgress", &xrc, &size, true);
}