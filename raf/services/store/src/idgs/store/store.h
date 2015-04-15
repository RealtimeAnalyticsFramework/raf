
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/actor/actor.h"

#include "idgs/store/data_map.h"
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
      StoreValue<google::protobuf::Message>& value, PartitionInfo* status = NULL) = 0;

  /// @brief  Set data to the store.
  /// @param  key The key of data to store.
  /// @param  value The value of data to store.
  /// @return Status code of result.
  virtual ResultCode setData(const StoreKey<google::protobuf::Message>& key,
      StoreValue<google::protobuf::Message>& value, PartitionInfo* status = NULL) = 0;

  /// @brief  Remove data by key from the store.
  /// @param  key The key of data.
  /// @param  value Return value of the removed data.
  /// @return Status code of result.
  virtual ResultCode removeData(const StoreKey<google::protobuf::Message>& key,
      StoreValue<google::protobuf::Message>& value, PartitionInfo* status = NULL) = 0;

  /// @brief  Clear the data of store.
  virtual void clearData() = 0;

  /// @brief  Get the data size of store.
  /// @param  size The size of store.
  /// @return Status code of result.
  virtual size_t dataSize() = 0;

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
  virtual ResultCode initialize() override;

  /// @brief  Get data by key from the store.
  /// @param  key The key of data.
  /// @param  value Return value of data.
  /// @return Status code of result.
  virtual ResultCode getData(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, PartitionInfo* status = NULL) override;

  /// @brief  Set data to the store.
  /// @param  key The key of data to store.
  /// @param  value The value of data to store.
  /// @return Status code of result.
  virtual ResultCode setData(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, PartitionInfo* status = NULL) override;

  /// @brief  Remove data by key from the store.
  /// @param  key The key of data.
  /// @param  value Return value of the removed data.
  /// @return Status code of result.
  virtual ResultCode removeData(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, PartitionInfo* status = NULL) override;

  /// @brief  Clear the data of store.
  virtual void clearData() override;

  /// @brief  Get the data size of store.
  /// @param  size The size of the given store.
  /// @return Status code of result.
  virtual size_t dataSize() override;

  size_t dataSize(const uint32_t& partition);

  ResultCode clearData(const uint32_t& partition);

  /// @brief  Get snapshot store map, for data migration.
  /// @param  partition  Partition of store.
  /// @param  map        Snapshot store map.
  /// @return Status code of result.
  ResultCode snapshotStore(const int32_t& partition, std::shared_ptr<StoreMap>& map);

  /// @brief  Scan data with store name and partition.
  /// @param  partition  Parition of data.
  /// @param  dataMap    Returned data.
  /// @return Status code of result.
  ResultCode scanPartitionData(const uint32_t& partition, std::shared_ptr<StoreMap>& dataMap);

  void addMigrationActor(idgs::actor::Actor* actor);

  void removeMigrationActor(idgs::actor::Actor* actor);

  std::vector<idgs::actor::Actor*> getMigrationActors();

  std::set<std::string> getMigrationActorIds();

private:
  uint32_t getDataPartition(const StoreKey<google::protobuf::Message>& key, PartitionInfo* status);
  std::map<int32_t, std::shared_ptr<StoreMap>> dataMap;
  tbb::spin_rw_mutex mutex;

  std::vector<idgs::actor::Actor*> migrationActors;
  std::set<std::string> migrationActorIds;

};
// class PartitionStore

/// Store class. <br>
/// To store data with replicated type. <br>
class ReplicatedStore: public Store {
public:

  /// @brief  Initialize store with replicated type.
  /// @return Status code of result.
  virtual ResultCode initialize() override;

  /// @brief  Get data by key from the the store.
  /// @param  key The key of data.
  /// @param  value Return value of data.
  /// @return Status code of result.
  virtual ResultCode getData(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, PartitionInfo* status = NULL) override;

  /// @brief  Set data to the store.
  /// @param  key The key of data to store.
  /// @param  value The value of data to store.
  /// @return Status code of result.
  virtual ResultCode setData(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, PartitionInfo* status = NULL) override;

  /// @brief  Remove data by key from the store.
  /// @param  key The key of data.
  /// @param  value Return value of the removed data.
  /// @return Status code of result.
  virtual ResultCode removeData(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, PartitionInfo* status = NULL) override;

  /// @brief  Clear the data of store.
  virtual void clearData() override;

  /// @brief  Get the data size of the given store.
  /// @param  size The size of the given store.
  /// @return Status code of result.
  virtual size_t dataSize();

  /// @brief  Load the data from store.
  /// @param  store The protobuf of sync store data.
  /// @return Status code of result.
  ResultCode scanData(std::shared_ptr<StoreMap>& dataMap);

  /// @brief  Get snapshot store map, for data migration.
  /// @param  partition  Partition of store.
  /// @param  map        Snapshot store map.
  /// @return Status code of result.
  ResultCode snapshotStore(std::shared_ptr<StoreMap>& map);

  void addSyncActor(idgs::actor::Actor* actor);

  void removeSyncActor(idgs::actor::Actor* actor);

  std::vector<idgs::actor::Actor*> getSyncActors();

  std::set<std::string> getSyncActorIds();

private:
  std::shared_ptr<StoreMap> dataMap;

  tbb::spin_rw_mutex mutex;

  std::vector<idgs::actor::Actor*> syncActors;
  std::set<std::string> syncActorIds;

};
// class ReplicatedStore

typedef std::shared_ptr<Store> StorePtr;

}// namespace store
} // namespace idgs
