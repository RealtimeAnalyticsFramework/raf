
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "data_store.h"

#include "config_parser.h"
#include "data_store_actor.h"
#include "data_sync_actor.h"
#include "aggregator_actor.h"
#include "idgs/actor/actor_descriptor_mgr.h"
#include "idgs/store/store_listener_factory.h"
#include "idgs/store/listener_manager.h"

using namespace google::protobuf;
using namespace idgs::actor;

namespace idgs {
namespace store {

// class DataStore
DataStore::DataStore() {
  isInited = false;
}

DataStore::~DataStore() {
  function_footprint();
}

ResultCode DataStore::initialize(const string& configFilePath) {
  ResultCode status;
  status = registerActor();
  if (status != RC_SUCCESS) {
    LOG(ERROR)<< "Failed to register listener. Error code : " << status << ", message : " << getErrorDescription(status) << ".";
    return status;
  }

  DVLOG(1) << "Load data store configuration: " << configFilePath;
  status = loadCfgFile(configFilePath);
  if (status != RC_SUCCESS) {
    LOG(ERROR)<< "Failed to load data store config. Error code : " << status << ", message : " << getErrorDescription(status) << ".";
    return status;
  }

  isInited = true;
  return RC_SUCCESS;
}

ResultCode DataStore::start() {
  ResultCode status;

  status = initializeDataStore();
  if (status != RC_SUCCESS) {
    DVLOG(2) << "Failed to initialize store of data. Error code : " << status << ", message : "
                << getErrorDescription(status) << ".";
    return status;
  }

  return RC_SUCCESS;
}

ResultCode DataStore::stop() {
  storeMap.clear();

  return RC_SUCCESS;
}

ResultCode DataStore::loadStoreConfig(const string& storeName, shared_ptr<StoreConfigWrapper>& storeConfigWrapper) {
  return getStoreConfigWrappers().getStoreConfig(storeName, storeConfigWrapper);
}

ResultCode DataStore::registerStoreConfig(const shared_ptr<StoreConfigWrapper>& storeConfigWrapper) {
  return getStoreConfigWrappers().addStoreConfig(storeConfigWrapper);
}

ResultCode DataStore::insertData(const string& storeName, const StoreKey<Message>& key, StoreValue<Message>& value, PartitionStatus* ps) {
  if (key.get() == NULL) {
    DVLOG(2) << "Failed. Error code : " << RC_INVALID_KEY << ", message : " << getErrorDescription(RC_INVALID_KEY);
    return RC_INVALID_KEY;
  }

  if (value.get() == NULL) {
    DVLOG(1) << "Failed. Error code : " << RC_INVALID_VALUE << ", message : " << getErrorDescription(RC_INVALID_VALUE);
    return RC_INVALID_VALUE;
  }
  auto it = storeMap.find(storeName);
  if (it == storeMap.end()) {
    return RC_STORE_NOT_FOUND;
  }

  if (!it->second) {
    return RC_STORE_NOT_FOUND;
  }

  return it->second->setData(key, value, ps);
}

ResultCode DataStore::getData(const string& storeName, const StoreKey<Message>& key, StoreValue<Message>& value, PartitionStatus* ps) {
  if (key.get() == NULL) {
    DVLOG(2) << "Failed. Error code : " << RC_INVALID_KEY << ", message : " << getErrorDescription(RC_INVALID_KEY);
    return RC_INVALID_KEY;
  }

  auto it = storeMap.find(storeName);
  if (it == storeMap.end()) {
    return RC_STORE_NOT_FOUND;
  }

  if (!it->second) {
    return RC_STORE_NOT_FOUND;
  }

  return it->second->getData(key, value, ps);
}

ResultCode DataStore::updateData(const string& storeName, const StoreKey<Message>& key, StoreValue<Message>& value, PartitionStatus* ps) {
  if (key.get() == NULL) {
    DVLOG(2) << "Failed. Error code : " << RC_INVALID_KEY << ", message : " << getErrorDescription(RC_INVALID_KEY);
    return RC_INVALID_KEY;
  }

  if (value.get() == NULL) {
    DVLOG(2) << "Failed. Error code : " << RC_INVALID_VALUE << ", message : " << getErrorDescription(RC_INVALID_VALUE);
    return RC_INVALID_VALUE;
  }

  auto it = storeMap.find(storeName);
  if (it == storeMap.end()) {
    return RC_STORE_NOT_FOUND;
  }

  if (!it->second) {
    return RC_STORE_NOT_FOUND;
  }

  return it->second->setData(key, value, ps);
}

ResultCode DataStore::removeData(const string& storeName, const StoreKey<Message>& key,
    StoreValue<google::protobuf::Message>& value, PartitionStatus* ps) {
  if (key.get() == NULL) {
    DVLOG(2) << "Failed. Error code : " << RC_INVALID_KEY << ", message : " << getErrorDescription(RC_INVALID_KEY);
    return RC_INVALID_KEY;
  }
  auto it = storeMap.find(storeName);
  if (it == storeMap.end()) {
    return RC_STORE_NOT_FOUND;
  }

  if (!it->second) {
    return RC_STORE_NOT_FOUND;
  }

  return it->second->removeData(key, value, ps);
}

ResultCode DataStore::syncStore(const string& storeName, shared_ptr<idgs::store::pb::SyncStore>& store) {
  shared_ptr<StoreConfigWrapper> storeConfigWrapper;
  ResultCode code = storeConfigWrappers.getStoreConfig(storeName, storeConfigWrapper);
  if (code != RC_SUCCESS) {
    return code;
  }

  auto it = storeMap.find(storeName);
  if (it == storeMap.end()) {
    return RC_STORE_NOT_FOUND;
  }

  if (!it->second) {
    return RC_STORE_NOT_FOUND;
  }

  switch (storeConfigWrapper->getStoreConfig().partition_type()) {
  case idgs::store::pb::REPLICATED: {
    ReplicatedStore* repStore = dynamic_cast<ReplicatedStore*>(it->second.get());
    repStore->syncData(store);
    break;
  }
  default: {
    break;
  }
  }

  return RC_SUCCESS;

}

ResultCode DataStore::storeSize(const string& storeName, size_t& size) {
  auto it = storeMap.find(storeName);
  if (it == storeMap.end()) {
    return RC_STORE_NOT_FOUND;
  }

  if (!it->second) {
    return RC_STORE_NOT_FOUND;
  }

  size = it->second->storeSize(size);
  return RC_OK;
}

ResultCode DataStore::storeSize(const std::string& storeName, const uint32_t& partition, size_t& size) {
  auto it = storeMap.find(storeName);
  if (it == storeMap.end()) {
    return RC_STORE_NOT_FOUND;
  }

  if (!it->second) {
    return RC_STORE_NOT_FOUND;
  }

  PartitionStore* partStore = dynamic_cast<PartitionStore*>(it->second.get());

  size = partStore->storeSize(partition, size);

  return RC_OK;
}

ResultCode DataStore::loadCfgFile(const string& configFilePath) {
  return StoreConfigParser().parseStoreConfig(configFilePath, getStoreConfigWrappers());
}

ResultCode DataStore::initializeDataStore() {
  shared_ptr<StoreConfigWrapper> storeConfigWrapper;
  vector<string> storeNames = storeConfigWrappers.getStoreNames();
  for (size_t i = 0; i < storeNames.size(); i++) {
    storeConfigWrappers.getStoreConfig(storeNames[i], storeConfigWrapper);

    switch (storeConfigWrapper->getStoreConfig().partition_type()) {
    case idgs::store::pb::PARTITION_TABLE: {
      break;
    }
    case idgs::store::pb::REPLICATED: {
      auto it = storeMap.find(storeNames[i]);
      if (it == storeMap.end()) {
        storeMap[storeNames[i]].reset(new ReplicatedStore);
        storeMap[storeNames[i]]->setStoreConfigWrapper(storeConfigWrapper);
        storeMap[storeNames[i]]->initialize();
      }

      break;
    }
    case idgs::store::pb::CONSISTENCE_HASH: {
      return RC_NOT_SUPPORT;
    }
    case idgs::store::pb::CUSTOM: {
      return RC_NOT_SUPPORT;
    }
    default: {
      return RC_NOT_SUPPORT;
    }
    }
  }

  return RC_SUCCESS;

}

ResultCode DataStore::scanPartitionData(const string& storeName, const uint32_t& partition,
    std::shared_ptr<StoreMap>& dataMap) {

  shared_ptr<StoreConfigWrapper> storeConfigWrapper;
  ResultCode code = storeConfigWrappers.getStoreConfig(storeName, storeConfigWrapper);
  if (code != RC_SUCCESS) {
    return code;
  }

  auto it = storeMap.find(storeName);
  if (it == storeMap.end()) {
    return RC_STORE_NOT_FOUND;
  }

  switch (storeConfigWrapper->getStoreConfig().partition_type()) {
  case idgs::store::pb::PARTITION_TABLE: {
    PartitionStore* store = dynamic_cast<PartitionStore*>(it->second.get());
    store->scanPartitionData(partition, dataMap);
    break;
  }
  case idgs::store::pb::REPLICATED: {
    ReplicatedStore* store = dynamic_cast<ReplicatedStore*>(it->second.get());
    store->scanData(dataMap);
    break;
  }
  default: {
    break;
  }
  }

  return RC_SUCCESS;

}

ResultCode DataStore::migrateData(const uint32_t& partition, const int32_t& localMemberId, const int32_t& curMemberId,
    const int32_t& newMemberId) {
  shared_ptr<StoreConfigWrapper> storeConfigWrapper;
  vector<string> storeNames = storeConfigWrappers.getStoreNames();
  for (size_t i = 0; i < storeNames.size(); i++) {
    storeConfigWrappers.getStoreConfig(storeNames[i], storeConfigWrapper);
    switch (storeConfigWrapper->getStoreConfig().partition_type()) {
    case idgs::store::pb::PARTITION_TABLE: {
      auto it = storeMap.find(storeNames[i]);
      if (it == storeMap.end()) {
        storeMap[storeNames[i]].reset(new PartitionStore);
        storeMap[storeNames[i]]->setStoreConfigWrapper(storeConfigWrapper);
      }

      PartitionStore* partStore = dynamic_cast<PartitionStore*>(storeMap[storeNames[i]].get());
      partStore->migrateData(partition, localMemberId, curMemberId, newMemberId);
      break;
    }
    default: {
      break;
    }
    }
  }

  return RC_SUCCESS;

}

ResultCode DataStore::clearData(const std::string& storeName, const uint32_t& partition) {
  auto it = storeMap.find(storeName);
  if (it == storeMap.end()) {
    return RC_STORE_NOT_FOUND;
  }

  shared_ptr<StoreConfigWrapper> storeConfigWrapper;
  ResultCode code = storeConfigWrappers.getStoreConfig(storeName, storeConfigWrapper);
  if (code == RC_SUCCESS) {
    switch (storeConfigWrapper->getStoreConfig().partition_type()) {
    case idgs::store::pb::PARTITION_TABLE: {
      PartitionStore* store = dynamic_cast<PartitionStore*>(it->second.get());
      code = store->clearData(partition);
      break;
    }
    case idgs::store::pb::REPLICATED: {
      ReplicatedStore* store = dynamic_cast<ReplicatedStore*>(it->second.get());
      code = store->clearData();
      break;
    }
    default: {
      code = RC_NOT_SUPPORT;
      break;
    }
    }
  }

  return code;
}

ResultCode DataStore::registerActor() {
  ActorFramework* actorFramework = ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework();

  // register stateless actor and its descriptor
  StoreServiceActor* storeActor = new StoreServiceActor(ACTORID_STORE_SERVCIE);
  actorFramework->Register(ACTORID_STORE_SERVCIE, storeActor);

  StoreSyncStatelessActor* syncActor = new StoreSyncStatelessActor(DATA_STORE_SYNC_ACTOR);
  actorFramework->Register(DATA_STORE_SYNC_ACTOR, syncActor);

  ListenerManager* listenerManager = new ListenerManager(LISTENER_MANAGER);
  actorFramework->Register(LISTENER_MANAGER, listenerManager);

  // register actor descriptor
  shared_ptr<ModuleDescriptorWrapper> module_descriptor(new ModuleDescriptorWrapper);
  module_descriptor->setName(STORE_MODULE_DESCRIPTOR_NAME);
  module_descriptor->setDescription(STORE_MODULE_DESCRIPTOR_DESCRIPTION);
  module_descriptor->addActorDescriptor(ReplicatedStoreSyncStatefulActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(DataAggregatorActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(DataSizeAggregatorActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(StoreServiceActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(StoreSyncStatelessActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(ListenerManager::generateActorDescriptor());
  ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().registerModuleDescriptor(
      module_descriptor->getName(), module_descriptor);

  return RC_SUCCESS;
}

std::vector<std::string> DataStore::getAllStoreNames() {
  return getStoreConfigWrappers().getStoreNames();
}

bool DataStore::isInit() {
  return isInited;
}

std::shared_ptr<Store>& DataStore::getStore(const std::string& name) {
  static std::shared_ptr<Store> nullStore;
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  auto it = storeMap.find(name);
  if (it == storeMap.end()) {
    return nullStore;
  }

  return it->second;
}

ResultCode DataStore::registerStoreListener(StoreListener* listener) {
  if (!listener) {
    return RC_INVALID_STORE_LISTENER;
  }

  StoreListenerFactory::registerStoreListener(listener);
  return RC_SUCCESS;
}

}//namespace store
} // namespace idgs
