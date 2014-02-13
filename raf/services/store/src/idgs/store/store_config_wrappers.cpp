
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "store_config_wrappers.h"

using namespace std;

namespace idgs {
namespace store {

StoreConfigWrappers::StoreConfigWrappers() {
}

StoreConfigWrappers::~StoreConfigWrappers() {
}

// class StoreConfigWrappers
ResultCode StoreConfigWrappers::getStoreConfig(const string& storeName,
    shared_ptr<StoreConfigWrapper>& storeConfigWrapper) {
  if (storeConfigMap.find(storeName) == storeConfigMap.end()) {
    return RC_STORE_NOT_FOUND;
  }

  storeConfigWrapper = storeConfigMap[storeName];
  return RC_SUCCESS;
}

ResultCode StoreConfigWrappers::addStoreConfig(std::shared_ptr<StoreConfigWrapper> storeConfigWrapper) {
  storeConfigMap[storeConfigWrapper->getStoreConfig().name()] = storeConfigWrapper;
  return RC_SUCCESS;
}

vector<string> StoreConfigWrappers::getStoreNames() {
  vector<string> storeNames;
  storeNames.resize(getStoreSize());
  int32_t index = 0;
  auto it = storeConfigMap.begin();
  for (; it != storeConfigMap.end(); ++it) {
    storeNames[index++] = (it->first);
  }

  return storeNames;
}

size_t StoreConfigWrappers::getStoreSize() {
  return storeConfigMap.size();
}

// class StoreConfigWrappers

}// namespace store
} // namespace idgs
