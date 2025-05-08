#include <glog/logging.h>

#include "brpc/server.h"
#include "common/flags.h"
#include "cppcommon/utils/error.h"
#include "cppcommon/utils/to_str.h"
#include "service/impl/http_impl.h"
#include "service/impl/impl.h"
#include "utils/log.h"

int main(int argc, char** argv) {
  // 1. init
  RINFO("[{}] Run Kung Fu Panda. [argv={}]", __func__, cppcommon::ToString(argc, argv));
  google::SetCommandLineOption("log_dir", "log");
  google::SetCommandLineOption("graceful_quit_on_sigterm", "true");
  GFLAGS_NAMESPACE::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(*argv);
  cppcommon::Assert(google::IsGoogleLoggingInitialized(), "[main] init google logging failed.");

  // 2. start server
  brpc::Server server;
  brpc::ServerOptions options;
  kfpanda::KfPandaServiceImpl service;
  cppcommon::Assert(!server.AddService(&service, brpc::SERVER_DOESNT_OWN_SERVICE), "[main] add service failed.");
  kfpanda::HttpKfPandaServiceImpl http_service;
  auto mp = "/api/echo => Echo,/api/replay => Replay";
  cppcommon::Assert(!server.AddService(&service, brpc::SERVER_DOESNT_OWN_SERVICE, mp), "[main] add service failed.");
  cppcommon::Assert(!server.Start(kfpanda::FLAGS_port, &options), "[main] start server failed.");

  // 3. waiting
  RINFO("[{}] waiting to quit ...", __func__);
  server.RunUntilAskedToQuit();
  google::ShutdownGoogleLogging();
  return 0;
}
