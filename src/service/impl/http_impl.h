#pragma once
#include <absl/status/status.h>
#include <absl/strings/string_view.h>
#include <cppcommon/extends/spdlog/log.h>
#include <fmt/format.h>
#include <google/protobuf/util/json_util.h>
#include <spdlog/spdlog.h>

#include <string>

#include "brpc/closure_guard.h"
#include "common/flags.h"
#include "cppcommon/extends/rapidjson/builder.h"
#include "cppcommon/utils/to_str.h"
#include "data/local/rocksdb_storage.h"
#include "handler/record_handler.h"
#include "handler/replay_handler.h"
#include "protos/service/kfpanda/kfpanda.pb.h"
#include "rapidjson/document.h"

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
  cntl->response_attachment().append(cntl->request_attachment());
  // spdlog::info("[{}] echo. [message={}]", __func__, cntl->request_attachment().to_string());

  // record request
  kfpanda::RecordRequest n_req;
  kfpanda::RecordResponse n_resp;
  n_req.set_service("KungFuPandaServer");
  n_req.set_type(::kfpanda::RECORD_TYPE_HTTP);
  n_req.mutable_uri()->set_path("/api/echo");
  n_req.set_data(cntl->request_attachment().to_string());
  auto status = RecordHandler::Handle(RecordContext{.cntl = controller, .request = &n_req, .response = &n_resp});
  if (!status.ok()) {
    RERROR("[{}] record faield", __func__);
  }
}

inline void HttpKfPandaServiceImpl::Replay(::google::protobuf::RpcController* controller,
                                           const ::kfpanda::HttpRequest* request, ::kfpanda::HttpResponse* response,
                                           ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
  kfpanda::ReplayResponse rsp;
  kfpanda::ReplayRequest req;

  absl::Status status = absl::OkStatus();
  if (cntl->request_attachment().empty()) {
    req.set_service("KungFuPandaServer");
    req.mutable_target()->set_host("127.0.0.1");
    req.mutable_target()->set_port(FLAGS_port);
    req.mutable_option()->set_count(1);
  } else {
    status = google::protobuf::util::JsonStringToMessage(cntl->request_attachment().to_string(), &req);
  }
  if (status.ok()) {
    status = ReplayHandler::Handle(ReplayContext{.cntl = controller, .request = &req, .response = &rsp});
  }
  if (status.ok()) {
    std::string js;
    auto j = google::protobuf::util::MessageToJsonString(rsp, &js);
    cntl->response_attachment().append(js);
  } else {
    auto r = fmt::format("code={}, message={}", static_cast<int>(status.code()), status.message());
    cntl->response_attachment().append(r);
  }
}

inline void HttpKfPandaServiceImpl::Api(::google::protobuf::RpcController* controller,
                                        const ::kfpanda::HttpRequest* request, ::kfpanda::HttpResponse* response,
                                        ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
  auto pt = cntl->http_request().uri().path();
  if (pt == "/debug/stat") {
    auto dbstat = RocksDbManager::GetDbState();
    cntl->response_attachment().append(cppcommon::ToString(dbstat));
  } else if (pt == "/debug/sample") {
    auto r = cntl->request_attachment().to_string();
    rapidjson::Document doc;
    doc.Parse(r.c_str(), r.size());
    if (doc.HasParseError()) {
      cntl->response_attachment().append("parse request body failed (json)");
    } else {
      auto it_service = doc.FindMember("service");
      auto it_count = doc.FindMember("count");
      if (it_service == doc.MemberEnd()) {
        cntl->response_attachment().append("service is not specified");
      } else {
        auto count = it_count == doc.MemberEnd() ? 1 : it_count->value.GetInt();
        auto items = RocksDbManager::TryGetIterms(it_service->value.GetString(), count);

        cppcommon::JsonBuilder jb;
        for (auto& [k, v] : items) {
          kfpanda::RecordRequest rr;
          if (rr.ParseFromString(v)) {
            if (rr.type() == kfpanda::RECORD_TYPE_HTTP) {
              std::string njs;
              auto s = google::protobuf::util::MessageToJsonString(rr, &njs);
              if (s.ok()) {
                jb.AddJsonStr(k, njs);
              }
            } else {
              spdlog::info("[{}] unsupported protocol type", __func__);
            }
          } else {
            spdlog::info("[{}] parse record request failed", __func__);
          }
        }
        cntl->response_attachment().append(jb.Build());
      }
    }
  } else {
    cntl->response_attachment().append("not impl");
  }
}
}  // namespace kfpanda
