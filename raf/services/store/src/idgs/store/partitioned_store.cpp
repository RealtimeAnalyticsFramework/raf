
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include "idgs/store/partitioned_store.h"
#include "idgs/application.h"


namespace idgs {
namespace store {

/// class PartitionStore
/// @fixme one member only own part of partitions.
ResultCode PartitionedStore::initialize() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  auto part_size = idgs_application()->getClusterFramework()->getPartitionCount();
  for (int32_t i = 0; i < part_size; ++ i) {
    auto it = dataMap.find(i);
    if (it == dataMap.end()) {
      if (getStoreConfig()->getStoreConfig().store_type() == pb::ORDERED) {
        dataMap[i] = std::make_shared<TreeMap>();
      } else if (getStoreConfig()->getStoreConfig().store_type() == pb::UNORDERED) {
        dataMap[i] = std::make_shared<HashMap>();
      }
    }
  }

  return RC_SUCCESS;
}

ResultCode PartitionedStore::snapshotStore(const int32_t& partition, std::shared_ptr<StoreMap>& map) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  auto it = dataMap.find(partition);
  if (it == dataMap.end()) {
    return RC_PARTITION_NOT_FOUND;
  }

  auto temp = it->second->snapshot();
  map.swap(temp);
  return RC_SUCCESS;
}

/// @todo partitionExist
ResultCode PartitionedStore::get(const StoreKey<google::protobuf::Message>& key,
    StoreValue<google::protobuf::Message>& value, StoreOption* status) {
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
    std::string s;
    auto i = dataMap.begin();
    for (; i != dataMap.end(); ++i) {
      s.append(std::to_string(i->first)).append(", ");
    }

    LOG_FIRST_N(INFO, 20) << s;
    DVLOG(2) << "partition : " << partition;
    return RC_PARTITION_NOT_FOUND;
  }

  return map->get(key, value);
}

ResultCode PartitionedStore::put(const StoreKey<google::protobuf::Message>& key,
    StoreValue<google::protobuf::Message>& value, StoreOption* status) {
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
    std::string s;
    auto i = dataMap.begin();
    for (; i != dataMap.end(); ++i) {
      s.append(std::to_string(i->first)).append(", ");
    }

    LOG_FIRST_N(INFO, 20) << s;
    DVLOG(2) << "partition : " << partition;
    return RC_PARTITION_NOT_FOUND;
  }

  return map->set(key, value);
}

ResultCode PartitionedStore::remove(const StoreKey<google::protobuf::Message>& key,
    StoreValue<google::protobuf::Message>& value, StoreOption* status) {
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
    std::string s;
    auto i = dataMap.begin();
    for (; i != dataMap.end(); ++i) {
      s.append(std::to_string(i->first)).append(", ");
    }

    LOG_FIRST_N(INFO, 20) << s;
    DVLOG(2) << "partition : " << partition;
    return RC_PARTITION_NOT_FOUND;
  }

  return map->remove(key, value);
}

void PartitionedStore::removeAll() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  auto partSize = idgs_application()->getClusterFramework()->getPartitionCount();
  for (int32_t i = 0; i < partSize; ++ i) {
    auto it = dataMap.find(i);
    if (it != dataMap.end()) {
      it->second->clear();
    }
  }
}

size_t PartitionedStore::size() {
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

size_t PartitionedStore::dataSize(const uint32_t& partition) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  auto it = dataMap.find(partition);
  if (it == dataMap.end()) {
    return 0;
  }

  return it->second->size();
}

ResultCode PartitionedStore::clearData(const uint32_t& partition) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  auto it = dataMap.find(partition);
  if (it == dataMap.end()) {
    return RC_PARTITION_NOT_FOUND;
  }

  it->second->clear();

  return RC_SUCCESS;
}

ResultCode PartitionedStore::scanPartitionData(const uint32_t& partition, std::shared_ptr<StoreMap>& dataMap) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  auto it = this->dataMap.find(partition);
  if (it == this->dataMap.end()) {
    return RC_PARTITION_NOT_FOUND;
  }

  dataMap = it->second;

  return RC_SUCCESS;
}

uint32_t PartitionedStore::getDataPartition(const StoreKey<google::protobuf::Message>& key, StoreOption* status) {
  if (status) {
    return  status->partitionId;
  } else {
    if (getStoreConfig()) {
      StoreOption ps;
      getStoreConfig()->calculatePartitionInfo(const_cast<StoreKey<google::protobuf::Message>&>(key), &ps);
      return ps.partitionId;
    } else {
      LOG(WARNING) << "store config is NULL";
      uint32_t partSize = idgs_application()->getClusterFramework()->getPartitionCount();
      hashcode_t hashcode = protobuf::HashCode::hashcode(key.get());
      return hashcode % partSize;
    }
  }
}

void PartitionedStore::addMigrationActor(idgs::actor::Actor* actor) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  migrationActors.push_back(actor);
}

void PartitionedStore::removeMigrationActor(idgs::actor::Actor* actor) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);

  auto it = find(migrationActors.begin(), migrationActors.end(), actor);
  if (it != migrationActors.end()) {
    migrationActors.erase(it);
  }
}

std::vector<idgs::actor::Actor*> PartitionedStore::getMigrationActors() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  return migrationActors;
}

} // namespace store
} // namespace idgs
