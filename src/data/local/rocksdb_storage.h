/**
 * @file rocksdb_storage.h
 * @brief
 * @author zhenkai.sun
 * @date 2025-05-08 15:11:28
 */
#pragma once
#include <cppcommon/extends/spdlog/log.h>
#include <fmt/format.h>

#include <filesystem>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/flags.h"
#include "cppcommon/partterns/singleton.h"
#include "rocksdb/db.h"
#include "rocksdb/utilities/db_ttl.h"

namespace kfpanda {
using CStr = const std::string;
using DBT = rocksdb::DBWithTTL;
using DBItemT = std::pair<std::string, std::string>;
using DBItemsT = std::vector<DBItemT>;
using DBStatT = std::unordered_map<std::string, std::string>;

class RocksDbManager : public cppcommon::Singleton<RocksDbManager> {
 public:
  static DBT *GetDb(const std::string &service);
  static DBItemsT TryGetIterms(const std::string &service, size_t count = 1);
  static DBStatT GetDbState();

  ~RocksDbManager();

 protected:
  DBT *get_db(const std::string &service);
  DBT *try_get_db(const std::string &service);

 protected:
  std::unordered_map<std::string, DBT *> store_;
  std::shared_mutex store_mtx_;
};

inline RocksDbManager::~RocksDbManager() {
  std::unique_lock lock(store_mtx_);
  for (auto &[k, v] : store_) {
    delete v;
    v = nullptr;
  }
}

inline DBT *RocksDbManager::try_get_db(const std::string &service) {
  auto it = store_.find(service);
  if (it != store_.end()) return it->second;
  return nullptr;
}

inline DBT *RocksDbManager::get_db(const std::string &service) {
  DBT *db = nullptr;
  {
    std::shared_lock lock(store_mtx_);
    try_get_db(service);
  }
  if (db == nullptr) {
    std::unique_lock lock(store_mtx_);
    db = try_get_db(service);
    if (db != nullptr) return db;

    rocksdb::Options options;
    options.create_if_missing = true;

    auto pt = std::filesystem::path(fLS::FLAGS_local_db_path) / std::filesystem::path(service);
    if (!std::filesystem::exists(pt)) {
      std::filesystem::create_directories(pt);
    }
    rocksdb::Status status = DBT::Open(options, pt.string(), &db, FLAGS_data_ttl);
    if (!status.ok()) {
      RERROR("[{}] create db failed. [service={}, path={}, message={}]", __func__, service, pt.string(),
             status.ToString());
      return nullptr;
    } else {
      store_[service] = db;
      return db;
    }
  } else {
    return db;
  }
}

inline DBT *RocksDbManager::GetDb(const std::string &service) { return Instance().get_db(service); }

inline DBItemsT RocksDbManager::TryGetIterms(const std::string &service, size_t count) {
  count = std::min(count, 1000ul);
  DBItemsT ret;
  auto db = Instance().get_db(service);
  if (db != nullptr) {
    rocksdb::Iterator *it = db->NewIterator(rocksdb::ReadOptions());
    // traverse by timestamp in desc order
    for (it->SeekToLast(); it->Valid(); it->Prev()) {
      ret.emplace_back(it->key().ToString(), it->value().ToString());
      if (ret.size() >= count) break;
    }
    delete it;
  }
  return ret;
}
inline DBStatT RocksDbManager::GetDbState() {
  DBStatT ret;
  std::shared_lock lock(Instance().store_mtx_);
  for (auto it = Instance().store_.begin(); it != Instance().store_.end(); ++it) {
    auto &v = ret[it->first];
    it->second->GetProperty("rocksdb.estimate-num-keys", &v);
  }
  return ret;
}

class LogsRocksDbManager : public RocksDbManager {
 public:
  static DBT *GetDb(const std::string &service, const std::string &log_name);
  static DBItemsT TryGetIterms(const std::string &service, const std::string &log_name, size_t count = 1);
};

inline DBT *LogsRocksDbManager::GetDb(const std::string &service, const std::string &log_name) {
  return RocksDbManager::GetDb(fmt::format("{}::{}", service, log_name));
}

inline DBItemsT LogsRocksDbManager::TryGetIterms(CStr &service, CStr &log_name, size_t count) {
  return RocksDbManager::TryGetIterms(fmt::format("{}::{}", service, log_name), count);
}
}  // namespace kfpanda
