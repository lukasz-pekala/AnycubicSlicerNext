#pragma once
#include <utility/utils/query.hxx>

#include <wx/string.h>
#include <wx/uri.h>

#define push_error(mngr, text, htext)

enum DownloadStatus : int32_t {
  DownloadStatus_Complete = 0,     ///< 完成
  DownloadStatus_Canceled = 1,     ///< 用户关闭
  DownloadStatus_Error = -1,       ///< 遇到错误
  DownloadStatus_Timeout = -2,     ///< 超时
  DownloadStatus_NotSupported = 2, ///< 协议不支持
  DownloadStatus_Suggested = 3,    ///< 文件名称被建议
  DownloadStatus_Unknown = std::numeric_limits<int32_t>::min() ///< 未知状态
};
typedef void (*download_callback)(void *ctx, size_t download_id, int32_t status,
                                  wxString &filename);

class Protocol {
public:
  virtual ~Protocol() = default;
  virtual bool can_handle() = 0;
  virtual bool start(size_t id, const wxString &output_path) = 0;
  virtual bool parse_url(const wxString &url);
  wxString get_filename() const;

protected:
  wxURI uri_;
};
