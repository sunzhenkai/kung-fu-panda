
/**
 * @file flags.h
 * @brief
 * @author zhenkai.sun
 * @date 2025-05-08 15:06:28
 */
#pragma once
#include "gflags/gflags.h"

namespace kfpanda {
DECLARE_int32(port);
DECLARE_string(local_db_path);
DECLARE_int32(data_ttl);
}  // namespace kfpanda
