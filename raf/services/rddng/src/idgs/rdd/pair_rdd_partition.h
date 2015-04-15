
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <unordered_map>


#include "idgs/rdd/base_rdd_partition.h"
#include "idgs/rdd/rdd_const.h"

#include "idgs/store/comparer.h"

namespace idgs {
namespace rdd {

typedef std::map<idgs::actor::PbMessagePtr, std::vector<idgs::actor::PbMessagePtr>, protobuf::sharedless> OrderedRddDataMap;
typedef std::unordered_map<idgs::actor::PbMessagePtr, std::vector<idgs::actor::PbMessagePtr>,
    protobuf::shared_hash_code, protobuf::sharedless> UnorderedRddDataMap;

/// The stateful actor of RDD partition.
/// The partition actor of RDDs.
class PairRddPartition: public BaseRddPartition {
public:

  /// @brief Constructor
  /// @param partitionId  Partition of store.
  PairRddPartition(const std::string& rddname, const uint32_t& partitionId);

  /// @brief Destructor
  virtual ~PairRddPartition();

  /// @brief  Generate descriptor for RddPartition.
  /// @return The descriptor for RddPartition.
  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

  /// @brief  Get the descriptor for RddPartition.
  /// @return The descriptor for RddPartition.
  const idgs::actor::ActorDescriptorPtr& getDescriptor() const override;

  const std::string& getActorName() const override {
    static std::string actorName = PAIR_RDD_PARTITION;
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

  /// @brief  Get the key data size of RDD.
  /// @return The data size of RDD.
  virtual size_t keySize() const override;

  /// @brief  Get the value data size of RDD.
  /// @return The data size of RDD.
  virtual size_t valueSize() const override;

  virtual bool parse(idgs::actor::ActorMessagePtr& msg) override;

  void put(const idgs::actor::PbMessagePtr key, std::vector<idgs::actor::PbMessagePtr>& value);
  void put(const idgs::actor::PbMessagePtr key, const idgs::actor::PbMessagePtr value, const bool& unique = false);
  void persist(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value, const bool& unique = false);

  void repartition(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value);
  void repartition(const idgs::actor::PbMessagePtr& key, std::vector<idgs::actor::PbMessagePtr>& value);

  void processRow(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value, const bool& putUnique = false);

  virtual const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

  void flushLocalCache();
  void setUseRepartitionLocalCache(bool v) {
    useRepartitionLocalCache = v;
  }

private:
  static idgs::actor::ActorDescriptorPtr descriptor;

  // data container
  OrderedRddDataMap orderedDataMap;
  UnorderedRddDataMap unorderedDataMap;

  // local repartition cache
  std::multimap<idgs::actor::PbMessagePtr, idgs::actor::PbMessagePtr, idgs::store::less> localCache;
  bool useRepartitionLocalCache = false;

private:
  size_t calcPartitionId(const idgs::actor::PbMessagePtr& key);

private:
  void handleRddTransform(const idgs::actor::ActorMessagePtr& msg);
  void handlePartitionStore(const idgs::actor::ActorMessagePtr& msg);
  void handleRePartition(const idgs::actor::ActorMessagePtr& msg);
  void handleCheckPartitionReady(const idgs::actor::ActorMessagePtr& msg);
  void handleActionRequest(const idgs::actor::ActorMessagePtr& msg);
};

} // namespace rdd
} // namespace idgs 
