
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/store/store.h"

namespace idgs {
namespace store {

/// Wrapper class. <br>
/// Wrap all store config wrappers. <br>
class StoreConfigWrappers {
public:

  /// @brief Constructor
  StoreConfigWrappers();

  /// @brief Destructor
  ~StoreConfigWrappers();

  /// Copy constructor, called by containers.
  /// This should be singleton, copy constructor should be deleted.
  StoreConfigWrappers(const StoreConfigWrappers& other) = delete;
  StoreConfigWrappers(StoreConfigWrappers&& other) = delete;
  StoreConfigWrappers& operator()(const StoreConfigWrappers& other) = delete;
  StoreConfigWrappers& operator()(StoreConfigWrappers&& other) = delete;

  /// @brief  Get the wrapper of store config with store name.
  /// @param  storeName The name of store.
  /// @param  storeConfigWrapper The return value of the wrapper of store config.
  /// @return Status code of result.
  ResultCode getStoreConfig(const std::string& storeName, std::shared_ptr<StoreConfigWrapper>& storeConfigWrapper);

  /// @brief  Add the wrapper of store config.
  /// @param  storeConfigWrapper The wrapper of store config with protobuf type.
  /// @return Status code of result.
  ResultCode addStoreConfig(std::shared_ptr<StoreConfigWrapper> storeConfigWrapper);

  /// @brief  Get the array of store names.
  /// @return The vector of store names.
  std::vector<std::string> getStoreNames();

  /// @brief  Get the size of stores.
  /// @return The size of stores.
  size_t getStoreSize();
private:
  std::map<std::string, std::shared_ptr<StoreConfigWrapper>> storeConfigMap;

};
// class StoreConfigWrappers

}// namespace store
} // namespace idgs
