#include "constants.h"

#include <absl/status/status.h>

namespace kfpanda {
const absl::Status kDbErr = absl::ErrnoToStatus(500, "write data into db failed");
}
