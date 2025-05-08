/**
 * @file http_replay.h
 * @brief
 * @author zhenkai.sun
 * @date 2025-05-08 15:51:10
 */
#pragma once

#include <brpc/controller.h>
#include <brpc/http_method.h>
#include <fmt/format.h>
#include <protos/dumper/http.pb.h>
#include <protos/service/kfpanda/kfpanda.pb.h>

#include "absl/status/status.h"
#include "brpc/channel.h"
#include "cppcommon/extends/spdlog/log.h"

namespace kfpanda {
class HttpReplayClient {
 public:
  explicit HttpReplayClient(const kfpanda::ReplayRequest::Target &target) : target_(target) { Init(); }
  inline bool IsOk() { return !has_error_; }
  absl::Status Replay(const kfpanda::RecordRequest *req);

 private:
  void Init();

 private:
  brpc::Channel channel_;
  kfpanda::ReplayRequest::Target target_;
  bool has_error_{false};
};

inline void HttpReplayClient::Init() {
  brpc::ChannelOptions options;
  options.protocol = "http";
  options.connection_type = "single";
  options.timeout_ms = 1000 /*milliseconds*/;
  options.max_retry = 0;
  auto server = fmt::format("{}:{}", target_.ip(), target_.port());
  if (channel_.Init(server.c_str(), "", &options) != 0) {
    RERROR("absl::string_view message");
    has_error_ = true;
  }
}

inline absl::Status HttpReplayClient::Replay(const kfpanda::RecordRequest *req) {
  if (has_error_) {
    auto msg = fmt::format("http replay client init failed. [ip={}, port={}]", target_.ip(), target_.port());
    return absl::ErrnoToStatus(501, msg);
  }
  auto rid = req->request_id();
  dumper::HttpRequest hr;
  hr.ParseFromString(req->data());
  // send request
  brpc::Controller cntl;
  cntl.http_request().uri() = hr.uri().path();
  cntl.http_request().set_method(brpc::HTTP_METHOD_POST);
  cntl.request_attachment().append(hr.body());
  channel_.CallMethod(nullptr, &cntl, nullptr, nullptr, nullptr);
  if (cntl.Failed()) {
    return absl::ErrnoToStatus(cntl.ErrorCode(), cntl.ErrorText());
  }
  return absl::OkStatus();
}
}  // namespace kfpanda
