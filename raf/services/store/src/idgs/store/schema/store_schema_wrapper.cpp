
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "store_schema_wrapper.h"


namespace idgs {
namespace store {

StoreSchemaWrapper::StoreSchemaWrapper(const std::string& schemaName) : name(schemaName) {
}

StoreSchemaWrapper::~StoreSchemaWrapper() {
}

protobuf::MessageHelper& StoreSchemaWrapper::getMessageHelper() {
  return pbMessageHelper;
}

ResultCode StoreSchemaWrapper::addStore(const StoreConfigWrapperPtr& storeConfigWrapper) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  auto config = storeConfigWrapper->getStoreConfig();
  auto& storeName = config.name();
  auto it = storeMap.find(storeName);
  if (it != storeMap.end()) {
    LOG(ERROR) << "schema " << name << " store " << storeName << " is already exists.";
    return RC_STORE_EXISTED;
  }

  auto type = config.partition_type();
  StorePtr store;
  switch (type) {
    case idgs::store::pb::PARTITION_TABLE: {
      store = std::make_shared<PartitionStore>();
      break;
    }
    case idgs::store::pb::REPLICATED: {
      store = std::make_shared<ReplicatedStore>();
      break;
    }
    default: {
      return RC_NOT_SUPPORT;
    }
  }

  store->setStoreConfigWrapper(storeConfigWrapper);
  storeMap.insert(std::pair<std::string, StorePtr>(storeName, store));

  return RC_SUCCESS;
}

ResultCode StoreSchemaWrapper::removeStore(const std::string& storeName) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  auto it = storeMap.find(storeName);
  if (it == storeMap.end()) {
    return RC_STORE_NOT_FOUND;
  }

  storeMap.erase(it);
  return RC_SUCCESS;
}

StorePtr StoreSchemaWrapper::getStore(const std::string& storeName) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  auto it = storeMap.find(storeName);
  if (it == storeMap.end()) {
    return StorePtr();
  }

  return it->second;
}

const std::vector<StorePtr> StoreSchemaWrapper::getStores() const {
  std::vector<StorePtr> stores;
  auto it = storeMap.begin();
  for (; it != storeMap.end(); ++ it) {
    stores.push_back(it->second);
  }

  return stores;
}

void StoreSchemaWrapper::drop() {
  storeMap.clear();
}

} // namespace store
} // namespace idgs
