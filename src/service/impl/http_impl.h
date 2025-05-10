#pragma once
#include <absl/status/status.h>
#include <absl/strings/string_view.h>
#include <cppcommon/extends/spdlog/log.h>
#include <fmt/format.h>
#include <google/protobuf/util/json_util.h>
#include <spdlog/spdlog.h>

#include <string>

#include "brpc/closure_guard.h"
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
  auto s = api_echo(cntl, nullptr);
  if (!s.ok()) {
    RERROR("[{}] process faield. [message={}]", __func__, s.message());
  }
}

inline void HttpKfPandaServiceImpl::Replay(::google::protobuf::RpcController* controller,
                                           const ::kfpanda::HttpRequest* request, ::kfpanda::HttpResponse* response,
                                           ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
  Response rsp;
  auto s = api_replay(cntl, &rsp);
  if (!s.ok()) {
    RERROR("[{}] process faield. [message={}]", __func__, s.message());
  }
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
  if (it != kRestfulApiViews.end()) {
    Response res;
    auto status = it->second(cntl, &res);
    cntl->response_attachment().append(res.ToJson(status));
  } else {
    cntl->response_attachment().append(Response::From(absl::ErrnoToStatus(410, "no such api")));
  }
}
}  // namespace kfpanda
