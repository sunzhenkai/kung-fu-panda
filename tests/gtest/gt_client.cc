#include <fmt/format.h>
#include <google/protobuf/descriptor.h>
#include <protos/service/kfpanda/kfpanda.pb.h>
#include <spdlog/spdlog.h>

#include <string>

#include "client/client.h"
#include "client/debug_client.h"
#include "common/flags.h"
#include "gtest/gtest.h"
#include "replay/grpc_replay.h"

TEST(Client, SampleWrite) {
  auto server = fmt::format("127.0.0.1:{}", kfpanda::FLAGS_port);
  auto client = kfpanda::KfpandaClient(server);
  auto s = client.Init();
  ASSERT_TRUE(s.ok());
  auto stub = client.Stub();

  brpc::Controller cntl;
  kfpanda::RecordRequest request;
  request.set_service("echo_v2");
  kfpanda::RecordResponse response;

  stub->Record(&cntl, &request, &response, nullptr);
  ASSERT_TRUE(!cntl.Failed());
  spdlog::info("code: {}, message: {}", response.code(), response.message());
}

TEST(Client, GrpcEcho) {
  auto server = fmt::format("127.0.0.1:{}", kfpanda::FLAGS_port);
  auto client = kfpanda::KfPandaDebugClient(server);
  auto s = client.Init();
  ASSERT_TRUE(s.ok());
  auto stub = client.Stub();

  brpc::Controller cntl;
  kfpanda::EchoMessage request;
  request.set_message("Client.GrpcEcho test message");
  kfpanda::EchoMessage response;

  stub->Echo(&cntl, &request, &response, nullptr);
  ASSERT_TRUE(!cntl.Failed());
  spdlog::info("response: {}", cntl.response_attachment().to_string());
}

TEST(Client, GrpcReplay) {
  auto target = kfpanda::URI{};
  target.set_host("127.0.0.1");
  target.set_port(kfpanda::FLAGS_port);
  auto grpc_replay_client = kfpanda::GrpcReplayClient(target);
  kfpanda::RecordRequest req;
  kfpanda::HttpRequest echo_req;
  auto uri = req.mutable_uri();
  uri->set_path("/kfpanda.KfPandaDebugService/Echo");
  echo_req.SerializeToString(req.mutable_data());
  kfpanda::SvcResponse rsp;
  auto s = grpc_replay_client.Replay(&req, &rsp);
  spdlog::info("echo result: code={}, message={}", static_cast<int>(s.code()), s.message());
}
