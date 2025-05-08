#include <fmt/format.h>
#include <protos/service/kfpanda/kfpanda.pb.h>
#include <spdlog/spdlog.h>

#include "client/client.h"
#include "common/flags.h"
#include "gtest/gtest.h"

TEST(Client, SampleWrite) {
  auto server = fmt::format("127.0.0.1:", kfpanda::FLAGS_port);
  auto client = kfpanda::KfpandaClient(server);
  auto s = client.Init();
  ASSERT_TRUE(s.ok());
  auto stub = client.Stub();

  brpc::Controller cntl;
  kfpanda::RecordRequest request;
  kfpanda::RecordResponse response;

  stub->Record(&cntl, &request, &response, nullptr);
  ASSERT_TRUE(!cntl.Failed());
  spdlog::info("code: {}, message: {}", response.code(), response.message());
}
