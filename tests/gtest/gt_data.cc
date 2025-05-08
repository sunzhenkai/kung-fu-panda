#include <spdlog/spdlog.h>

#include <string>

#include "data/local/rocksdb_storage.h"
#include "google/protobuf/util/json_util.h"
#include "gtest/gtest.h"
#include "protos/service/kfpanda/kfpanda.pb.h"

TEST(Data, Read) {
  auto db = kfpanda::RocksDbManager::GetDb("echo_v2");
  rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
  // 从最后一个键开始遍历
  for (it->SeekToLast(); it->Valid(); it->Prev()) {
    auto v = it->value();
    kfpanda::RecordRequest req;
    req.ParseFromArray(v.data(), v.size());
    std::string jd;
    auto s = google::protobuf::util::MessageToJsonString(req, &jd);
    spdlog::info("status: {}, key: {}, value: {}", s.ToString(), it->key().ToString(), jd);
  }
  delete it;
}
