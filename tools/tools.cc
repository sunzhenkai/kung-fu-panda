#include <brpc/controller.h>
#include <glog/logging.h>

#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>

#include "cppcommon/extends/spdlog/log.h"
#include "cppcommon/utils/error.h"
#include "cppcommon/utils/to_str.h"
#include "google/protobuf/util/json_util.h"
#include "handler/replay_handler.h"

DEFINE_string(address, "127.0.0.1:9820", "kfpanda service address");
DEFINE_string(pb_files, "", "protobuf files");
DEFINE_string(service, "", "service");
DEFINE_int32(count, 1, "count");
DEFINE_string(target, "", "target base (host:port)");
DEFINE_string(target_base, "", "target base (host:port)");
DEFINE_string(target_compare, "", "target compare (host:port)");

void ReplayV1() {
  brpc::Controller controller;
  ::kfpanda::ReplayRequest request;
  ::kfpanda::ReplayResponse response;

  request.set_service(FLAGS_service);
  request.mutable_option()->set_count(FLAGS_count);
  auto target = FLAGS_target.empty() ? FLAGS_target_base : FLAGS_target;
  request.mutable_target()->set_host(target);

  auto status = kfpanda::ReplayHandler::Handle(
      kfpanda::ReplayContext{.cntl = &controller, .request = &request, .response = &response});
  response.set_code(static_cast<int>(status.code()));
  response.set_message(status.message());

  std::string js;
  auto s = google::protobuf::util::MessageToJsonString(response, &js);
  if (s.ok()) {
    std::cout << js << std::endl;
  } else {
    CERROR("parse response failed. [error={}]", s.message());
  }
}

void ReplayV2() {
  brpc::Controller controller;
  ::kfpanda::ReplayRequestV2 request;
  ::kfpanda::ReplayResponseV2 response;

  request.set_service(FLAGS_service);
  request.mutable_option()->set_count(FLAGS_count);
  request.mutable_target_base()->set_host(FLAGS_target_base);
  request.mutable_target_compare()->set_host(FLAGS_target_compare);

  std::string js;
  auto s = google::protobuf::util::MessageToJsonString(response, &js);
  if (s.ok()) {
    std::cout << js << std::endl;
  } else {
    CERROR("parse response failed. [error={}]", s.message());
  }
}

void Replay() {
  cppcommon::OkOrExit(!FLAGS_service.empty(), "service should not be empty");
  if (FLAGS_target_compare.empty()) {
    auto target = FLAGS_target.empty() ? FLAGS_target_base : FLAGS_target;
    cppcommon::OkOrExit(!target.empty(), "target(or target base) should not be empty");
    ReplayV1();
  } else {
    cppcommon::OkOrExit(!FLAGS_target_base.empty(), "target base should not be empty");
    ReplayV2();
  }
}

void Stat() {}

std::unordered_map<std::string, std::function<void()>> kFunMaps{
    {"replay", Replay},
    {"stat", Stat},
};

int main(int argc, char** argv) {
  CINFO("Run Kung-Fu-Panda Replay Tool. [argv={}]", cppcommon::ToString(argc, argv));
  cppcommon::OkOrExit(argc >= 2, "unpexpected argument count");
  google::SetCommandLineOption("log_dir", "log");
  google::SetCommandLineOption("graceful_quit_on_sigterm", "true");
  GFLAGS_NAMESPACE::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(*argv);
  cppcommon::Assert(google::IsGoogleLoggingInitialized(), "[main] init google logging failed.");

  auto fun = std::string(argv[1]);
  auto it = kFunMaps.find(fun);
  if (it != kFunMaps.end()) {
    it->second();
  } else {
    cppcommon::OkOrExit(false, "unexpected function: " + fun);
  }

  google::ShutdownGoogleLogging();
  return 0;
}
