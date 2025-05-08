#pragma once
#include "absl/status/status.h"
#include "base.h"
#include "protos/service/kfpanda/kfpanda.pb.h"

namespace kfpanda {
struct RecordContext {
  ::google::protobuf::RpcController *cntl;
  const kfpanda::RecordRequest *request;
  kfpanda::RecordResponse *response;
};

class RecordHandler : public BaseHandler {
 public:
  absl::Status Process() override;
  static absl::Status Handle(RecordContext &&ctx);
};

inline absl::Status RecordHandler::Process() { return absl::OkStatus(); }

inline absl::Status RecordHandler::Handle(RecordContext &&ctx) {
  RecordHandler handler;
  return handler.Process();
}
}  // namespace kfpanda
