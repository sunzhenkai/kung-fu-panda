#pragma once
#include "brpc/closure_guard.h"
#include "handler/log_handler.h"
#include "handler/record_handler.h"
#include "handler/replay_handler.h"
#include "protos/service/kfpanda/kfpanda.pb.h"

namespace kfpanda {
class KfPandaServiceImpl : public kfpanda::KfPandaService {
  void Record(::google::protobuf::RpcController* controller, const ::kfpanda::RecordRequest* request,
              ::kfpanda::RecordResponse* response, ::google::protobuf::Closure* done) override;
  void Replay(::google::protobuf::RpcController* controller, const ::kfpanda::ReplayRequest* request,
              ::kfpanda::ReplayResponse* response, ::google::protobuf::Closure* done) override;
  void ReplayV2(::google::protobuf::RpcController* controller, const ::kfpanda::ReplayRequestV2* request,
                ::kfpanda::ReplayResponseV2* response, ::google::protobuf::Closure* done) override;
  void Sample(::google::protobuf::RpcController* controller, const ::kfpanda::SampleRequest* request,
              ::kfpanda::SampleResponse* response, ::google::protobuf::Closure* done) override;
  void Log(::google::protobuf::RpcController* controller, const ::kfpanda::LogRequest* request,
           ::kfpanda::LogResponse* response, ::google::protobuf::Closure* done) override;
};

inline void KfPandaServiceImpl::Record(::google::protobuf::RpcController* controller,
                                       const ::kfpanda::RecordRequest* request, ::kfpanda::RecordResponse* response,
                                       ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  auto status = RecordHandler::Handle(RecordContext{.cntl = controller, .request = request, .response = response});
  response->set_code(static_cast<int>(status.code()));
  response->set_message(status.message());
}

inline void KfPandaServiceImpl::Replay(::google::protobuf::RpcController* controller,
                                       const ::kfpanda::ReplayRequest* request, ::kfpanda::ReplayResponse* response,
                                       ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  auto status = ReplayHandler::Handle(ReplayContext{.cntl = controller, .request = request, .response = response});
  response->set_code(static_cast<int>(status.code()));
  response->set_message(status.message());
}

inline void KfPandaServiceImpl::ReplayV2(::google::protobuf::RpcController* controller,
                                         const ::kfpanda::ReplayRequestV2* request,
                                         ::kfpanda::ReplayResponseV2* response, ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  auto status = ReplayHandlerV2::Handle(ReplayV2Context{.cntl = controller, .request = request, .response = response});
  response->set_code(static_cast<int>(status.code()));
  response->set_message(status.message());
}

inline void KfPandaServiceImpl::Sample(::google::protobuf::RpcController* controller,
                                       const ::kfpanda::SampleRequest* request, ::kfpanda::SampleResponse* response,
                                       ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  auto items = RocksDbManager::TryGetIterms(request->service(), request->count());
  for (auto& item : items) {
    response->add_data(item.second);
  }
  response->set_success(true);
  response->set_code(0);
}

inline void KfPandaServiceImpl::Log(::google::protobuf::RpcController* controller, const ::kfpanda::LogRequest* request,
                                    ::kfpanda::LogResponse* response, ::google::protobuf::Closure* done) {
  brpc::ClosureGuard dg(done);
  auto status = LogHandler::Handle(LogContext{.cntl = controller, .request = request, .response = response});
  response->set_code(static_cast<int>(status.code()));
  response->set_message(status.message());
}
}  // namespace kfpanda
