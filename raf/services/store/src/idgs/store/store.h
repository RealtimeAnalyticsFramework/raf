
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/actor/actor.h"

#include "idgs/store/data_map.h"
#include "idgs/store/store_config.h"

namespace idgs {
namespace store {


/// Store interface. <br>
/// JSR107 JCache API
/// @code
/// boolean containsKey(K key)
/// V get(K key)
/// V getAndPut(K key, V value)
/// V getAndRemove(K key)
/// V getAndReplace(K key, V value)
///
/// void put(K key, V value)
/// boolean putIfAbsent(K key, V value)
///
/// boolean remove(K key)
/// boolean remove(K key, V oldValue)
/// void removeAll()
///
/// boolean replace(K key, V value)
/// boolean replace(K key, V oldValue, V newValue)
/// @endcode
class Store {
public:

  virtual ~Store() {}
  virtual ResultCode initialize() = 0;


  /// @brief  Get data by key from the store.
  /// @param  key The key of data.
  /// @param  value Return value of data.
  /// @return Status code of result.
  virtual ResultCode get(const StoreKey<google::protobuf::Message>& key,
      StoreValue<google::protobuf::Message>& value, StoreOption* status = NULL) = 0;

  /// @brief  Set data to the store.
  /// @param  key The key of data to store.
  /// @param  value The value of data to store.
  /// @return Status code of result.
  virtual ResultCode put(const StoreKey<google::protobuf::Message>& key,
      StoreValue<google::protobuf::Message>& value, StoreOption* status = NULL) = 0;

  /// @brief  Remove data by key from the store.
  /// @param  key The key of data.
  /// @param  value Return value of the removed data.
  /// @return Status code of result.
  virtual ResultCode remove(const StoreKey<google::protobuf::Message>& key,
      StoreValue<google::protobuf::Message>& value, StoreOption* status = NULL) = 0;

  /// @brief  Clear the data of store.
  virtual void removeAll() = 0;

  /// @brief  Get the data size of store.
  /// @param  size The size of store.
  /// @return Status code of result.
  virtual size_t size() = 0;

  /// @brief  Get the wrapper of store config of the store.
  /// @return The wrapper of store config.
  const std::shared_ptr<StoreConfig>& getStoreConfig() {
    return storeConfig;
  }

  /// @brief  Put the wrapper of store config.
  /// @param  storeConfig_ The wrapper of store config.
  void setStoreConfig(const std::shared_ptr<StoreConfig>& storeConfig_) {
    storeConfig = storeConfig_;
  }

protected:
  std::shared_ptr<StoreConfig> storeConfig;
};

typedef std::shared_ptr<Store> StorePtr;

} // namespace store
} // namespace idgs
