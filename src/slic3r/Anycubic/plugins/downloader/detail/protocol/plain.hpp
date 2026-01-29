#pragma once

#include "protocol.hpp"
class DownloaderPlugin;
class PlainProtocol : public Protocol {
public:
  PlainProtocol(DownloaderPlugin *owner);
  ~PlainProtocol() override = default;
  bool can_handle() override;
  bool start(size_t id, const wxString &output_path) override;

private:
  DownloaderPlugin *owner_;
};