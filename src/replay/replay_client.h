
/**
 * @file replay_client.h
 * @brief
 * @author zhenkai.sun
 * @date 2025-05-11 14:21:29
 */
#pragma once
#include <string>

#include "brpc/channel.h"
#include "cppcommon/extends/spdlog/log.h"
#include "fmt/format.h"
#include "protos/service/kfpanda/kfpanda.pb.h"

namespace kfpanda {
using SvcResponse = kfpanda::ServiceResponse;

class ReplayClient {
 public:
  explicit ReplayClient(const kfpanda::URI &target) : target_(target) {}
  inline bool IsOk() { return !has_error_; }

 protected:
  virtual void Init(const std::string &protocol);

 protected:
  brpc::Channel channel_;
  kfpanda::URI target_;
  bool has_error_{false};
};

inline void ReplayClient::Init(const std::string &protocol) {
  brpc::ChannelOptions options;
  options.protocol = protocol;
  options.timeout_ms = 5000;
  options.max_retry = 0;
  auto server = fmt::format("{}:{}", target_.host(), target_.port());
  if (channel_.Init(server.c_str(), "", &options) != 0) {
    RERROR(fmt::format("absl::string_view message. [server={}, protocol={}]", server, protocol));
    has_error_ = true;
  }
}
}  // namespace kfpanda
