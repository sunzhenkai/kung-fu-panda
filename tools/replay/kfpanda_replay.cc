#include <glog/logging.h>

#include "cppcommon/extends/spdlog/log.h"
#include "cppcommon/utils/error.h"
#include "cppcommon/utils/to_str.h"

DEFINE_string(pb_files, "", "protobuf files");
DEFINE_string(service, "", "service");
DEFINE_int32(count, 1, "count");
DEFINE_string(target_base, "", "target base (host:port)");
DEFINE_string(target_compare, "", "target compare (host:port)");

void CheckArgs() {
  cppcommon::Assert(!FLAGS_service.empty(), "service should not be empty");
  cppcommon::Assert(!FLAGS_target_base.empty(), "target base should not be empty");
}

void ReplayV1() {}
void ReplayV2() {}

int main(int argc, char** argv) {
  CINFO("Run Kung-Fu-Panda Replay Tool. [argv={}]", cppcommon::ToString(argc, argv));
  google::SetCommandLineOption("log_dir", "log");
  google::SetCommandLineOption("graceful_quit_on_sigterm", "true");
  GFLAGS_NAMESPACE::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(*argv);
  cppcommon::Assert(google::IsGoogleLoggingInitialized(), "[main] init google logging failed.");
  CheckArgs();

  if (FLAGS_target_compare.empty()) {
    ReplayV1();
  } else {
    ReplayV2();
  }

  google::ShutdownGoogleLogging();
  return 0;
}
