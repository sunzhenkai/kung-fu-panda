#pragma once
#include <absl/strings/numbers.h>
#include <absl/strings/string_view.h>

#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "base.h"
#include "common/constants.h"
#include "data/local/rocksdb_storage.h"
#include "protos/service/kfpanda/kfpanda.pb.h"
#include "replay/base_replay.h"

namespace kfpanda {
struct ReplayContext {
  ::google::protobuf::RpcController *cntl{nullptr};
  const kfpanda::ReplayRequest *request{nullptr};
  kfpanda::ReplayResponse *response{nullptr};

  inline int ReplayCount() { return request->option().count(); }
  inline const std::string &ReplayService() { return request->service(); }
};

class ReplayHandler : public BaseHandler {
 public:
  explicit ReplayHandler(ReplayContext &&ctx) : c(std::move(ctx)) {}
  absl::Status Process() override;
  static absl::Status Handle(ReplayContext &&ctx);

 private:
  ReplayContext c;
};

inline absl::StatusOr<std::vector<ReplayRecord>> GetReplayRecords(const std::string &svc, int replay_count) {
  if (svc.empty()) {
    return kReqErr;
  }
  // 1. get db
  auto db = RocksDbManager::GetDb(svc);
  if (db == nullptr) {
    return kDbErr;
  }
  std::vector<ReplayRecord> records;
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
    records.emplace_back(std::move(record));
    if (static_cast<int>(records.size()) >= replay_count) break;
  }
  delete it;
  return records;
}

inline absl::Status ReplayHandler::Process() {
  auto records = GetReplayRecords(c.ReplayService(), c.ReplayCount());
  // 2. traverse request
  ReplayInput input{.request = c.request, .records = std::move(records.value())};
  // 3. replay request
  ReplayOutput output{.response = c.response};
  ReplayOperator::Replay(input, output);
  return absl::OkStatus();
}

inline absl::Status ReplayHandler::Handle(ReplayContext &&ctx) {
  ReplayHandler handler(std::move(ctx));
  return handler.Process();
}

struct ReplayV2Context {
  ::google::protobuf::RpcController *cntl{nullptr};
  const kfpanda::ReplayRequestV2 *request{nullptr};
  kfpanda::ReplayResponseV2 *response{nullptr};

  inline int ReplayCount() { return request->option().count(); }
  inline const std::string &ReplayService() { return request->service(); }
};

class ReplayHandlerV2 : public BaseHandler {
 public:
  explicit ReplayHandlerV2(ReplayV2Context &&ctx) : c(std::move(ctx)) {}
  absl::Status Process() override;
  static absl::Status Handle(ReplayV2Context &&ctx);

 private:
  ReplayV2Context c;
};

inline absl::Status ReplayHandlerV2::Process() {
  auto records = GetReplayRecords(c.ReplayService(), c.ReplayCount());
  // 2. traverse request
  ReplayInputV2 input{.request = c.request, .records = std::move(records.value())};
  // 3. replay request
  ReplayOutputV2 output{.response = c.response};
  ReplayOperator::ReplayV2(input, output);
  return absl::OkStatus();
}
inline absl::Status ReplayHandlerV2::Handle(ReplayV2Context &&ctx) {
  ReplayHandlerV2 handler(std::move(ctx));
  return handler.Process();
}
}  // namespace kfpanda
