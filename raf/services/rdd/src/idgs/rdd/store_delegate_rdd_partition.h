
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <memory>
#include "base_rdd_partition.h"
#include "idgs/actor/actor_descriptor.h"
#include "idgs/actor/rpc_framework.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/store/data_map.h"

namespace idgs {
namespace rdd {

typedef std::shared_ptr<idgs::store::StoreMap> StoreDataMap;

/// The stateful actor of store delegate RDD partition.
/// The partition actor of store delegate RDD.
class StoreDelegateRddPartition: public BaseRddPartition {
public:

  /// @brief Constructor
  /// @param partitionId  Partition of store.
  /// @param storename    The name of store.
  StoreDelegateRddPartition(const uint32_t& partitionId, const std::string& storename);

  /// @brief Destructor
  virtual ~StoreDelegateRddPartition();

  /// @brief  Generate descriptor for StoreDelegateRddPartition.
  /// @return The descriptor for StoreDelegateRddPartition.
  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

  /// @brief  Get the descriptor for StoreDelegateRddPartition.
  /// @return The descriptor for StoreDelegateRddPartition.
  const idgs::actor::ActorDescriptorPtr& getDescriptor() const override;

  const std::string& getActorName() const {
    static std::string actorName = STORE_DELEGATE_RDD_PARTITION;
    return actorName;
  }

  /// @brief  Get the data for store delegate RDD.
  /// @return The data for store delegate RDD.
  const StoreDataMap& getData() const;

  /// @brief  Get the value of data by key.
  /// @param  key Key of data.
  /// @return The value of data.
  virtual idgs::actor::PbMessagePtr get(const idgs::actor::PbMessagePtr& key) const override;

  /// @brief  Get the value of data by key.
  /// @param  key Key of data.
  /// @return The value of data.
  virtual std::vector<idgs::actor::PbMessagePtr> getValue(const idgs::actor::PbMessagePtr& key) const override;

  /// @brief  Whether key is in RDD data.
  /// @param  key Key of data.
  /// @return True or false.
  virtual bool containKey(const idgs::actor::PbMessagePtr& key) const override;

  /// @brief  Put data a pair key and value to RDD, it is maybe repartition.
  /// @param  key   Key of data.
  /// @param  value Value of data.
  virtual void put(const idgs::actor::PbMessagePtr& key, std::vector<idgs::actor::PbMessagePtr>& value) override;
  virtual void put(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) override;

  /// @brief  Put data a pair key and value to local RDD.
  /// @param  key   Key of data.
  /// @param  value Value of data.
  virtual void putLocal(const idgs::actor::PbMessagePtr& key, std::vector<idgs::actor::PbMessagePtr>& value) override;
  virtual void putLocal(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) override;

  /// @brief  Loop the data of RDD.
  /// @param  fn A function to loop data.
  virtual void foreach(RddEntryFunc fn) const override;

  /// @brief  Loop the data of RDD by key.
  /// @param  fn A function to loop data.
  virtual void foreachGroup(RddGroupEntryFunc fn) const override;

  /// @brief  Whether data of RDD is empty.
  /// @return True or false.
  virtual bool empty() const override;

  /// @brief  Get the data size of RDD.
  /// @return The data size of RDD.
  virtual size_t size() const override;

  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

private:
  static idgs::actor::ActorDescriptorPtr descriptor;
  std::string storeName;
  StoreDataMap dataMap;

  void handleProcess(const idgs::actor::ActorMessagePtr& msg);
  void handleAction(const idgs::actor::ActorMessagePtr& msg);
  void handleDestory(const idgs::actor::ActorMessagePtr& msg);
};

} // namespace rdd
} // namespace idgs 
