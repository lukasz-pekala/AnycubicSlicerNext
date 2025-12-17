#ifndef slic3r_ACFileUploader_hpp_
#define slic3r_ACFileUploader_hpp_

#include "slic3r/Utils/Http.hpp"

namespace Slic3r { namespace GUI {

struct ACFileUploader_InfoObj
{
    boost::filesystem::path source_path;
    boost::filesystem::path upload_path;
};

class ACFileUploader
{
    std::string                        cred_file;
    std::map<std::string, std::string> cred;

    bool do_api_call(std::function<Http(bool /*is_retry*/)>                                                           build_request,
                     std::function<bool(std::string /* body */, unsigned /* http_status */)>                          on_complete,
                     std::function<bool(std::string /* body */, std::string /* error */, unsigned /* http_status */)> on_error) const;

    bool do_temp_upload(const boost::filesystem::path& file_path,
                        const std::string&             chunk_id,
                        const std::string&             filename,
                        Http::ProgressFn               prorgess_fn,
                        Http::ErrorFn                  error_fn) const;

    bool do_chunk_upload(const boost::filesystem::path& file_path,
                         const std::string&             filename,
                         Http::ProgressFn               prorgess_fn,
                         Http::ErrorFn                  error_fn) const;

public:
    ACFileUploader();
    ~ACFileUploader() = default;
    bool upload(ACFileUploader_InfoObj upload_data, Http::ProgressFn prorgess_fn, Http::ErrorFn error_fn, Http::CompleteFn info_fn) const;

    std::string m_url;
};
}} // namespace Slic3r

#endif
