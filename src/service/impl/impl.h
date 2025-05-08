#pragma once
#include "brpc/closure_guard.h"
#include "handler/record_hundler.h"
#include "protos/service/kfpanda/kfpanda.pb.h"

namespace kfpanda {
class KfPandaServiceImpl : public kfpanda::KfPandaService {
  void Record(::google::protobuf::RpcController* controller, const ::kfpanda::RecordRequest* request,
              ::kfpanda::RecordResponse* response, ::google::protobuf::Closure* done) override;
};

inline void KfPandaServiceImpl::Record(::google::protobuf::RpcController* controller,
                                       const ::kfpanda::RecordRequest* request, ::kfpanda::RecordResponse* response,
                                       ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  auto status = RecordHandler::Handle(RecordContext{.cntl = controller, .request = request, .response = response});
  response->set_code(static_cast<int>(status.code()));
  response->set_message(status.message());
}
}  // namespace kfpanda
