#pragma once
#include <google/protobuf/util/json_util.h>

#include <string>
#include <utility>

#include "absl/status/status.h"
#include "base.h"
#include "common/constants.h"
#include "cppcommon/utils/time.h"
#include "data/local/rocksdb_storage.h"
#include "protos/service/kfpanda/kfpanda.pb.h"

namespace kfpanda {
struct LogContext {
  ::google::protobuf::RpcController *cntl;
  const kfpanda::LogRequest *request;
  kfpanda::LogResponse *response;
};

class LogHandler : public BaseHandler {
 public:
  explicit LogHandler(LogContext &&ctx) : c(std::move(ctx)) {}
  absl::Status Process() override;
  static absl::Status Handle(LogContext &&ctx);

 private:
  LogContext c;
};

inline absl::Status LogHandler::Process() {
  auto r = c.request;
  auto &svc = r->service();
  if (svc.empty()) {
    return kReqErr;
  }

  auto db = LogsRocksDbManager::GetDb(svc, r->log_name());
  if (db == nullptr) {
    return kDbErr;
  }
  auto ts_ms = r->timestamp_ms() <= 0 ? cppcommon::CurrentTs() : r->timestamp_ms();
  std::string pbout;
  // r->SerializeToString(&pbout);
  auto s = google::protobuf::util::MessageToJsonString(*r, &pbout);
  if (!s.ok()) {
    return s;
  }
  auto status = db->Put(rocksdb::WriteOptions(), std::to_string(ts_ms), pbout);
  if (!status.ok()) {
    return absl::ErrnoToStatus(status.code(), status.ToString());
  }
  return absl::OkStatus();
}

inline absl::Status LogHandler::Handle(LogContext &&ctx) {
  LogHandler handler(std::move(ctx));
  return handler.Process();
}
}  // namespace kfpanda
