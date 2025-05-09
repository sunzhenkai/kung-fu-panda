#include <google/protobuf/util/json_util.h>

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "protos/service/kfpanda/kfpanda.pb.h"

TEST(Demo, GenReplayApiData) {
  kfpanda::ReplayRequest req;
  req.set_request_id("7a49c678-205e-4c83-884b-796d865c2964");
  req.set_service("KungFuPandaServer");
  req.mutable_target()->set_ip("127.0.0.1");
  req.mutable_target()->set_port(9820);
  req.mutable_option()->set_count(1);
  req.mutable_option()->set_timeout_ms(1000);

  std::string js;
  auto s = google::protobuf::util::MessageToJsonString(req, &js);
  if (s.ok()) {
    std::cout << js << std::endl;
  } else {
    std::cout << s.message() << std::endl;
  }
}
