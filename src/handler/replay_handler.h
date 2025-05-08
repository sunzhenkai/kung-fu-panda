#pragma once
#include <string>
#include <utility>

#include "absl/status/status.h"
#include "base.h"
#include "common/constants.h"
#include "cppcommon/utils/time.h"
#include "data/local/rocksdb_storage.h"
#include "protos/service/kfpanda/kfpanda.pb.h"

namespace kfpanda {
struct ReplayContext {
  ::google::protobuf::RpcController *cntl;
  const kfpanda::ReplayRequest *request;
  kfpanda::ReplayResponse *response;
};

class ReplayHandler : public BaseHandler {
 public:
  explicit ReplayHandler(ReplayContext &&ctx) : c(std::move(ctx)) {}
  absl::Status Process() override;
  static absl::Status Handle(ReplayContext &&ctx);

 private:
  ReplayContext c;
};

inline absl::Status ReplayHandler::Process() {
  auto r = c.request;
  auto &svc = r->service();
  if (svc.empty()) {
    return kReqErr;
  }
  // 1. get db
  auto db = RocksDbManager::GetDb(svc);
  if (db == nullptr) {
    return kDbErr;
  }
  // 2. traverse request
  // 3. replay request
  return absl::OkStatus();
}

inline absl::Status ReplayHandler::Handle(ReplayContext &&ctx) {
  ReplayHandler handler(std::move(ctx));
  return handler.Process();
}
}  // namespace kfpanda
