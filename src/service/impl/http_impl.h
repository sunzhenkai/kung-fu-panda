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
class HttpKfPandaServiceImpl : public kfpanda::HttpKfPandaService {
  void Echo(::google::protobuf::RpcController* controller, const ::kfpanda::HttpRequest* request,
            ::kfpanda::HttpResponse* response, ::google::protobuf::Closure* done) override;
  void Replay(::google::protobuf::RpcController* controller, const ::kfpanda::HttpRequest* request,
              ::kfpanda::HttpResponse* response, ::google::protobuf::Closure* done) override;
  void Api(::google::protobuf::RpcController* controller, const ::kfpanda::HttpRequest* request,
           ::kfpanda::HttpResponse* response, ::google::protobuf::Closure* done) override;
};

inline void HttpKfPandaServiceImpl::Echo(::google::protobuf::RpcController* controller,
                                         const ::kfpanda::HttpRequest* request, ::kfpanda::HttpResponse* response,
                                         ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
  auto pt = fmt::format("/{}/{}", cntl->method()->name(), cntl->method()->service()->name());
  spdlog::info("Grpc::Echo. [path={}]", pt);

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

inline void HttpKfPandaServiceImpl::Replay(::google::protobuf::RpcController* controller,
                                           const ::kfpanda::HttpRequest* request, ::kfpanda::HttpResponse* response,
                                           ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
  auto pt = fmt::format("/{}/{}", cntl->method()->name(), cntl->method()->service()->name());
  spdlog::info("Grpc::Replay . [path={}]", pt);

  Response rsp;
  auto s = api_replay(cntl, &rsp);
  if (!s.ok()) {
    RERROR("[{}] process faield. [message={}]", __func__, s.message());
  }
  cntl->http_response().SetHeader("Content-Type", "application/json");
  cntl->response_attachment().append(rsp.ToJson(s));
}

inline void HttpKfPandaServiceImpl::Api(::google::protobuf::RpcController* controller,
                                        const ::kfpanda::HttpRequest* request, ::kfpanda::HttpResponse* response,
                                        ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
  auto pt = cntl->http_request().uri().path();

  auto it = kRestfulApiViews.find(pt);
  auto status = absl::OkStatus();
  cntl->http_response().SetHeader("Content-Type", "application/json");
  if (it != kRestfulApiViews.end()) {
    Response res;
    auto status = it->second(cntl, &res);
    if (pt != "/api/echo") {
      cntl->response_attachment().append(res.ToJson(status));
    }
  } else {
    cntl->response_attachment().append(Response::From(absl::ErrnoToStatus(410, "no such api")));
  }
}
}  // namespace kfpanda
