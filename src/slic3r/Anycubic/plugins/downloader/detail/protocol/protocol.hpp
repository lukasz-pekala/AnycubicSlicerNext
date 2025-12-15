#pragma once
#include <utility/utils/query.hxx>

#include <wx/string.h>
#include <wx/uri.h>

typedef void (*download_callback)(void *ctx, size_t download_id, int32_t status,
                                  const wxString &filename);

class Protocol {
public:
  virtual ~Protocol() = default;
  virtual bool can_handle() = 0;
  virtual bool start(size_t id, const wxString &output_path) = 0;
  virtual bool parse_url(const wxString &url);

protected:
  wxURI uri_;
};
