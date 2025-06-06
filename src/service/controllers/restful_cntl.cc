#include "restful_cntl.h"

#include <absl/status/status.h>
#include <butil/base64.h>
#include <fmt/format.h>
#include <protos/service/kfpanda/kfpanda.pb.h>
#include <spdlog/spdlog.h>

#include <string>
#include <unordered_map>

#include "absl/strings/numbers.h"
#include "cppcommon/extends/rapidjson/builder.h"
#include "google/protobuf/util/json_util.h"
#include "handler/log_handler.h"
#include "handler/record_handler.h"
#include "handler/replay_handler.h"
#include "rapidjson/document.h"

namespace kfpanda {
const std::unordered_map<std::string, ViewFunc> kRestfulApiViews = {
    {"/api/echo", api_echo},
    {"/api/grpc/echo", api_grpc_echo},
    {"/api/replay", api_replay},
    {"/api/replayv2", api_replay_v2},
    {"/api/debug/stat", api_debug_stat},
    {"/api/debug/sample", api_debug_sample},
    {"/api/log/record", api_log_record},
    {"/api/log/sample", api_log_sample},
    {"/api/log/stat", api_log_stat},
};

absl::Status api_echo(brpc::Controller *cntl, Response *rsp) {
  cntl->response_attachment().append(cntl->request_attachment());

  // record request
  kfpanda::RecordRequest n_req;
  kfpanda::RecordResponse n_resp;
  n_req.set_service("KungFuPandaServer");
  n_req.set_type(::kfpanda::RECORD_TYPE_HTTP);
  n_req.mutable_uri()->set_path(cntl->http_request().uri().path());
  n_req.set_data(cntl->request_attachment().to_string());
  return RecordHandler::Handle(RecordContext{.cntl = cntl, .request = &n_req, .response = &n_resp});
}

absl::Status api_grpc_echo(brpc::Controller *cntl, Response *rsp) {
  auto method = kfpanda::KfPandaDebugService::descriptor()->FindMethodByName("Echo");
  auto pt = fmt::format("/{}/{}", method->service()->full_name(), method->name());

  kfpanda::RecordRequest n_req;
  kfpanda::RecordResponse n_resp;
  n_req.set_service("KungFuPandaServer");
  n_req.set_type(::kfpanda::RECORD_TYPE_GRPC);
  n_req.mutable_uri()->set_path(pt);
  if (cntl->method()->name() == "Api") {
    cntl->response_attachment().append(cntl->request_attachment());
    kfpanda::EchoMessage req;
    auto s = google::protobuf::util::JsonStringToMessage(cntl->request_attachment().to_string(), &req);
    if (!s.ok()) {
      return s;
    }
    req.SerializeToString(n_req.mutable_data());
  } else {
    n_req.set_data(cntl->request_attachment().to_string());
  }
  return RecordHandler::Handle(RecordContext{.cntl = cntl, .request = &n_req, .response = &n_resp});
}

absl::Status api_replay(brpc::Controller *cntl, Response *rrsp) {
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
    status = ReplayHandler::Handle(ReplayContext{.cntl = cntl, .request = &req, .response = &rsp});
  }
  if (status.ok()) {
    std::string js;
    auto j = google::protobuf::util::MessageToJsonString(rsp, &js);
    if (rrsp != nullptr) {
      rrsp->AddJsonStr("data", js);
    }
    return absl::OkStatus();
  } else {
    return status;
  }
}

