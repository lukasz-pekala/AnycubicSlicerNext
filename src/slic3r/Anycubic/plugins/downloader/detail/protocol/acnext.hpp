#pragma once

#include "protocol.hpp"

class DownloaderPlugin;

class ACNextProtocol : public Protocol {
public:
  ACNextProtocol(DownloaderPlugin *owner);
  ~ACNextProtocol() override = default;
  bool can_handle() override;
  bool start(size_t id, const wxString &output_path) override;

private:
  std::string get_jsonvalue();
  std::string get_downloadurl(const struct WebWakeUpInfo &info);
  int32_t get_url_index() const;
  const char *get_url() const;

private:
  DownloaderPlugin *owner_;
};