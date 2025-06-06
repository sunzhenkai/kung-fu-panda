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

struct ReplayInputV2 {
  const kfpanda::ReplayRequestV2 *request;
  std::vector<ReplayRecord> records;
};

struct ReplayOutputV2 {
  kfpanda::ReplayResponseV2 *response{nullptr};
};

class ReplayOperator : public cppcommon::Singleton<ReplayOperator> {
 public:
  static void Replay(const ReplayInput &input, ReplayOutput &output);
  static void ReplayV2(const ReplayInputV2 &input, ReplayOutputV2 &output);
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

inline absl::Status ReplayOnce(std::shared_ptr<ReplayClientBundle> &client, ServiceResponse *rsp,
                               const kfpanda::RecordRequest &req) {
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
}

inline void ReplayOperator::Replay(const ReplayInput &input, ReplayOutput &output) {
  auto base_clients = NewReplayClientBundle(input.request->target());
  for (auto &record : input.records) {
    auto resp = output.response->add_responses();
    kfpanda::RecordRequest req;
    if (!req.ParseFromString(record.value)) {
      RERROR("[{}] parse request failed. [service={}]", __func__, req.service());
    } else {
      auto s = ReplayOnce(base_clients, resp, req);
      if (s.ok()) {
        output.response->set_success_count(output.response->success_count() + 1);
      } else {
        output.response->set_failed_count(output.response->failed_count() + 1);
      }
    }
  }
}

inline void ReplayOperator::ReplayV2(const ReplayInputV2 &input, ReplayOutputV2 &output) {
  auto base_clients = NewReplayClientBundle(input.request->target_base());
  auto compare_clients = NewReplayClientBundle(input.request->target_compare());
  for (auto &record : input.records) {
    auto resp = output.response->add_responses();
    kfpanda::RecordRequest req;
    if (!req.ParseFromString(record.value)) {
      RERROR("[{}] parse request failed. [service={}]", __func__, req.service());
    } else {
      auto s1 = ReplayOnce(base_clients, resp->mutable_base(), req);
      auto s2 = absl::OkStatus();
      if (compare_clients) {
        s2 = ReplayOnce(compare_clients, resp->mutable_compare(), req);
      }
      if (s1.ok() && s2.ok()) {
        output.response->set_success_count(output.response->success_count() + 1);
      } else {
        output.response->set_failed_count(output.response->failed_count() + 1);
      }
    }
  }
}
}  // namespace kfpanda
