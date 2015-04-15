
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/store/store.h"

#include "idgs/application.h"

using namespace google::protobuf;
using namespace protobuf;

namespace idgs {
namespace store {

/// class PartitionStore
/// @fixme one member only own part of partitions.
ResultCode PartitionStore::initialize() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  auto part_size = idgs_application()->getClusterFramework()->getPartitionCount();
  for (int32_t i = 0; i < part_size; ++ i) {
    auto it = dataMap.find(i);
    if (it == dataMap.end()) {
      if (getStoreConfigWrapper()->getStoreConfig().store_type() == pb::ORDERED) {
        dataMap[i] = std::make_shared<TreeMap>();
      } else if (getStoreConfigWrapper()->getStoreConfig().store_type() == pb::UNORDERED) {
        dataMap[i] = std::make_shared<HashMap>();
      }
    }
  }

  return RC_SUCCESS;
}

ResultCode PartitionStore::snapshotStore(const int32_t& partition, std::shared_ptr<StoreMap>& map) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  auto it = dataMap.find(partition);
  if (it == dataMap.end()) {
    return RC_PARTITION_NOT_FOUND;
  }

  StoreMap* snapshot = NULL;
  if (getStoreConfigWrapper()->getStoreConfig().store_type() == idgs::store::pb::ORDERED) {
    auto treeMap = dynamic_cast<TreeMap*>(it->second.get());
    snapshot = new TreeMap(* treeMap);
  } else if (getStoreConfigWrapper()->getStoreConfig().store_type() == idgs::store::pb::UNORDERED) {
    auto hashMap = dynamic_cast<HashMap*>(it->second.get());
    snapshot = new HashMap(* hashMap);
  } else {
    return RC_NOT_SUPPORT;
  }

  map.reset(snapshot);

  return RC_SUCCESS;
}

/// @todo partitionExist
ResultCode PartitionStore::getData(const StoreKey<Message>& key, StoreValue<Message>& value, PartitionInfo* status) {
  std::shared_ptr<StoreMap> map;
  int32_t partition = getDataPartition(key, status);
  bool partitionExist = false;
  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
    DVLOG(1) << "partition store get data";

    auto it = dataMap.find(partition);
    partitionExist = (it != dataMap.end());
    if (partitionExist) {
      map = it->second;
    }
  }
  if (!partitionExist) {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    string s;
    auto i = dataMap.begin();
    for (; i != dataMap.end(); ++i) {
      s.append(to_string(i->first)).append(", ");
    }

    LOG_FIRST_N(INFO, 20) << s;
    DVLOG(2) << "partition : " << partition;
    return RC_PARTITION_NOT_FOUND;
  }

  return map->get(key, value);
}

ResultCode PartitionStore::setData(const StoreKey<Message>& key, StoreValue<Message>& value, PartitionInfo* status) {
  std::shared_ptr<StoreMap> map;
  int32_t partition = getDataPartition(key, status);
  bool partitionExist = false;
  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
    DVLOG_EVERY_N(1, 100) << "partition store insert data";

    auto it = dataMap.find(partition);
    partitionExist = (it != dataMap.end());
    if (partitionExist) {
      map = it->second;
    }
  }
  if (!partitionExist) {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    string s;
    auto i = dataMap.begin();
    for (; i != dataMap.end(); ++i) {
      s.append(to_string(i->first)).append(", ");
    }

    LOG_FIRST_N(INFO, 20) << s;
    DVLOG(2) << "partition : " << partition;
    return RC_PARTITION_NOT_FOUND;
  }

  return map->set(key, value);
}

ResultCode PartitionStore::removeData(const StoreKey<Message>& key, StoreValue<Message>& value, PartitionInfo* status) {
  std::shared_ptr<StoreMap> map;
  int32_t partition = getDataPartition(key, status);
  bool partitionExist = false;
  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
    DVLOG(1) << "partition store delete data";

    auto it = dataMap.find(partition);
    partitionExist = (it != dataMap.end());
    if (partitionExist) {
      map = it->second;
    }
  }
  if (!partitionExist) {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    string s;
    auto i = dataMap.begin();
    for (; i != dataMap.end(); ++i) {
      s.append(to_string(i->first)).append(", ");
    }

    LOG_FIRST_N(INFO, 20) << s;
    DVLOG(2) << "partition : " << partition;
    return RC_PARTITION_NOT_FOUND;
  }

  return map->remove(key, value);
}

void PartitionStore::clearData() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  auto partSize = idgs_application()->getClusterFramework()->getPartitionCount();
  for (int32_t i = 0; i < partSize; ++ i) {
    auto it = dataMap.find(i);
    if (it != dataMap.end()) {
      it->second->clear();
    }
  }
}

size_t PartitionStore::dataSize() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  size_t size = 0;
  auto cluster = idgs_application()->getClusterFramework();
  auto localMemberId = cluster->getMemberManager()->getLocalMemberId();
  auto partSize = cluster->getPartitionCount();
  for (int32_t i = 0; i < partSize; ++ i) {
    if (localMemberId == cluster->getPartitionManager()->getPartition(i)->getPrimaryMemberId()) {
      auto it = dataMap.find(i);
      if (it != dataMap.end()) {
        size += it->second->size();
      }
    }
  }

  return size;
}

