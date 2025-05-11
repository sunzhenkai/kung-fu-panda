#include <fmt/format.h>
#include <google/protobuf/descriptor.h>
#include <protos/service/kfpanda/kfpanda.pb.h>
#include <spdlog/spdlog.h>

#include <string>

#include "client/client.h"
#include "client/http_client.h"
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
  auto client = kfpanda::HttpKfPandaClient(server);
  auto s = client.Init();
  ASSERT_TRUE(s.ok());
  auto stub = client.Stub();

  brpc::Controller cntl;
  kfpanda::HttpRequest request;
  kfpanda::HttpResponse response;

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
  uri->set_path("/HttpKfPandaService/Echo");
  echo_req.SerializeToString(req.mutable_data());
  kfpanda::SvcResponse rsp;
  auto s = grpc_replay_client.Replay(&req, &rsp);
  spdlog::info("echo result: code={}, message={}", static_cast<int>(s.code()), s.message());

  // auto server = fmt::format("127.0.0.1:{}", kfpanda::FLAGS_port);
  // brpc::Channel channel_;
  // brpc::ChannelOptions options;
  // options.protocol = "baidu_std";
  // options.connection_type = "single";
  // options.timeout_ms = 1000 /*milliseconds*/;
  // options.max_retry = 0;
  // auto s = channel_.Init(server.c_str(), "", &options);
  // ASSERT_EQ(s, 0);
  //
  // brpc::Controller cntl;
  // // auto desc_service = const_cast<google::protobuf::ServiceDescriptor *>(kfpanda::KfPandaService::descriptor());
  // // auto desc_method = const_cast<google::protobuf::MethodDescriptor *>(desc_service->method(0));
  //
  // google::protobuf::MethodDescriptor *mtd;
  // google::protobuf::Message *request_msg;
  // google::protobuf::Message *response_msg{nullptr};
  // // send request
  // kfpanda::HttpRequest req;
  // std::string request_bytes;
  // req.SerializeToString(&request_bytes);
  // channel_.CallMethod(mtd, &cntl, request_msg, response_msg, nullptr);
  // ASSERT_TRUE(!cntl.Failed());

  // kfpanda::RecordRequest request;
  // kfpanda::RecordResponse response;
  // spdlog::info("code: {}, message: {}", response.code(), response.message());
}
