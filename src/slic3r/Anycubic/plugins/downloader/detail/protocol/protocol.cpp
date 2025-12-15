#include "protocol.hpp"

bool Protocol::parse_url(const wxString &url) {
  if (!uri_.Create(url)) {
    return false;
  }
  return uri_.HasScheme();
}
