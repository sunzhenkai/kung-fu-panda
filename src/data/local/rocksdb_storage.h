/**
 * @file rocksdb_storage.h
 * @brief
 * @author zhenkai.sun
 * @date 2025-05-08 15:11:28
 */
#pragma once
#include <cppcommon/extends/spdlog/log.h>

#include <filesystem>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "common/flags.h"
#include "cppcommon/partterns/singleton.h"
#include "rocksdb/db.h"
#include "rocksdb/utilities/db_ttl.h"

namespace kfpanda {
using DBT = rocksdb::DBWithTTL;
class RocksDbManager : public cppcommon::Singleton<RocksDbManager> {
 public:
  static DBT *GetDb(const std::string &service);
  ~RocksDbManager();

 private:
  DBT *get_db(const std::string &service);
  DBT *try_get_db(const std::string &service);

 private:
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
    rocksdb::Status status = DBT::Open(options, pt.string(), &db, FLAGS_data_ttl);
    if (!std::filesystem::exists(pt)) {
      std::filesystem::create_directories(pt);
    }
    if (!status.ok()) {
      RERROR("[{}] create db failed. [service={}, pat={}, message={}]", __func__, service, pt.string(),
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
}  // namespace kfpanda
