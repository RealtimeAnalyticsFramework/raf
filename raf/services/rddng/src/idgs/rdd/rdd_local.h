/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/rdd/transform/transformer.h"
#include "idgs/store/listener/store_listener.h"

namespace idgs {
namespace rdd {

class RddLocal {
public:
  RddLocal();
  virtual ~RddLocal();

public:
  void setRddInfo(const std::string& rddname, const int32_t& memberId, const std::string& actorId, const idgs::rdd::pb::RddState& rddState);
  const std::string& getRddName() const;
  const idgs::pb::ActorId& getRddId() const;

  void setRddState(const idgs::rdd::pb::RddState& rddState);
  const idgs::rdd::pb::RddState& getRddState() const;

  void setTransformerMsg(const idgs::actor::ActorMessagePtr& transformerMsg);
  const idgs::actor::ActorMessagePtr& getTransformerMsg() const;

  void setKeyValueTemplate(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value);
  const idgs::actor::PbMessagePtr& getKeyTemplate() const;
  const idgs::actor::PbMessagePtr& getValueTemplate() const;

  void setPersistType(const idgs::rdd::pb::PersistType& type);
  const idgs::rdd::pb::PersistType& getPersistType() const;

  void setTransformer(const TransformerPtr& trans);
  const TransformerPtr& getTransformer() const;

  const std::shared_ptr<google::protobuf::FileDescriptorProto>& getKeyMetadata() const;
  const std::shared_ptr<google::protobuf::FileDescriptorProto>& getValueMetadata() const;

  void setReplicatedRdd(const bool& isReplicatedRdd);
  bool isReplicatedRdd();

  void setRepartition(const bool& isRepartition);
  bool isRepartition();

  bool isAggregate();

  void setUpstreamSync(const bool& isUpstreamSync);
  bool isUpstreamSync() const;

  void setDelegateRdd(const bool& isDelegateRdd);
  bool isDelegateRdd() const;

  void setTransformed(const bool& isTransformed);
  bool isTransformed() const;

  void addPartitionActor(const size_t& partition, const idgs::pb::ActorId& actorId);
  const idgs::pb::ActorId& getPartitionActor(const size_t& partition);

  void setPartitionState(const size_t& partition, const idgs::rdd::pb::RddState& state);
  const idgs::rdd::pb::RddState& getPartitionState(const size_t& partition);

  void addLocalPartition(const size_t& partition, BaseRddPartition* rddPartition);
  BaseRddPartition* getLocalPartition(const size_t& partition);
  
  const std::vector<BaseRddPartition*> getUpstreamPartition(const size_t& partition);

  const std::vector<PairRddPartition*> getDownstreamPartition(const size_t& partition);
  
  void setMainRddOperator(op::RddOperator* rddOp);
  void addRddOperator(const idgs::actor::ActorMessagePtr& msg, const op::RddOperator* rddOperator);

  const std::vector<size_t>& getLocalPartitions() const;

  void setRddStoreListener(idgs::store::StoreListener* listener);
  idgs::store::StoreListener* getRddStoreListener();

  void addUpstreamRddLocal(RddLocal* rddLocal);
  const std::vector<RddLocal*>& getUpstreamRddLocal() const;

  void addDownstreamRddLocal(RddLocal* rddLocal);
  const std::vector<RddLocal*>& getDownstreamRddLocal() const;

  // destroy all local partition actors
  // remove refcount in upstream rdd
  void onDestroy();

private:
  /// all member shared

  /// info of RDD include name, actorId and state
  std::string rddName;
  idgs::pb::ActorId rddId;
  idgs::rdd::pb::RddState state;

  /// the message of create RDD, payload is CreateDelegateRddRequest or CreateRddRequest
  idgs::actor::ActorMessagePtr transMsg;

  /// template proto message of key
  idgs::actor::PbMessagePtr keyTemplate;

  /// template proto message of value
  idgs::actor::PbMessagePtr valueTemplate;

  /// persist type of RDD, default is NONE
  idgs::rdd::pb::PersistType persistType;

  /// the transformer of RDD
  TransformerPtr transformer;

  /// the RDD operator of first child
  op::RddOperator* rddOperator;

  /// the key and value metadata of RDD
  std::shared_ptr<google::protobuf::FileDescriptorProto> keyMetadata;
  std::shared_ptr<google::protobuf::FileDescriptorProto> valueMetadata;

  /// whether RDD is replicated
  bool replicatedRdd;

  /// whether RDD need repartition
  bool repartition;

  /// whether RDD need aggregate
  bool aggregate;

  /// whether upstream RDD sync process or async
  bool upstreamSync;

  /// whether RDD is delegate
  bool delegateRdd;

  /// whether RDD is already running transformer
  bool transformed;

  /// all partition info
  std::vector<idgs::pb::ActorId> partitionActors;
  std::vector<idgs::rdd::pb::RddState> partitionStates;

  std::vector<RddLocal*> upstreamRddLocal;
  std::vector<RddLocal*> downstreamRddLocal;

  /// local member shared

  /// local partitions
  std::map<size_t, BaseRddPartition*> localPartitionMap;

  /// partition IDs of local member
  std::vector<size_t> localPartitions;

  idgs::store::StoreListener* rddStoreListener;

};

} // namespace rdd 
} // namespace idgs 
