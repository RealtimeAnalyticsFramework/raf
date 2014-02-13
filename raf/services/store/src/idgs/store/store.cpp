
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/store/store.h"

#include "idgs/cluster/cluster_framework.h"

using namespace idgs::cluster;
using namespace google::protobuf;
using namespace protobuf;

namespace idgs {
namespace store {

/// class PartitionStore
/// @fixme one member only own part of partitions.
ResultCode PartitionStore::initialize() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  int32_t part_size = getPartitionSize();
  for (int32_t i = 0; i < part_size; i++) {
    auto it = dataMap.find(i);
    if (it == dataMap.end()) {
      if (getStoreConfigWrapper()->getStoreConfig().store_type() == pb::ORDERED) {
        dataMap[i].reset(new TreeMap);
      } else if (getStoreConfigWrapper()->getStoreConfig().store_type() == pb::UNORDERED) {
        dataMap[i].reset(new HashMap);
      }
    }
  }

  return RC_SUCCESS;
}

/// @todo refactor
ResultCode PartitionStore::migrateData(const uint32_t& partition, const int32_t& localMemberId,
    const int32_t& curMemberId, const int32_t& newMemberId) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  if (localMemberId == newMemberId) {
    auto it = dataMap.find(partition);
    if (it == dataMap.end()) {
      if (getStoreConfigWrapper()->getStoreConfig().store_type() == idgs::store::pb::ORDERED) {
        dataMap[partition].reset(new TreeMap);
      } else if (getStoreConfigWrapper()->getStoreConfig().store_type() == idgs::store::pb::UNORDERED) {
        dataMap[partition].reset(new HashMap);
      }
    }
  } else if (localMemberId == curMemberId) {
    dataMap.erase(partition);
  }

  return RC_SUCCESS;
}

/// @todo partitionExist
ResultCode PartitionStore::getData(const StoreKey<Message>& key, StoreValue<Message>& value, PartitionStatus* status) {
  std::shared_ptr<StoreMap> map;
  int32_t partition = getDataPartition(key, status);
  bool partitionExist = false;
  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
    DVLOG(1) << "partititon store insert data";

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

ResultCode PartitionStore::setData(const StoreKey<Message>& key, StoreValue<Message>& value, PartitionStatus* status) {
  std::shared_ptr<StoreMap> map;
  int32_t partition = getDataPartition(key, status);
  bool partitionExist = false;
  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
    DVLOG_EVERY_N(1, 100) << "partititon store insert data";

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

ResultCode PartitionStore::removeData(const StoreKey<Message>& key, StoreValue<Message>& value, PartitionStatus* status) {
  std::shared_ptr<StoreMap> map;
  int32_t partition = getDataPartition(key, status);
  bool partitionExist = false;
  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
    DVLOG(1) << "partititon store insert data";

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

ResultCode PartitionStore::storeSize(size_t& size) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  size = 0;
  int32_t partSize = getPartitionSize();
  for (int32_t i = 0; i < partSize; i++) {
    auto it = dataMap.find(i);
    if (it != dataMap.end()) {
      size += it->second->size();
    }
  }
  return RC_SUCCESS;
}

ResultCode PartitionStore::storeSize(const uint32_t& partition, size_t& size) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  auto it = dataMap.find(partition);
  if (it == dataMap.end()) {
    return RC_PARTITION_NOT_FOUND;
  }

  size = it->second->size();
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

size_t PartitionStore::getPartitionSize() {
  return ::idgs::util::singleton<ClusterFramework>::getInstance().getPartitionCount();
}

uint32_t PartitionStore::getDataPartition(const StoreKey<Message>& key, PartitionStatus* status) {
  if (status) {
    return  status->partitionId;
  } else {
    if (getStoreConfigWrapper()) {
      PartitionStatus ps;
      getStoreConfigWrapper()->calculatePartitionStatus(const_cast<StoreKey<Message>&>(key), &ps);
      return ps.partitionId;
    } else {
      LOG(WARNING) << "store config is NULL";
      uint32_t partSize = ::idgs::util::singleton<ClusterFramework>::getInstance().getPartitionCount();
      hashcode_t hashcode = HashCode::hashcode(key.get());
      return hashcode % partSize;
    }
  }
}

ResultCode PartitionStore::clearData(const uint32_t& partition) {
  DVLOG(1) << "clear distributed partition: " << partition;
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  auto it = dataMap.find(partition);
  if (it == dataMap.end()) {
    return RC_PARTITION_NOT_FOUND;
  }

  return it->second->clear();
}
// class PartitionStore

// class ReplicatedStore
ResultCode ReplicatedStore::initialize() {
  if (getStoreConfigWrapper()->getStoreConfig().store_type() == pb::ORDERED) {
    dataMap.reset(new TreeMap);
  } else if (getStoreConfigWrapper()->getStoreConfig().store_type() == pb::UNORDERED) {
    dataMap.reset(new HashMap);
  }

  return RC_SUCCESS;
}

ResultCode ReplicatedStore::getData(const StoreKey<Message>& key, StoreValue<Message>& value, PartitionStatus* status) {
  return dataMap->get(key, value);
}

ResultCode ReplicatedStore::setData(const StoreKey<Message>& key, StoreValue<Message>& value, PartitionStatus* status) {
  return dataMap->set(key, value);
}

ResultCode ReplicatedStore::removeData(const StoreKey<Message>& key, StoreValue<Message>& value, PartitionStatus* status) {
  return dataMap->remove(key, value);
}

ResultCode ReplicatedStore::syncData(shared_ptr<idgs::store::pb::SyncStore>& store) {
  return dataMap->scan(store);
}

ResultCode ReplicatedStore::storeSize(size_t& size) {
  size = dataMap->size();
  return RC_SUCCESS;
}

ResultCode ReplicatedStore::clearData() {
  DVLOG(1) << "clear replicated store";
  dataMap->clear();
  return RC_SUCCESS;
}

ResultCode ReplicatedStore::scanData(std::shared_ptr<StoreMap>& dataMap) {
  dataMap = this->dataMap;
  return RC_SUCCESS;
}

// class ReplicatedStore


}// namespace store
} // namespace idgs
