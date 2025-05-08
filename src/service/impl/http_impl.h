#pragma once
#include "brpc/closure_guard.h"
#include "handler/record_handler.h"
#include "handler/replay_handler.h"
#include "protos/service/kfpanda/kfpanda.pb.h"

namespace kfpanda {
class HttpKfPandaServiceImpl : public kfpanda::HttpKfPandaService {
  void Echo(::google::protobuf::RpcController* controller, const ::kfpanda::HttpRequest* request,
            ::kfpanda::HttpResponse* response, ::google::protobuf::Closure* done) override;
  void Replay(::google::protobuf::RpcController* controller, const ::kfpanda::HttpRequest* request,
              ::kfpanda::HttpResponse* response, ::google::protobuf::Closure* done) override;
};

inline void HttpKfPandaServiceImpl ::Echo(::google::protobuf::RpcController* controller,
                                          const ::kfpanda::HttpRequest* request, ::kfpanda::HttpResponse* response,
                                          ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
  cntl->response_attachment().append(cntl->request_attachment());
}
inline void HttpKfPandaServiceImpl ::Replay(::google::protobuf::RpcController* controller,
                                            const ::kfpanda::HttpRequest* request, ::kfpanda::HttpResponse* response,
                                            ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  // TODO: fill logic code
}
}  // namespace kfpanda
