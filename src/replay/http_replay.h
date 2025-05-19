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
#include "replay_client.h"

namespace kfpanda {
class HttpReplayClient : public kfpanda::ReplayClient {
 public:
  explicit HttpReplayClient(const kfpanda::URI &target) : ReplayClient(target) { Init("http"); }
  absl::Status Replay(const kfpanda::RecordRequest *req, SvcResponse *rsp);
};

inline absl::Status HttpReplayClient::Replay(const kfpanda::RecordRequest *req, SvcResponse *rsp) {
  if (has_error_) {
    auto msg = fmt::format("replay client init failed. [host={}, port={}]", target_.host(), target_.port());
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
  rsp->set_type_str(kfpanda::RecordType_Name(req->type()));
  if (cntl.response_attachment().empty()) {
    rsp->set_message("empty response. [code={}]", cntl.request_code());
  } else {
    rsp->set_message(cntl.response_attachment().to_string());
  }
  return absl::OkStatus();
}
}  // namespace kfpanda
