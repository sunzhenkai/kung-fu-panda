/**
 * @file restful_cntl.h
 * @brief
 * @author zhenkai.sun
 * @date 2025-05-10 11:34:33
 */
#pragma once

#include <absl/status/status.h>
#include <brpc/controller.h>

#include <functional>
#include <string>
#include <unordered_map>

#include "cppcommon/extends/rapidjson/builder.h"

namespace kfpanda {
class Response {
 public:
  Response() : status_(absl::OkStatus()) {}
  explicit Response(const absl::Status &status) : status_(status) {}

  template <typename K, typename V>
  void Add(const K &k, const V &v);
  template <typename K, typename V>
  void AddJsonStr(const K &k, const V &v);
  std::string ToJson();
  std::string ToJson(const absl::Status &status);

  static std::string From(const absl::Status &status);
  template <typename V>
  static std::string From(const absl::Status &status, const V &v);

 private:
  absl::Status status_;
  cppcommon::JsonBuilder jb_;
};

template <typename K, typename V>
void Response::Add(const K &k, const V &v) {
  jb_.Add(k, v);
}

template <typename K, typename V>
void Response::AddJsonStr(const K &k, const V &v) {
  jb_.AddJsonStr(k, v);
}

template <typename V>
std::string Response::From(const absl::Status &status, const V &v) {
  Response res;
  res.Add("data", v);
  return res.ToJson(status);
}

using ViewFunc = std::function<absl::Status(brpc::Controller *cntl, Response *rsp)>;
extern const std::unordered_map<std::string, ViewFunc> kRestfulApiViews;

absl::Status api_echo(brpc::Controller *cntl, Response *rsp);
absl::Status api_grpc_echo(brpc::Controller *cntl, Response *rsp);
absl::Status api_replay(brpc::Controller *cntl, Response *rsp);
absl::Status api_debug_stat(brpc::Controller *cntl, Response *rsp);
absl::Status api_debug_sample(brpc::Controller *cntl, Response *rsp);
}  // namespace kfpanda
