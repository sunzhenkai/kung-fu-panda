/**
 * @file base_replay.h
 * @brief
 * @author zhenkai.sun
 * @date 2025-05-08 15:06:59
 */
#pragma once

#include <absl/status/status.h>
#include <cppcommon/extends/spdlog/log.h>
#include <cppcommon/partterns/singleton.h>
#include <protos/service/kfpanda/kfpanda.pb.h>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "common/constants.h"
#include "grpc_replay.h"
#include "http_replay.h"

namespace kfpanda {
struct ReplayRecord {
  int64_t timestamp_ms = 0;
  std::string value;
};

struct ReplayInput {
  const kfpanda::ReplayRequest *request;
  std::vector<ReplayRecord> records;
};

struct ReplayOutput {
  kfpanda::ReplayResponse *response{nullptr};
};

class ReplayOperator : public cppcommon::Singleton<ReplayOperator> {
 public:
  static void Replay(const ReplayInput &input, ReplayOutput &output);
};

struct ReplayClientBundle {
  HttpReplayClient http;
  GrpcReplayClient grpc;
};

inline std::shared_ptr<ReplayClientBundle> NewReplayClientBundle(const URI &target) {
  if (target.host().empty()) return std::shared_ptr<ReplayClientBundle>(nullptr);
  auto clients = new ReplayClientBundle{HttpReplayClient(target), GrpcReplayClient(target)};
  return std::shared_ptr<ReplayClientBundle>(clients);
}

inline void ReplayOperator::Replay(const ReplayInput &input, ReplayOutput &output) {
  auto base_clients = NewReplayClientBundle(input.request->target());
  auto compare_clients = NewReplayClientBundle(input.request->target_compare());
  auto replay = [](std::shared_ptr<ReplayClientBundle> &client, ReplayResponse::ServiceResponse *rsp,
                   const kfpanda::RecordRequest &req) -> absl::Status {
    absl::Status s = absl::OkStatus();
    if (client == nullptr) return s;
    rsp->set_type_str(kfpanda::RecordType_Name(req.type()));
    if (req.type() == kfpanda::RECORD_TYPE_HTTP) {
      s = client->http.Replay(&req, rsp);
    } else if (req.type() == kfpanda::RECORD_TYPE_GRPC) {
      s = client->grpc.Replay(&req, rsp);
    } else {
      RERROR("[{}] unknown protocol. [protocol={}]", __func__, kfpanda::RecordType_Name(req.type()));
      s = kReqErr;
    }
    if (!s.ok()) {
      rsp->set_message(s.ToString());
    }
    return s;
  };
  for (auto &record : input.records) {
    auto resp = output.response->add_results();
    kfpanda::RecordRequest req;
    if (!req.ParseFromString(record.value)) {
      RERROR("[{}] parse request failed. [service={}]", __func__, req.service());
    } else {
      auto s1 = replay(base_clients, resp->mutable_base(), req);
      auto s2 = replay(compare_clients, resp->mutable_compare(), req);
      if (s1.ok() && s2.ok()) {
        output.response->set_success_count(output.response->success_count() + 1);
      } else {
        output.response->set_failed_count(output.response->failed_count() + 1);
      }
    }
  }
}
}  // namespace kfpanda
