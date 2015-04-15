
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "base_rdd_partition.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/store/store.h"

namespace idgs {
namespace rdd {

typedef std::shared_ptr<idgs::store::StoreMap> StoreDataMap;

/// The stateful actor of store delegate RDD partition.
/// The partition actor of store delegate RDD.
class PairStoreDelegateRddPartition: public BaseRddPartition {
public:

  /// @brief Constructor
  /// @param partitionId  Partition of store.
  /// @param storename    The name of store.
  PairStoreDelegateRddPartition(const std::string& rddname, const uint32_t& partitionId);

  /// @brief Destructor
  virtual ~PairStoreDelegateRddPartition();

  /// @brief  Generate descriptor for StoreDelegateRddPartition.
  /// @return The descriptor for StoreDelegateRddPartition.
  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

  /// @brief  Get the descriptor for StoreDelegateRddPartition.
  /// @return The descriptor for StoreDelegateRddPartition.
  const idgs::actor::ActorDescriptorPtr& getDescriptor() const override;

  const std::string& getActorName() const {
    static std::string actorName = PAIR_STORE_DELEGATE_RDD_PARTITION;
    return actorName;
  }

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

  /// @brief  Loop the data of RDD.
  /// @param  fn A function to loop data.
  virtual void foreach(RddEntryFunc fn) const override;

  virtual void foreachGroup(RddGroupEntryFunc fn) const override;

  /// @brief  Whether data of RDD is empty.
  /// @return True or false.
  virtual bool empty() const override;

  /// @brief  Get the value key data size of RDD.
  /// @return The data size of RDD.
  virtual size_t keySize() const override;

  /// @brief  Get the value data size of RDD.
  /// @return The data size of RDD.
  virtual size_t valueSize() const override;

  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

  void initPartitionStore(const idgs::store::StorePtr& store);

private:
  static idgs::actor::ActorDescriptorPtr descriptor;
  std::string schemaName;
  std::string storeName;
  StoreDataMap dataMap;

  /// when running first transform active is true
  bool active;

private:
  void handleRddTransform(const idgs::actor::ActorMessagePtr& msg);
  void handleActionRequest(const idgs::actor::ActorMessagePtr& msg);
  void handleRddStoreListener(const idgs::actor::ActorMessagePtr& msg);

};

} // namespace rdd
} // namespace idgs 
