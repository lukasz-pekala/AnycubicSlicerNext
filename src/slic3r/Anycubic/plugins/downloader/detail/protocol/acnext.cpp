#include "acnext.hpp"

#include "../downloader_plugin.hpp"

#include <slic3r/GUI/GUI_App.hpp>
#include <slic3r/GUI/I18N.hpp>
#include <slic3r/GUI/NotificationManager.hpp>

#include <utility/codec/base64.hxx>
#include <utility/json/reader.hxx>
#include <utility/utils/query.hxx>

#include <nlohmann/json.hpp>

#include <boost/log/trivial.hpp>

// clang-format off
#define m_baseUrl_cn_test "https://common-mo-itdev-cn.anycubic.com/file/fileService/download"
#define m_baseUrl_cn "https://api.makeronline.cn/file/fileService/download"
#define m_baseUrl_en_test "https://common-mo-itdev.anycubic.com/file/fileService/download"
#define m_baseUrl_en "https://api.makeronline.com/file/fileService/download"

#define URL_SWITCH(cn, cn_test, en, en_test)                                   \
  static const char *urls[] = {cn, cn_test, en, en_test};                      \
  return urls[get_url_index()];
// clang-format on

#define push_error(mngr, text, htext)                                          \
  mngr->push_notification(                                                     \
      Slic3r::GUI::NotificationType::URLDownload,                              \
      Slic3r::GUI::NotificationManager::NotificationLevel::                    \
          ErrorNotificationLevel,                                              \
      text, htext, [](wxEvtHandler *evt) { return true; }, 8000)

struct WebWakeUpInfo {
  std::string accessToken;
  std::string modelLink;
  std::string hash;
  std::string fileName;
  std::string userId;
  int fileId = -1;
  int fileType = -1;
  std::string regionCn;
  std::string prod;
};
REFLECTION(WebWakeUpInfo, accessToken, modelLink, hash, fileName, userId,
           fileId, fileType, regionCn, prod);

ACNextProtocol::ACNextProtocol(DownloaderPlugin *owner) : owner_(owner) {
  assert(owner_ != nullptr);
  ntf_mngr_ = Slic3r::GUI::wxGetApp().notification_manager();
  assert(ntf_mngr_ != nullptr);
}

bool ACNextProtocol::can_handle() {
  // acnext://open
  if (uri_.GetScheme() != wxASCII_STR("acnext")) {
    return false;
  }
  if (uri_.GetServer() != wxASCII_STR("open")) {
    return false;
  }
  return uri_.HasQuery();
}

bool ACNextProtocol::start(size_t id, const wxString &output_path) {
  auto json = get_jsonvalue();
  if (json.empty()) {
    return false;
  }
  WebWakeUpInfo info;
  if (!Anycubic::utility::json::load_from_json(info, json.data(),
                                               json.size())) {
    // 推送错误通知
    push_error(ntf_mngr_,
               _u8L("reflection error: Invalid jsonvalue parameter."),
               _u8L("Contact Us"));
    return false;
  }
  auto downloadurl = get_downloadurl(info);
  if (downloadurl.empty()) {
    return false;
  }

  auto d = new Slic3r::GUI::Download(static_cast<int>(id), downloadurl, owner_, info.fileName,
                                     output_path.ToStdString());
  owner_->start_download_impl(id, d);
  return true;
}

std::string ACNextProtocol::get_jsonvalue() {
    using namespace Anycubic::utility;
    auto query = uri_.GetQuery().utf8_string();
    assert(!query.empty());
    // acnext://open?jsonvalue=base64encodedjson
    auto jsonvalue = parse_query(query);
    if (jsonvalue.empty() || jsonvalue.count("jsonvalue") == 0) {
        // 推送错误通知
        push_error(ntf_mngr_,
                   _u8L("Invalid URL from makeronline.com: Missing or invalid "
                        "jsonvalue  parameter."),
                   _u8L("Contact Us"));
        return std::string(); 
    }
  std::string jsonvalue_;
  if (auto itr = jsonvalue.find("jsonvalue"); itr != jsonvalue.end()) {
    jsonvalue_ = itr->second;
  } else {
    return std::string();
  }
  return base64Decode(jsonvalue_);
}

std::string ACNextProtocol::get_downloadurl(const WebWakeUpInfo &info) {

  nlohmann::json jsonObj;
  jsonObj["sourceType"] = "9";
  jsonObj["ip"] = "127.0.0.1";
  jsonObj["userId"] = info.userId;
  jsonObj["batchDownLoadFlg"] = "0";
  jsonObj["hash"] = info.hash;
  jsonObj["fileName"] = info.fileName;
  std::string requestBody = jsonObj.dump();

  Slic3r::Http http = Slic3r::Http::post(get_url());

  std::string result;
  http.header("Content-Type", "application/json") // header
      .header("Authorization", info.accessToken)  // header
      .timeout_max(6)                             // seconds
      .set_post_body(requestBody)                 // body
      .on_complete([&result, this](std::string body, unsigned status) {
        const auto jsonObj = nlohmann::json::parse(body, nullptr, false, true);
        if (!jsonObj.is_discarded()) {
          auto urlitr = jsonObj.find("data");
          if (urlitr != jsonObj.end() && urlitr->is_string()) {
            result = *urlitr;
            return;
          }
        }
        BOOST_LOG_TRIVIAL(error)
            << __FUNCTION__ << "Download Failed! STATUS: " << status
            << ", body: " << body;
        push_error(ntf_mngr_, _u8L("Download Failed"), _u8L("Contact Us"));
      })
      .on_error([this](std::string body, std::string error, unsigned status) {
        BOOST_LOG_TRIVIAL(error)
            << __FUNCTION__ << "Download Failed! STATUS: " << status
            << ", error: " << error << ", body: " << body;
        push_error(ntf_mngr_, _u8L("Download Failed"), _u8L("Contact Us"));
      });
  // http请求完整模型下载链接
  http.perform_sync();
  return result;
}

int32_t ACNextProtocol::get_url_index() const {
  int32_t idx = 0;
  if (owner_->is_test_env()) {
    idx += 1;
  }
  if (!owner_->is_china_env()) {
    idx += 2;
  }
  return idx;
}
const char *ACNextProtocol::get_url() const {
  URL_SWITCH(m_baseUrl_cn, m_baseUrl_cn_test, m_baseUrl_en, m_baseUrl_en_test)
}
