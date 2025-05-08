/**
 * @file base.h
 * @brief
 * @author zhenkai.sun
 * @date 2025-05-08 14:44:35
 */
#pragma once
#include "absl/status/status.h"

namespace kfpanda {
class BaseHandler {
  virtual absl::Status Prepare() { return absl::OkStatus(); }
  virtual absl::Status Process();
  virtual absl::Status Post() { return absl::OkStatus(); }
};
}  // namespace kfpanda