absl::Status api_replay_v2(brpc::Controller *cntl, Response *rrsp) {
  kfpanda::ReplayResponseV2 rsp;
  kfpanda::ReplayRequestV2 req;

  absl::Status status = absl::OkStatus();
  if (cntl->request_attachment().empty()) {
    req.set_service("KungFuPandaServer");
    req.mutable_target_base()->set_host("127.0.0.1");
    req.mutable_target_base()->set_port(FLAGS_port);
    req.mutable_target_compare()->set_host("127.0.0.1");
    req.mutable_target_compare()->set_port(FLAGS_port);
    req.mutable_option()->set_count(1);
  } else {
    status = google::protobuf::util::JsonStringToMessage(cntl->request_attachment().to_string(), &req);
  }
  if (status.ok()) {
    status = ReplayHandlerV2::Handle(ReplayV2Context{.cntl = cntl, .request = &req, .response = &rsp});
  }
  if (status.ok()) {
    std::string js;
    auto j = google::protobuf::util::MessageToJsonString(rsp, &js);
    if (rrsp != nullptr) {
      rrsp->AddJsonStr("data", js);
    }
    return absl::OkStatus();
  } else {
    return status;
  }
}

absl::Status api_debug_stat(brpc::Controller *cntl, Response *rsp) {
  auto dbstat = RocksDbManager::GetDbState();
  if (rsp != nullptr) {
    rsp->Add("data", dbstat);
  }
  return absl::OkStatus();
}

absl::Status api_debug_sample(brpc::Controller *cntl, Response *rsp) {
  auto r = cntl->request_attachment().to_string();
  rapidjson::Document doc;
  doc.Parse(r.c_str(), r.size());
  if (doc.HasParseError()) {
    return absl::ErrnoToStatus(400, fmt::format("parse request body failed. [message={}]", r));
  } else {
    auto it_service = doc.FindMember("service");
    auto it_count = doc.FindMember("count");
    if (it_service == doc.MemberEnd()) {
      return absl::ErrnoToStatus(400, "service is not specified");
    } else {
      auto count = it_count == doc.MemberEnd() ? 1 : it_count->value.GetInt();
      auto items = RocksDbManager::TryGetIterms(it_service->value.GetString(), count);

      cppcommon::JsonBuilder jb;
      for (auto &[k, v] : items) {
        kfpanda::RecordRequest rr;
        if (rr.ParseFromString(v)) {
          std::string njs;
          auto s = google::protobuf::util::MessageToJsonString(rr, &njs);
          if (s.ok()) {
            jb.AddJsonStr(k, njs);
          }
        } else {
          spdlog::info("[{}] parse record request failed", __func__);
        }
      }
      if (rsp != nullptr) {
        rsp->Add("data", jb);
      }
      return absl::OkStatus();
    }
  }
}

absl::Status api_log_sample(brpc::Controller *cntl, Response *rsp) {
  auto service = cntl->http_request().uri().GetQuery("service");
  auto log_name = cntl->http_request().uri().GetQuery("log_name");
  auto count_str = cntl->http_request().uri().GetQuery("count");
  int count = 1;
  if (!count_str->empty()) {
    auto s = absl::SimpleAtoi(*count_str, &count);
    if (!s) {
      return absl::ErrnoToStatus(400, "count field should be number");
    }
  }
  auto items = LogsRocksDbManager::TryGetIterms(*service, *log_name, count);
  cppcommon::JsonBuilder jb;
  for (auto &item : items) {
    jb.AddJsonStr(item.first, item.second);
  }
  rsp->Add("count", items.size());
  rsp->Add("data", jb);
  return absl::OkStatus();
}

absl::Status api_log_record(brpc::Controller *cntl, Response *rsp) {
  kfpanda::LogRequest req;
  auto s = google::protobuf::util::JsonStringToMessage(cntl->request_attachment().to_string(), &req);
  if (!s.ok()) return s;
  kfpanda::LogResponse trsp;
  return LogHandler::Handle(LogContext{.cntl = cntl, .request = &req, .response = &trsp});
}

absl::Status api_log_stat(brpc::Controller *cntl, Response *rsp) {
  auto dbstat = LogsRocksDbManager::GetDbState();
  if (rsp != nullptr) {
    rsp->Add("data", dbstat);
  }
  return absl::OkStatus();
}
}  // namespace kfpanda
