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

#include <cstdint>
#include <string>
#include <vector>

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
  kfpanda::ReplayResponse *response;
};

class ReplayOperator : public cppcommon::Singleton<ReplayOperator> {
 public:
  static void Replay(const ReplayInput &input, ReplayOutput &output);
};

inline void ReplayOperator::Replay(const ReplayInput &input, ReplayOutput &output) {
  auto r = output.response;
  auto http_replay_client = HttpReplayClient(input.request->target());
  for (auto &record : input.records) {
    kfpanda::RecordRequest req;
    if (!req.ParseFromString(record.value)) {
      RERROR("[{}] parse request failed. [service={}]", __func__, req.service());
      r->set_failed_count(r->failed_count() + 1);
      continue;
    }
    absl::Status status;
    if (req.type() == kfpanda::RECORD_TYPE_HTTP) {
      status = http_replay_client.Replay(&req);
    } else {
      r->set_failed_count(r->failed_count() + 1);
      RERROR("[{}] unknown protocol. [protocol={}]", __func__, kfpanda::RecordType_Name(req.type()));
      continue;
    }

    if (!status.ok()) {
      r->set_failed_count(r->failed_count() + 1);
    }
  }
}
}  // namespace kfpanda
