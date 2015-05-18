/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "base_store.h"

namespace idgs {
namespace store {

BaseStore::BaseStore() {
}

BaseStore::~BaseStore() {
}

// class BaseStore
ResultCode BaseStore::initialize() {
  if (getStoreConfig()->getStoreConfig().store_type() == pb::ORDERED) {
    dataMap = std::make_shared<TreeMap>();
  } else if (getStoreConfig()->getStoreConfig().store_type() == pb::UNORDERED) {
    dataMap = std::make_shared<HashMap>();
  }

  return RC_SUCCESS;
}

ResultCode BaseStore::get(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, StoreOption* status) {
  return dataMap->get(key, value);
}

ResultCode BaseStore::put(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, StoreOption* status) {
  return dataMap->set(key, value);
}

ResultCode BaseStore::remove(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, StoreOption* status) {
  return dataMap->remove(key, value);
}

size_t BaseStore::size() {
  return dataMap->size();
}

void BaseStore::removeAll() {
  DVLOG(1) << "clear replicated store";
  dataMap->clear();
}

ResultCode BaseStore::scanData(std::shared_ptr<StoreMap>& dataMap) {
  dataMap = this->dataMap;
  return RC_SUCCESS;
}

ResultCode BaseStore::snapshotStore(std::shared_ptr<StoreMap>& map) {
  auto temp = dataMap->snapshot();
  map.swap(temp);
  return RC_SUCCESS;
}

void BaseStore::addSyncActor(idgs::actor::Actor* actor) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  syncActors.push_back(actor);
}

void BaseStore::removeSyncActor(idgs::actor::Actor* actor) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  auto it = find(syncActors.begin(), syncActors.end(), actor);
  if (it != syncActors.end()) {
    syncActors.erase(it);
  }

}

std::vector<idgs::actor::Actor*> BaseStore::getSyncActors() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  return syncActors;
}

} // namespace store
} // namespace idgs
