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
#include <string>
#include <utility>
#include <vector>

#include "common/constants.h"
#include "replay/http_replay.h"

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
  inline void AddResult(const absl::Status &status, kfpanda::ReplayResponse::ServiceResponse *result = nullptr) {
    if (response == nullptr) return;
    auto rsp = response->add_responses();
    if (status.ok()) {
      response->set_success_count(response->success_count() + 1);
      if (result != nullptr) {
        rsp->set_body(result->body());
        rsp->set_message(result->message());
      }
    } else {
      response->set_failed_count(response->failed_count() + 1);
      rsp->set_message(status.message());
    }
  }
};

class ReplayOperator : public cppcommon::Singleton<ReplayOperator> {
 public:
  static void Replay(const ReplayInput &input, ReplayOutput &output);
};

inline void ReplayOperator::Replay(const ReplayInput &input, ReplayOutput &output) {
  auto http_replay_client = HttpReplayClient(input.request->target());
  for (auto &record : input.records) {
    kfpanda::RecordRequest req;
    if (!req.ParseFromString(record.value)) {
      RERROR("[{}] parse request failed. [service={}]", __func__, req.service());
      output.AddResult(kReqErr);
    } else {
      absl::Status status;
      if (req.type() == kfpanda::RECORD_TYPE_HTTP) {
        kfpanda::ReplayResponse::ServiceResponse result;
        auto s = http_replay_client.Replay(&req, &result);

        output.AddResult(s, &result);
      } else {
        output.AddResult(kReqErr);
        RERROR("[{}] unknown protocol. [protocol={}]", __func__, kfpanda::RecordType_Name(req.type()));
      }
    }
  }
}
}  // namespace kfpanda
