
/**
 * @file client.h
 * @brief
 * @author zhenkai.sun
 * @date 2025-05-08 16:23:17
 */
#pragma once
#include <memory>
#include <string>

#include "absl/status/status.h"
#include "brpc/channel.h"
#include "protos/service/kfpanda/kfpanda.pb.h"

namespace kfpanda {
class KfpandaClient {
 public:
  explicit KfpandaClient(const std::string &server) : server_(server) {}
  absl::Status Init();
  kfpanda::KfPandaService_Stub *Stub() const;

 private:
  std::string server_;
  brpc::Channel channel_;
  std::shared_ptr<kfpanda::KfPandaService_Stub> stub_;
};

inline absl::Status KfpandaClient::Init() {
  // Initialize the channel, NULL means using default options.
  brpc::ChannelOptions options;
  options.protocol = "baidu_std";
  options.connection_type = "single";
  options.timeout_ms = 1000 /*milliseconds*/;
  options.max_retry = 0;
  if (channel_.Init(server_.c_str(), "rr", &options) != 0) {
    return absl::ErrnoToStatus(400, "absl::string_view message");
  }
  stub_ = std::make_shared<kfpanda::KfPandaService_Stub>(&channel_);
  return absl::OkStatus();
}

inline kfpanda::KfPandaService_Stub *KfpandaClient::Stub() const { return stub_.get(); }
}  // namespace kfpanda
