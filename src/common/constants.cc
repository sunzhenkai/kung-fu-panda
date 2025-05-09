#include "constants.h"

#include <absl/status/status.h>

namespace kfpanda {
const absl::Status kReqErr = absl::ErrnoToStatus(400, "invalid request. empty service?");
const absl::Status kProtocolErr = absl::ErrnoToStatus(401, "protocol error");

const absl::Status kDbErr = absl::ErrnoToStatus(500, "write data into db failed");
}  // namespace kfpanda
