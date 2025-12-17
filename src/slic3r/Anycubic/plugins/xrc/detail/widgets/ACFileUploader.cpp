#include "ACFileUploader.hpp"

#include <openssl/sha.h>
#include <boost/beast/core/detail/base64.hpp>
#include <boost/nowide/fstream.hpp>
#include <boost/filesystem.hpp>

#include "nlohmann/json.hpp"
#include "libslic3r/Utils.hpp"
#include "slic3r/GUI/I18N.hpp"
#include "slic3r/GUI/format.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/MainFrame.hpp"

namespace Slic3r {


static constexpr uint64_t MAX_SINGLE_UPLOAD_FILE_SIZE = 100000ull;


static void set_auth(Http& http, const std::string& access_token) { 
    http.header("Content-Type", "multipart/form-data"); 
    http.header("X-File-Length", "");
}

ACFileUploader::ACFileUploader()
{
    
}

bool ACFileUploader::do_api_call(std::function<Http(bool)>                               build_request,
                              std::function<bool(std::string, unsigned)>              on_complete,
                              std::function<bool(std::string, std::string, unsigned)> on_error) const
{
    
    bool res = true;

    const auto create_request = [this, &build_request, &res, on_complete](bool is_retry) {
        auto http = build_request(is_retry);
        set_auth(http, "");

        

        return http;
    };

    create_request(false)
        .on_error([&res, on_error](std::string body, std::string error, unsigned http_status) {
            res = on_error(body, error, http_status);
            
        })
        .on_complete([&res, on_complete](std::string body, unsigned http_status) {
            res = on_complete(body, http_status);
           })
        .perform();

    return res;
}


bool ACFileUploader::do_temp_upload(const boost::filesystem::path& file_path,
                                 const std::string&             chunk_id,
                                 const std::string&             filename,
                                    Http::ProgressFn               prorgess_fn,
                                    Http::ErrorFn                  error_fn) const
{
    

    return do_api_call(
        [this,&file_path, prorgess_fn, &filename](bool is_retry) {
            auto http = Http::post(m_url);
            http.form_add("filename", filename);
            http.form_add_file("gcode", file_path);
            
            http.on_progress([prorgess_fn](Http::Progress progress, bool& cancel) { 
                prorgess_fn(progress, cancel);
                });

            return http;
        },
        [error_fn, &filename, this](std::string body, unsigned status) {
            BOOST_LOG_TRIVIAL(info) << boost::format("ACFileUploader: File uploaded: HTTP %1%: %2%") % status % body;

            // Get file UUID
            const auto j = nlohmann::json::parse(body, nullptr, false, true);
            if (j.is_discarded()) {
                BOOST_LOG_TRIVIAL(error) << "ACFileUploader: Invalid or no JSON data on token response: " << body;
                error_fn(body, "", status);
                return false;
            }

            if (j.find("uuid") == j.end()) {
                BOOST_LOG_TRIVIAL(error) << "ACFileUploader: Invalid or no JSON data on token response: " << body;
                error_fn(body, "", status);
                return false;
            }
            const std::string uuid = j["uuid"];

           

            return true;
        },
        [this, error_fn](std::string body, std::string error, unsigned status) {
            BOOST_LOG_TRIVIAL(error) << boost::format("ACFileUploader: Error uploading file : %1%, HTTP %2%, body: `%3%`") % error % status %
                                            body;
            error_fn(body, error, status);
            return false;
        });
}

bool ACFileUploader::do_chunk_upload(const boost::filesystem::path& file_path,
                                  const std::string&             filename,
                                     Http::ProgressFn                 prorgess_fn,
                                     Http::ErrorFn                  error_fn) const
{
    const auto file_size = boost::filesystem::file_size(file_path);

    constexpr auto buffer_size = 10240*8;


    const auto chunk_amount = (size_t) ceil((double) file_size / buffer_size);


    const auto clean_up = [this]() {
        

        do_api_call(
            [this](bool is_retry) {
                auto http = Http::get(m_url);
                return http;
            },
            [](std::string body, unsigned status) {
                return true;
            },
            [](std::string body, std::string error, unsigned status) {
                return false;
            });
    };

    for (size_t i = 0; i < chunk_amount; i++) {
        

        const boost::filesystem::ifstream::off_type offset = i * buffer_size;
        const size_t                                length = i == (chunk_amount - 1) ? file_size - offset : buffer_size;

        const bool succ = do_api_call(
            [this,&file_path, &filename, i, chunk_amount, file_size, offset, length, prorgess_fn](bool is_retry) {
                BOOST_LOG_TRIVIAL(info) << boost::format("ACFileUploader: Start uploading file chunk [%1%/%2%]...") % (i + 1) % chunk_amount;
                auto http = Http::post(m_url);
                http.form_add("filename", filename);
                http.form_add_file("file", file_path, filename, offset, length);

                http.on_progress([prorgess_fn](Http::Progress progress, bool& cancel) {

                    prorgess_fn(progress,cancel);
                });

                return http;
            },
            [this](std::string body, unsigned status) {
                return true;
            },
            [this, error_fn](std::string body, std::string error, unsigned status) {
                error_fn(body, error, status);
                return false;
            });

        if (!succ) {
            clean_up();
            return false;
        }
    }


    return true;
}

bool ACFileUploader::upload(ACFileUploader_InfoObj upload_data,
                            Http::ProgressFn       prorgess_fn,
                            Http::ErrorFn          error_fn,
                            Http::CompleteFn       info_fn) const
{
    
    const auto filename = upload_data.upload_path.filename().string();
    if (boost::filesystem::file_size(upload_data.source_path) > MAX_SINGLE_UPLOAD_FILE_SIZE) {
        return do_chunk_upload(upload_data.source_path, filename, prorgess_fn, error_fn);
    } else {
        
        return do_temp_upload(upload_data.source_path, {}, filename, prorgess_fn, error_fn);
    }
}

} // namespace Slic3r
