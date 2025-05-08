#pragma once
#include <absl/strings/numbers.h>
#include <absl/strings/string_view.h>

#include <string>
#include <utility>

#include "absl/status/status.h"
#include "base.h"
#include "common/constants.h"
#include "data/local/rocksdb_storage.h"
#include "protos/service/kfpanda/kfpanda.pb.h"
#include "replay/base_replay.h"

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
  ReplayInput input;
  {
    rocksdb::Iterator *it = db->NewIterator(rocksdb::ReadOptions());
    // traverse by timestamp in desc order
    for (it->SeekToLast(); it->Valid(); it->Prev()) {
      auto k = it->key();
      auto v = it->value();
      ReplayRecord record;
      if (!absl::SimpleAtoi(absl::string_view(k.data(), k.size()), &record.timestamp_ms)) {
        continue;
      }
      record.value = v.ToString();
      input.records.emplace_back(std::move(record));
      if (input.records.size() >= r->option().count()) break;
    }
    delete it;
  }
  // 3. replay request
  ReplayOutput output;
  ReplayOperator::Replay(input, output);
  // 4. convert to response
  // TODO(wii) pack
  return absl::OkStatus();
}

inline absl::Status ReplayHandler::Handle(ReplayContext &&ctx) {
  ReplayHandler handler(std::move(ctx));
  return handler.Process();
}
}  // namespace kfpanda
