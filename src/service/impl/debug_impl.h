#pragma once
#include <absl/status/status.h>
#include <absl/strings/string_view.h>
#include <cppcommon/extends/spdlog/log.h>
#include <fmt/format.h>
#include <google/protobuf/util/json_util.h>
#include <spdlog/spdlog.h>

#include <string>

#include "brpc/closure_guard.h"
#include "handler/record_handler.h"
#include "protos/service/kfpanda/kfpanda.pb.h"
#include "service/controllers/restful_cntl.h"

namespace kfpanda {
class KfPandaDebugServiceImpl : public kfpanda::KfPandaDebugService {
  void Echo(::google::protobuf::RpcController* controller, const ::kfpanda::HttpRequest* request,
            ::kfpanda::HttpResponse* response, ::google::protobuf::Closure* done) override;
  void Replay(::google::protobuf::RpcController* controller, const ::kfpanda::HttpRequest* request,
              ::kfpanda::HttpResponse* response, ::google::protobuf::Closure* done) override;
};

inline void KfPandaDebugServiceImpl::Echo(::google::protobuf::RpcController* controller,
                                          const ::kfpanda::HttpRequest* request, ::kfpanda::HttpResponse* response,
                                          ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
  auto pt = fmt::format("/{}/{}", cntl->method()->service()->full_name(), cntl->method()->name());
  spdlog::info("Grpc::Echo. [service_full_name={}, path={}, uri_path={}]", cntl->method()->service()->full_name(), pt,
               cntl->http_request().uri().path());

  // record request
  kfpanda::RecordRequest n_req;
  kfpanda::RecordResponse n_resp;
  n_req.set_service("KungFuPandaServer");
  n_req.set_type(::kfpanda::RECORD_TYPE_GRPC);
  n_req.mutable_uri()->set_path(pt);
  request->SerializeToString(n_req.mutable_data());
  auto status = RecordHandler::Handle(RecordContext{.cntl = cntl, .request = &n_req, .response = &n_resp});
  cntl->response_attachment().append(Response::From(status));
}

inline void KfPandaDebugServiceImpl::Replay(::google::protobuf::RpcController* controller,
                                            const ::kfpanda::HttpRequest* request, ::kfpanda::HttpResponse* response,
                                            ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
  auto pt = fmt::format("/{}/{}", cntl->method()->service()->name(), cntl->method()->name());
  spdlog::info("Grpc::Replay . [path={}]", pt);

  Response rsp;
  auto s = api_replay(cntl, &rsp);
  if (!s.ok()) {
    RERROR("[{}] process faield. [message={}]", __func__, s.message());
  }
  cntl->http_response().SetHeader("Content-Type", "application/json");
  cntl->response_attachment().append(rsp.ToJson(s));
}
}  // namespace kfpanda
