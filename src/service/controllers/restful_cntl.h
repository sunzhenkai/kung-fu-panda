/**
 * @file restful_cntl.h
 * @brief
 * @author zhenkai.sun
 * @date 2025-05-10 11:34:33
 */
#pragma once

#include <absl/status/status.h>
#include <brpc/controller.h>

#include <string>
#include <unordered_map>

#include "base.h"

namespace kfpanda {
extern const std::unordered_map<std::string, ViewFunc> kRestfulApiViews;

absl::Status api_echo(brpc::Controller *cntl, Response *rsp);
absl::Status api_grpc_echo(brpc::Controller *cntl, Response *rsp);
absl::Status api_replay(brpc::Controller *cntl, Response *rsp);
absl::Status api_replay_v2(brpc::Controller *cntl, Response *rsp);
absl::Status api_debug_stat(brpc::Controller *cntl, Response *rsp);
absl::Status api_debug_sample(brpc::Controller *cntl, Response *rsp);
absl::Status api_log_record(brpc::Controller *cntl, Response *rsp);
absl::Status api_log_sample(brpc::Controller *cntl, Response *rsp);
absl::Status api_log_stat(brpc::Controller *cntl, Response *rsp);
}  // namespace kfpanda
