/**
 * @file grpc_replay.h
 * @brief
 * @author zhenkai.sun
 * @date 2025-05-11 14:19:16
 */
#pragma once

#include <brpc/controller.h>
#include <brpc/http_method.h>
#include <fmt/format.h>
#include <protos/service/kfpanda/kfpanda.pb.h>
#include <spdlog/spdlog.h>

#include <string>

#include "absl/status/status.h"
#include "butil/base64.h"
#include "replay_client.h"

namespace kfpanda {
class GrpcReplayClient : public ReplayClient {
 public:
  explicit GrpcReplayClient(const kfpanda::URI &target) : ReplayClient(target) { Init("h2"); }
  absl::Status Replay(const kfpanda::RecordRequest *req, SvcResponse *rsp);
};

inline std::string BuildGrpcFrameHeader(const std::string &binary_data) {
  uint8_t compressed_flag = 0;  // 0: no compress
  uint32_t length = htonl(binary_data.size());
  std::string frame_header;
  frame_header.append(1, compressed_flag);
  frame_header.append(reinterpret_cast<const char *>(&length), 4);
  return frame_header;
}

inline absl::Status GrpcReplayClient::Replay(const kfpanda::RecordRequest *req, SvcResponse *rsp) {
  if (has_error_) {
    auto msg = fmt::format("replay client init failed. [host={}, port={}]", target_.host(), target_.port());
    return absl::ErrnoToStatus(501, msg);
  }

  auto rid = req->request_id();
  // send request
  brpc::Controller cntl;
  // firstly using the path in replay request
  auto path = target_.path().empty() ? req->uri().path() : target_.path();

  cntl.http_request().uri() = path;
  cntl.http_request().set_content_type("application/grpc");
  cntl.http_request().SetHeader("te", "trailers");

  auto frame_header = BuildGrpcFrameHeader(req->data());
  cntl.request_attachment().append(frame_header);
  cntl.request_attachment().append(req->data());

  channel_.CallMethod(nullptr, &cntl, nullptr, nullptr, nullptr);
  if (cntl.Failed()) {
    auto msg = fmt::format("[GrpcReplayClient::Replay] replay failed. [path={}, erro={}]", path, cntl.ErrorText());
    return absl::ErrnoToStatus(cntl.ErrorCode(), msg);
  }
  butil::Base64Encode(cntl.response_attachment().to_string(), rsp->mutable_body());
  return absl::OkStatus();
}
}  // namespace kfpanda
