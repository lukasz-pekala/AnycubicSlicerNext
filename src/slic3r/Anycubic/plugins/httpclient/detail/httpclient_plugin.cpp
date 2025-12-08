#include "httpclient_plugin.hpp"

#include <slic3r/Utils/Http.hpp>

#include <wx/filename.h>

#include <boost/log/trivial.hpp>

#define DOWNLOAD_BODY_SIZE (1024 * 1024 * 100) // 100MB
const char *skip_whitespace(const char *current) {
  while (*current == ' ' || *current == '\t' || *current == '\n' ||
         *current == '\r') {
    current++;
  };
  return current;
}
HttpClientPlugin::HttpClientPlugin(Anycubic::Plugins::PluginHost *host) {}

HttpClientPlugin::~HttpClientPlugin() {}

bool HttpClientPlugin::Download(const wxString &url, const wxString &path) {
  assert(!url.IsEmpty());
  assert(!path.IsEmpty());
  auto fn = wxFileName::FileName(path);
  if (!fn.IsOk() || fn.IsDir())
    return false;
  auto path_str = fn.GetPath();
  if (!wxFileName::DirExists(path_str) && !wxFileName::Mkdir(path_str)) {
    return false;
  }

  Slic3r::Http client = Slic3r::Http::get(url.utf8_string());
  bool result = false;
  client
      .on_complete([&result, &path](std::string body, unsigned http_status) {
        if (http_status == 200) {
          wxFileOutputStream(path).Write(body.data(), body.size());
          result = true;
        }
      })
#ifndef NDEBUG
      .on_header_callback([](std::string header) {
        static size_t header_lenth = strlen("Content-Length");
        if (auto pos = header.find("Content-Length");
            pos != std::string::npos) {
          auto lenStart = header.c_str() + pos + header_lenth;
          lenStart = skip_whitespace(lenStart);
          if (auto content_length = strtol(lenStart, nullptr, 10);
              content_length > DOWNLOAD_BODY_SIZE) {
            BOOST_LOG_TRIVIAL(info)
                << "download body size is larger than 100MB;Content-Length: "
                << content_length;
          }
        }
      });
#endif
  return result;
}

bool HttpClientPlugin::Get(const wxString &url, wxString *content) {
  assert(content != nullptr);
  Slic3r::Http client = Slic3r::Http::get(url.utf8_string());
  client.on_complete([content](std::string body, unsigned http_status) {
    if (http_status == 200)
      *content = wxString::FromUTF8(body.c_str());
  });
  client.perform_sync();
  return content->IsEmpty() == false;
}

bool HttpClientPlugin::Post(const wxString &url, wxString *content) {
  assert(content != nullptr);
  Slic3r::Http client = Slic3r::Http::post(url.utf8_string());
  client.on_complete([content](std::string body, unsigned http_status) {
    if (http_status == 200)
      *content = wxString::FromUTF8(body.c_str());
  });
  client.perform_sync();
  return content->IsEmpty() == false;
}

const char *HttpClientPlugin::Name(void) { return PLUGIN_NAME_STR; }
bool HttpClientPlugin::Start(void) { return true; }
void HttpClientPlugin::Destroy(void) { delete this; }
