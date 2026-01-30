#include "plain.hpp"
#include "../downloader_plugin.hpp"

#include <slic3r/GUI/Downloader.hpp>

#include <wx/filename.h>

PlainProtocol::PlainProtocol(DownloaderPlugin *owner) : owner_(owner) {
  assert(owner_ != nullptr);
}

bool PlainProtocol::can_handle() {
  if (uri_.GetScheme() != wxASCII_STR("http") ||
      uri_.GetScheme() != wxASCII_STR("https")) {
    return true;
  }
  return false;
}

bool PlainProtocol::start(size_t id, const wxString &output_path) {
  wxFileName full_path = wxFileName::FileName(output_path);
  auto downloadurl = uri_.BuildURI();
  auto d = new Slic3r::GUI::Download(
      static_cast<int>(id), downloadurl.ToStdString(), owner_,
      full_path.GetFullName().utf8_string(), full_path.GetPath().utf8_string());
  owner_->start_download_impl(id, d);
  return true;
}
