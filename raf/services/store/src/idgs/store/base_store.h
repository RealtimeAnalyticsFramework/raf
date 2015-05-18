/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#pragma once

#include "store.h"

namespace idgs {
namespace store {

class BaseStore: public idgs::store::Store {
public:
  BaseStore();
  virtual ~BaseStore();

public:
  /// @brief  Initialize store with replicated type.
  /// @return Status code of result.
  virtual ResultCode initialize() override;

  /// @brief  Get data by key from the the store.
  /// @param  key The key of data.
  /// @param  value Return value of data.
  /// @return Status code of result.
  virtual ResultCode get(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, StoreOption* status = NULL) override;

  /// @brief  Set data to the store.
  /// @param  key The key of data to store.
  /// @param  value The value of data to store.
  /// @return Status code of result.
  virtual ResultCode put(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, StoreOption* status = NULL) override;

  /// @brief  Remove data by key from the store.
  /// @param  key The key of data.
  /// @param  value Return value of the removed data.
  /// @return Status code of result.
  virtual ResultCode remove(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value, StoreOption* status = NULL) override;

  /// @brief  Clear the data of store.
  virtual void removeAll() override;

  /// @brief  Get the data size of the given store.
  /// @param  size The size of the given store.
  /// @return Status code of result.
  virtual size_t size();

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

private:
  std::shared_ptr<StoreMap> dataMap;

  tbb::spin_rw_mutex mutex;

  std::vector<idgs::actor::Actor*> syncActors;
};

} // namespace store
} // namespace idgs
