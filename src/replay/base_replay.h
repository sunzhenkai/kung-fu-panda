/**
 * @file base_replay.h
 * @brief
 * @author zhenkai.sun
 * @date 2025-05-08 15:06:59
 */
#pragma once

#include <cppcommon/partterns/singleton.h>
#include <protos/service/kfpanda/kfpanda.pb.h>

#include <cstdint>
#include <string>
#include <vector>

namespace kfpanda {
struct ReplayRecord {
  int64_t timestamp_ms = 0;
  std::string value;
};

struct ReplayInput {
  const kfpanda::ReplayRequest *request;
  std::vector<ReplayRecord> records;
};

struct ReplayOutput {};

class ReplayOperator : public cppcommon::Singleton<ReplayOperator> {
 public:
  static ReplayOutput Replay(const ReplayInput &input);
};

inline ReplayOutput ReplayOperator::Replay(const ReplayInput &input) {
  ReplayOutput ret{};
  // TODO: replay
  return ret;
}
}  // namespace kfpanda
