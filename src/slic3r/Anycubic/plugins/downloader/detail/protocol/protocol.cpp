#include "protocol.hpp"

bool Protocol::parse_url(const wxString &url) {
  if (!uri_.Create(url)) {
    return false;
  }
  return uri_.HasScheme();
}

wxString Protocol::get_filename() const {
  if (uri_.HasPath()) {
    return wxFileName::FileName(uri_.GetPath()).GetFullName();
  }
  // 随机命名
  return wxString::Format(wxT("download_%d"), wxGetUTCTime());
}
