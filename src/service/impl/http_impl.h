#pragma once
#include <cppcommon/extends/spdlog/log.h>
#include <protos/dumper/http.pb.h>

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

  // record request
  kfpanda::RecordRequest n_req;
  kfpanda::RecordResponse n_resp;
  n_req.set_service("KungFuPandaServer");
  n_req.set_type(::kfpanda::RECORD_TYPE_HTTP);
  dumper::HttpRequest hr;
  hr.mutable_uri()->set_path("/api/echo");
  hr.set_body(cntl->request_attachment().to_string());
  hr.SerializeToString(n_req.mutable_data());
  auto status = RecordHandler::Handle(RecordContext{.cntl = controller, .request = &n_req, .response = &n_resp});
  if (!status.ok()) {
    RERROR("[{}] record faield", __func__);
  }
}
inline void HttpKfPandaServiceImpl ::Replay(::google::protobuf::RpcController* controller,
                                            const ::kfpanda::HttpRequest* request, ::kfpanda::HttpResponse* response,
                                            ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  // TODO: fill logic code
}
}  // namespace kfpanda
