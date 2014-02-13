
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "data_map.h"
#include "idgs/store/store_config_wrapper.h"

namespace idgs {
namespace store {


/// Store interface. <br>
/// To store data with store partition type, such as partition table, replicated, consistence hash and usr custom. <br>
class Store {
public:

  virtual ~Store() {}
  virtual ResultCode initialize() = 0;

  /// @brief  Get data by key from the store.
  /// @param  key The key of data.
  /// @param  value Return value of data.
  /// @return Status code of result.
  virtual ResultCode getData(const StoreKey<google::protobuf::Message>& key,
      StoreValue<google::protobuf::Message>& value, PartitionStatus* status = NULL) = 0;

  /// @brief  Set data to the store.
  /// @param  key The key of data to store.
  /// @param  value The value of data to store.
  /// @return Status code of result.
  virtual ResultCode setData(const StoreKey<google::protobuf::Message>& key,
      StoreValue<google::protobuf::Message>& value, PartitionStatus* status = NULL) = 0;

  /// @brief  Remove data by key from the store.
  /// @param  key The key of data.
  /// @param  value Return value of the removed data.
  /// @return Status code of result.
  virtual ResultCode removeData(const StoreKey<google::protobuf::Message>& key,
      StoreValue<google::protobuf::Message>& value, PartitionStatus* status = NULL) = 0;

  /// @brief  Get the data size of the given store.
  /// @param  size The size of the given store.
  /// @return Status code of result.
  virtual ResultCode storeSize(size_t& size) = 0;

  /// @brief  Get the wrapper of store config of the store.
  /// @return The wrapper of store config.
  const std::shared_ptr<StoreConfigWrapper>& getStoreConfigWrapper() {
    return storeConfigWrapper;
  }

  /// @brief  Put the wrapper of store config.
  /// @param  storeConfigWrapper The wrapper of store config.
  void setStoreConfigWrapper(const std::shared_ptr<StoreConfigWrapper>& storeConfigWrapper_) {
    storeConfigWrapper = storeConfigWrapper_;
  }

protected:
  // To store wrapper of store config.
  std::shared_ptr<StoreConfigWrapper> storeConfigWrapper;
};
// class Store

/// Store class. <br>
/// To store data with partition table type. <br>
class PartitionStore: public Store {
public:

  /// @brief  Initialize store with partition table type.
  /// @return Status code of result.
  ResultCode initialize();

  /// @brief  Get data by key from the store.
  /// @param  key The key of data.
  /// @param  value Return value of data.
  /// @return Status code of result.
  ResultCode getData(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, PartitionStatus* status = NULL);

  /// @brief  Set data to the store.
  /// @param  key The key of data to store.
  /// @param  value The value of data to store.
  /// @return Status code of result.
  ResultCode setData(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, PartitionStatus* status = NULL);

  /// @brief  Remove data by key from the store.
  /// @param  key The key of data.
  /// @param  value Return value of the removed data.
  /// @return Status code of result.
  ResultCode removeData(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, PartitionStatus* status = NULL);

  /// @brief  Get the data size of the given store.
  /// @param  size The size of the given store.
  /// @return Status code of result.
  ResultCode storeSize(size_t& size);

  /// @brief  Get the data size of the given store.
  /// @param  partition The partition of store.
  /// @param  size The size of the given store.
  /// @return Status code of result.
  ResultCode storeSize(const uint32_t& partition, size_t& size);

  /// @brief  When parititon changed, rebalnace data of store.
  /// @param  partition  Witch partition changed.
  /// @param  localMemberId  Member id of local.
  /// @param  curMemberId  Member id before changed.
  /// @param  newMemberId  Member id after changed.
  /// @return Status code of result.
  ResultCode migrateData(const uint32_t& partition, const int32_t& localMemberId, const int32_t& curMemberId,
      const int32_t& newMemberId);

  /// @brief  Scan data with store name and partition.
  /// @param  partition  Parition of data.
  /// @param  dataMap    Returned data.
  /// @return Status code of result.
  ResultCode scanPartitionData(const uint32_t& partition, std::shared_ptr<StoreMap>& dataMap);

  /// @brief  Clear all data of given partition in memory.
  /// @param  partition  Parition of store.
  /// @return Status code of result.
  ResultCode clearData(const uint32_t& partition);

private:
  uint32_t getDataPartition(const StoreKey<google::protobuf::Message>& key, PartitionStatus* status);
  size_t getPartitionSize();
  std::map<int, std::shared_ptr<StoreMap>> dataMap;
  tbb::spin_rw_mutex mutex;
};
// class PartitionStore

/// Store class. <br>
/// To store data with replicated type. <br>
class ReplicatedStore: public Store {
public:

  /// @brief  Initialize store with replicated type.
  /// @return Status code of result.
  ResultCode initialize();

  /// @brief  Get data by key from the the store.
  /// @param  key The key of data.
  /// @param  value Return value of data.
  /// @return Status code of result.
  ResultCode getData(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, PartitionStatus* status = NULL);

  /// @brief  Set data to the store.
  /// @param  key The key of data to store.
  /// @param  value The value of data to store.
  /// @return Status code of result.
  ResultCode setData(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, PartitionStatus* status = NULL);

  /// @brief  Remove data by key from the store.
  /// @param  key The key of data.
  /// @param  value Return value of the removed data.
  /// @return Status code of result.
  ResultCode removeData(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, PartitionStatus* status = NULL);

  /// @brief  Get the data size of the given store.
  /// @param  size The size of the given store.
  /// @return Status code of result.
  ResultCode storeSize(size_t& size);

  /// @brief  Get the data of the given store.
  /// @param  store The protobuf of sync store data.
  /// @return Status code of result.
  ResultCode syncData(std::shared_ptr<idgs::store::pb::SyncStore>& store);

  /// @brief  Clear all data of store in memory.
  /// @return Status code of result.
  ResultCode clearData();

  ResultCode scanData(std::shared_ptr<StoreMap>& dataMap);

private:
  std::shared_ptr<StoreMap> dataMap;

};
// class ReplicatedStore


}// namespace store
} // namespace idgs
