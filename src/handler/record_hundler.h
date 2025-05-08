#pragma once
#include <utility>

#include "absl/status/status.h"
#include "base.h"
#include "common/constants.h"
#include "cppcommon/utils/time.h"
#include "data/local/rocksdb_storage.h"
#include "protos/service/kfpanda/kfpanda.pb.h"

namespace kfpanda {
struct RecordContext {
  ::google::protobuf::RpcController *cntl;
  const kfpanda::RecordRequest *request;
  kfpanda::RecordResponse *response;
};

class RecordHandler : public BaseHandler {
 public:
  explicit RecordHandler(RecordContext &&ctx) : c(std::move(ctx)) {}
  absl::Status Process() override;
  static absl::Status Handle(RecordContext &&ctx);

 private:
  RecordContext c;
};

inline absl::Status RecordHandler::Process() {
  auto r = c.request;
  auto &svc = r->service();
  auto db = RocksDbManager::GetDb(svc);
  if (db == nullptr) {
    return kDbErr;
  }
  auto ts_ms = r->timestamp_ms() <= 0 ? cppcommon::CurrentTs() : r->timestamp_ms();
  return absl::OkStatus();
}

inline absl::Status RecordHandler::Handle(RecordContext &&ctx) {
  RecordHandler handler(std::move(ctx));
  return handler.Process();
}
}  // namespace kfpanda
