#include "anonymous.hpp"

#include <utility/codec/bin2ascii.hxx>
#include <utility/codec/md5.hxx>

#include <libslic3r/AppConfig.hpp>
#include <slic3r/GUI/GUI_App.hpp>

#include <chrono>
#include <thread>
#include <vector>

#ifdef __WXMAC__
#import <IOKit/IOKitLib.h>
#include <dlfcn.h>
#elif defined(__WXMSW__)
#include <Iphlpapi.h>
#include <netlistmgr.h>
#include <windows.h>
#else

#endif

namespace Slic3r {

class Snowflake {
private:
  static constexpr int64_t kEpoch =
      1577836800000L; // 2020-01-01 00:00:00 UTC in milliseconds
  static constexpr int64_t kWorkerIdBits = 5;
  static constexpr int64_t kDatacenterIdBits = 5;
  static constexpr int64_t kSequenceBits = 12;
  static constexpr int64_t kMaxWorkerId = (1LL << kWorkerIdBits) - 1;
  static constexpr int64_t kMaxDatacenterId = (1LL << kDatacenterIdBits) - 1;
  static constexpr int64_t kMaxSequence = (1LL << kSequenceBits) - 1;
  static constexpr int64_t kWorkerIdShift = kSequenceBits;
  static constexpr int64_t kDatacenterIdShift = kSequenceBits + kWorkerIdBits;
  static constexpr int64_t kTimestampLeftShift =
      kSequenceBits + kWorkerIdBits + kDatacenterIdBits;

  int64_t worker_id_;
  int64_t datacenter_id_;
  int64_t sequence_ = 0;
  int64_t last_timestamp_ = -1;

  int64_t get_current_time_millis() {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    return now.count();
  }

  int64_t wait_next_millis(int64_t last_timestamp) {
    int64_t timestamp = get_current_time_millis();
    while (timestamp <= last_timestamp) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      timestamp = get_current_time_millis();
    }
    return timestamp;
  }

public:
  Snowflake(int64_t worker_id, int64_t datacenter_id)
      : worker_id_(worker_id), datacenter_id_(datacenter_id) {
    if (worker_id > kMaxWorkerId || worker_id < 0) {
      throw std::invalid_argument(
          "worker Id can't be greater than 31 or less than 0");
    }
    if (datacenter_id > kMaxDatacenterId || datacenter_id < 0) {
      throw std::invalid_argument(
          "datacenter Id can't be greater than 31 or less than 0");
    }
  }

  int64_t next_id() {
    int64_t timestamp = get_current_time_millis();

    if (timestamp < last_timestamp_) {
      throw std::runtime_error("Clock moved backwards");
    }

    if (last_timestamp_ == timestamp) {
      sequence_ = (sequence_ + 1) & kMaxSequence;
      if (sequence_ == 0) {
        timestamp = wait_next_millis(last_timestamp_);
      }
    } else {
      sequence_ = 0;
    }

    last_timestamp_ = timestamp;

    return ((timestamp - kEpoch) << kTimestampLeftShift) |
           (datacenter_id_ << kDatacenterIdShift) |
           (worker_id_ << kWorkerIdShift) | sequence_;
  }
};

wxString GetPCID(class AppConfig *app_config) {

  if (app_config == nullptr) {
    app_config = GUI::wxGetApp().app_config;
  }

  auto pcid = app_config->get("pcid");
  if (!pcid.empty()) {
    return pcid;
  }
  auto serialNumber = getSerialNumber();
  if (serialNumber.IsEmpty()) {
    // 使用雪花算法生成ID作为备用方案
    static Snowflake snowflake(1, 1); // 使用默认的worker_id和datacenter_id
    int64_t id = snowflake.next_id();
    serialNumber = wxString::Format("%lld", id);
  }
  auto unique = serialNumber.utf8_string();
  auto tmp =
      GetMD5HexString(reinterpret_cast<char *>(unique.data()), unique.size());
  app_config->set("pcid", tmp.utf8_string());
  return tmp;
}
wxString GetMD5HexString(char *data, size_t length) {
  if (data == nullptr || length == 0)
    return wxEmptyString;
  char buffer[+MD5LEN * 2 + 1] = {0};
  char *pmd5str = buffer + MD5LEN;
  md5Sum(data, length, buffer);
  bin2hex(pmd5str, buffer, MD5LEN);
  return wxASCII_STR(pmd5str);
}
} // namespace Slic3r