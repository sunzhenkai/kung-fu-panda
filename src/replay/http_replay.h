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
#include <protos/service/kfpanda/kfpanda.pb.h>
#include <spdlog/spdlog.h>

#include "absl/status/status.h"
#include "brpc/channel.h"
#include "cppcommon/extends/spdlog/log.h"

namespace kfpanda {
class HttpReplayClient {
 public:
  explicit HttpReplayClient(const kfpanda::URI &target) : target_(target) { Init(); }
  inline bool IsOk() { return !has_error_; }
  absl::Status Replay(const kfpanda::RecordRequest *req, kfpanda::ReplayResponse::ServiceResponse *rsp);

 private:
  void Init();

 private:
  brpc::Channel channel_;
  kfpanda::URI target_;
  bool has_error_{false};
};

inline void HttpReplayClient::Init() {
  brpc::ChannelOptions options;
  options.protocol = "http";
  options.timeout_ms = 1000;
  options.max_retry = 0;
  auto server = fmt::format("{}:{}", target_.host(), target_.port());
  if (channel_.Init(server.c_str(), "", &options) != 0) {
    RERROR("absl::string_view message");
    has_error_ = true;
  }
}

inline absl::Status HttpReplayClient::Replay(const kfpanda::RecordRequest *req,
                                             kfpanda::ReplayResponse::ServiceResponse *rsp) {
  if (has_error_) {
    auto msg = fmt::format("http replay client init failed. [host={}, port={}]", target_.host(), target_.port());
    return absl::ErrnoToStatus(501, msg);
  }
  auto rid = req->request_id();
  // send request
  brpc::Controller cntl;
  // firstly using the path in replay request
  if (!target_.path().empty()) {
    cntl.http_request().uri() = target_.path();
  } else {
    cntl.http_request().uri() = req->uri().path();
  }
  cntl.http_request().set_method(brpc::HTTP_METHOD_POST);
  cntl.request_attachment().append(req->data());
  channel_.CallMethod(nullptr, &cntl, nullptr, nullptr, nullptr);
  if (cntl.Failed()) {
    return absl::ErrnoToStatus(cntl.ErrorCode(), cntl.ErrorText());
  }
  rsp->set_message(cntl.response_attachment().to_string());
  return absl::OkStatus();
}
}  // namespace kfpanda
