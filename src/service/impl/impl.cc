#include "brpc/closure_guard.h"
#include "protos/service/kfpanda/kfpanda.pb.h"

namespace kfpanda {
class KfPandServiceImpl : public kfpanda::KfPandaService {
  void Record(::google::protobuf::RpcController* controller, const ::kfpanda::RecordRequest* request,
              ::kfpanda::RecordResponse* response, ::google::protobuf::Closure* done) override {
    brpc::ClosureGuard dg(done);
  }
};
}  // namespace kfpanda
