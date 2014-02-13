
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "base_rdd_partition.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <tbb/spin_rw_mutex.h>
#include <google/protobuf/descriptor.h>

#include "protobuf/msg_comparer.h"
#include "idgs/expr/expression.h"

#include "idgs/actor/actor_message.h"

#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_common.pb.h"
#include "idgs/rdd/pb/rdd_svc.pb.h"
#include "idgs/rdd/op/expr_operator.h"

namespace idgs {
namespace rdd {

typedef std::map<idgs::actor::PbMessagePtr, std::vector<idgs::actor::PbMessagePtr>, protobuf::sharedless> OrderedRddDataMap;
typedef std::unordered_map<idgs::actor::PbMessagePtr, std::vector<idgs::actor::PbMessagePtr>,
    protobuf::shared_hash_code, protobuf::sharedless> UnorderedRddDataMap;

enum DataType {
  T_ORDERED = 1,
  T_UNORDERED = 2
};


/// The stateful actor of RDD partition.
/// The partition actor of RDDs.
class RddPartition: public BaseRddPartition {
public:

  /// @brief Constructor
  /// @param partitionId  Partition of store.
  RddPartition(const uint32_t& partitionId);

  /// @brief Destructor
  virtual ~RddPartition();

  /// @brief  Generate descriptor for RddPartition.
  /// @return The descriptor for RddPartition.
  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

  /// @brief  Get the descriptor for RddPartition.
  /// @return The descriptor for RddPartition.
  const idgs::actor::ActorDescriptorPtr& getDescriptor() const override;

  const std::string& getActorName() const override {
    static std::string actorName = RDD_PARTITION;
    return actorName;
  }

  /// parse payload and attachments
  ///
  virtual bool parse(idgs::actor::ActorMessagePtr& msg) override;

  /// @brief  Set RDD ID.
  /// @param  actorId RDD actor ID.
  void setRddId(const idgs::pb::ActorId& actorId);

  /// @brief  Add depending RDD partition actor of the same partition.
  /// @param  partitonActor Actor ID of RDD partition.
  /// @return Status code of result.
  pb::RddResultCode addRddPartitionActor(const std::string& partitonActor);

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
  /// @param  value Values of data.
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

  /// @brief  Set the context (key and value) about current rdd partition.
  /// @param  key   Key of context.
  /// @param  value Value of context.
  void setRddPartitionContext(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value);

  const idgs::rdd::op::OutMessagePair& getOutMessage(const int32_t inRddIndex) const;

  /// @brief  Get the field names by alias.
  /// @param  fieldAlias   Field alias.
  /// @return The field names use by alias.
  const idgs::expr::Expression* getFilterExpression(const int32_t inRddIndex);

  const bool& isReUseKey() const {
    return reUseKey;
  }

  virtual const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

  idgs::rdd::op::ExprMapOperator& getExprOperator(int index) {
    return inRddInfo.at(index);
  }

private:
  static idgs::actor::ActorDescriptorPtr descriptor;
  idgs::pb::ActorId rddId;
  std::vector<BaseRddPartition*> dependingPartitionActors;
  OrderedRddDataMap orderedDataMap;
  UnorderedRddDataMap unorderedDataMap;
  DataType dataType;
  std::vector<idgs::rdd::op::ExprMapOperator> inRddInfo;
  std::vector<idgs::pb::ActorId> partitionActors;
  bool reUseKey;
  static tbb::spin_rw_mutex mutex;

  void handlePartitionPreparedRequest(const idgs::actor::ActorMessagePtr& msg);
  void handleProcess(const idgs::actor::ActorMessagePtr& msg);
  void handleAction(const idgs::actor::ActorMessagePtr& msg);
  void handleRePartitionMigrate(const idgs::actor::ActorMessagePtr& msg);
  void handleSendRddInfo(const idgs::actor::ActorMessagePtr& msg);
  void handleCheckPartitionReady(const idgs::actor::ActorMessagePtr& msg);
  void handleDestory(const idgs::actor::ActorMessagePtr& msg);

  static void createDynamicMessage(const uint32_t partition, const pb::CreateRddRequest& request);
};

} // namespace rdd
} // namespace idgs 
