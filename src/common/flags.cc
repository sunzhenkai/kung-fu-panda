#include "flags.h"

#include <gflags/gflags.h>

namespace kfpanda {
DEFINE_int32(port, 9820, "listening port");
DEFINE_string(local_db_path, "/tmp/fkpanda", "local db root path");
DEFINE_int32(data_ttl, 60 * 60 * 24 * 7, "data ttl");  // default: 7day
}  // namespace kfpanda
