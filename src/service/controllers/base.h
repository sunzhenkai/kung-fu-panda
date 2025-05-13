/**
 * @file base.h
 * @brief
 * @author zhenkai.sun
 * @date 2025-05-13 21:53:09
 */
#pragma once
#include <functional>
#include <string>

#include "absl/status/status.h"
#include "brpc/controller.h"
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

inline std::string Response::ToJson() { return ToJson(status_); }

inline std::string Response::ToJson(const absl::Status &status) {
  auto code = static_cast<int>(status.code());
  jb_.Add("success", code == 0);
  jb_.Add("code", code);
  jb_.Add("message", status.message());
  return jb_.Build();
}

inline std::string Response::From(const absl::Status &status) {
  Response res(status);
  return res.ToJson();
}

using ViewFunc = std::function<absl::Status(brpc::Controller *cntl, Response *rsp)>;
}  // namespace kfpanda