size_t PartitionStore::dataSize(const uint32_t& partition) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  auto it = dataMap.find(partition);
  if (it == dataMap.end()) {
    return 0;
  }

  return it->second->size();
}

ResultCode PartitionStore::clearData(const uint32_t& partition) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  auto it = dataMap.find(partition);
  if (it == dataMap.end()) {
    return RC_PARTITION_NOT_FOUND;
  }

  it->second->clear();

  return RC_SUCCESS;
}

ResultCode PartitionStore::scanPartitionData(const uint32_t& partition, shared_ptr<StoreMap>& dataMap) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  auto it = this->dataMap.find(partition);
  if (it == this->dataMap.end()) {
    return RC_PARTITION_NOT_FOUND;
  }

  dataMap = it->second;

  return RC_SUCCESS;
}

uint32_t PartitionStore::getDataPartition(const StoreKey<Message>& key, PartitionInfo* status) {
  if (status) {
    return  status->partitionId;
  } else {
    if (getStoreConfigWrapper()) {
      PartitionInfo ps;
      getStoreConfigWrapper()->calculatePartitionInfo(const_cast<StoreKey<Message>&>(key), &ps);
      return ps.partitionId;
    } else {
      LOG(WARNING) << "store config is NULL";
      uint32_t partSize = idgs_application()->getClusterFramework()->getPartitionCount();
      hashcode_t hashcode = HashCode::hashcode(key.get());
      return hashcode % partSize;
    }
  }
}

void PartitionStore::addMigrationActor(idgs::actor::Actor* actor) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  migrationActors.push_back(actor);
  migrationActorIds.insert(actor->getActorId());
}

void PartitionStore::removeMigrationActor(idgs::actor::Actor* actor) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  auto it = find(migrationActors.begin(), migrationActors.end(), actor);
  if (it != migrationActors.end()) {
    migrationActors.erase(it);
  }

  auto idit = migrationActorIds.find(actor->getActorId());
  if (idit != migrationActorIds.end()) {
    migrationActorIds.erase(idit);
  }
}

std::vector<idgs::actor::Actor*> PartitionStore::getMigrationActors() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  return migrationActors;
}

std::set<std::string> PartitionStore::getMigrationActorIds() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  return migrationActorIds;
}

// class PartitionStore

// class ReplicatedStore
ResultCode ReplicatedStore::initialize() {
  if (getStoreConfigWrapper()->getStoreConfig().store_type() == pb::ORDERED) {
    dataMap = std::make_shared<TreeMap>();
  } else if (getStoreConfigWrapper()->getStoreConfig().store_type() == pb::UNORDERED) {
    dataMap = std::make_shared<HashMap>();
  }

  return RC_SUCCESS;
}

ResultCode ReplicatedStore::getData(const StoreKey<Message>& key, StoreValue<Message>& value, PartitionInfo* status) {
  return dataMap->get(key, value);
}

ResultCode ReplicatedStore::setData(const StoreKey<Message>& key, StoreValue<Message>& value, PartitionInfo* status) {
  return dataMap->set(key, value);
}

ResultCode ReplicatedStore::removeData(const StoreKey<Message>& key, StoreValue<Message>& value, PartitionInfo* status) {
  return dataMap->remove(key, value);
}

size_t ReplicatedStore::dataSize() {
  return dataMap->size();
}

void ReplicatedStore::clearData() {
  DVLOG(1) << "clear replicated store";
  dataMap->clear();
}

ResultCode ReplicatedStore::scanData(std::shared_ptr<StoreMap>& dataMap) {
  dataMap = this->dataMap;
  return RC_SUCCESS;
}

ResultCode ReplicatedStore::snapshotStore(std::shared_ptr<StoreMap>& map) {
  StoreMap* snapshot = NULL;
  if (getStoreConfigWrapper()->getStoreConfig().store_type() == idgs::store::pb::ORDERED) {
    auto treeMap = dynamic_cast<TreeMap*>(dataMap.get());
    snapshot = new TreeMap(* treeMap);
  } else if (getStoreConfigWrapper()->getStoreConfig().store_type() == idgs::store::pb::UNORDERED) {
    auto hashMap = dynamic_cast<HashMap*>(dataMap.get());
    snapshot = new HashMap(* hashMap);
  } else {
    return RC_NOT_SUPPORT;
  }

  map.reset(snapshot);

  return RC_SUCCESS;
}

void ReplicatedStore::addSyncActor(idgs::actor::Actor* actor) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  syncActors.push_back(actor);
  syncActorIds.insert(actor->getActorId());
}

void ReplicatedStore::removeSyncActor(idgs::actor::Actor* actor) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  auto it = find(syncActors.begin(), syncActors.end(), actor);
  if (it != syncActors.end()) {
    syncActors.erase(it);
  }

  auto idit = syncActorIds.find(actor->getActorId());
  if (idit != syncActorIds.end()) {
    syncActorIds.erase(idit);
  }
}

std::vector<idgs::actor::Actor*> ReplicatedStore::getSyncActors() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  return syncActors;
}

std::set<std::string> ReplicatedStore::getSyncActorIds() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  return syncActorIds;
}

// class ReplicatedStore


}// namespace store
} // namespace idgs
