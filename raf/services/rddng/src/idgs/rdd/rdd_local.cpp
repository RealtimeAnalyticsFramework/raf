/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "rdd_local.h"

#include "idgs/application.h"

#include "idgs/rdd/base_rdd_actor.h"
#include "idgs/rdd/pair_rdd_partition.h"
#include "idgs/rdd/transform/transformer.h"

#include "idgs/store/listener/store_listener.h"

using namespace idgs::pb;
using namespace idgs::actor;
using namespace idgs::rdd::pb;
using namespace google::protobuf;

namespace idgs {
namespace rdd {

RddLocal::RddLocal() : state(pb::INIT), persistType(NONE), transformer(NULL), rddOperator(NULL), keyMetadata(NULL), valueMetadata(NULL),
    replicatedRdd(false), repartition(false), aggregate(false), upstreamSync(true), delegateRdd(false), transformed(false), rddStoreListener(NULL) {
  auto cluster = idgs_application()->getClusterFramework();

  auto partitionSize = cluster->getPartitionCount();
  partitionActors.resize(partitionSize);
  partitionStates.resize(partitionSize);

  auto localMemberId = cluster->getMemberManager()->getLocalMemberId();
  for (size_t partition = 0; partition < partitionSize; ++ partition) {
    if (localMemberId == cluster->getPartitionManager()->getPartition(partition)->getPrimaryMemberId()) {
      localPartitions.push_back(partition);
    }
  }
}

RddLocal::~RddLocal() {
}

void RddLocal::setRddInfo(const string& rddname, const int32_t& memberId, const string& actorId, const RddState& rddState) {
  rddName = rddname;
  rddId.set_member_id(memberId);
  rddId.set_actor_id(actorId);
  state = rddState;
}

const string& RddLocal::getRddName() const {
  return rddName;
}

const ActorId& RddLocal::getRddId() const {
  return rddId;
}

void RddLocal::setRddState(const RddState& rddState) {
  state = rddState;
}

const RddState& RddLocal::getRddState() const {
  return state;
}

void RddLocal::setTransformerMsg(const ActorMessagePtr& transformerMsg) {
  transMsg = const_cast<ActorMessagePtr&>(transformerMsg);
}

const ActorMessagePtr& RddLocal::getTransformerMsg() const {
  return transMsg;
}

void RddLocal::setKeyValueTemplate(const PbMessagePtr& key, const PbMessagePtr& value) {
  keyTemplate = key;
  valueTemplate = value;

  if (!keyMetadata) {
    keyMetadata = make_shared<FileDescriptorProto>();
  }

  auto keyDescriptor = keyTemplate->GetDescriptor();
  auto& keyName = keyDescriptor->full_name();
  auto pos = keyName.find_last_of(".");
  auto keyPackage = (pos == string::npos) ? "" : keyName.substr(0, pos);

  keyMetadata->set_name(getRddName() + "_KEY.proto");
  keyMetadata->set_package(keyPackage);
  keyDescriptor->CopyTo(keyMetadata->add_message_type());

  if (!valueMetadata) {
    valueMetadata = make_shared<FileDescriptorProto>();
  }

  auto valueDescriptor = valueTemplate->GetDescriptor();
  auto& valueName = valueDescriptor->full_name();
  pos = valueName.find_last_of(".");
  auto valuePackage = (pos == string::npos) ? "" : valueName.substr(0, pos);

  valueMetadata->set_name(getRddName() + "_VALUE.proto");
  valueMetadata->set_package(valuePackage);
  valueDescriptor->CopyTo(valueMetadata->add_message_type());
}

const PbMessagePtr& RddLocal::getKeyTemplate() const {
  return keyTemplate;
}

const PbMessagePtr& RddLocal::getValueTemplate() const {
  return valueTemplate;
}

void RddLocal::setPersistType(const PersistType& type) {
  persistType = type;
}

const PersistType& RddLocal::getPersistType() const {
  return persistType;
}

void RddLocal::setTransformer(const idgs::rdd::TransformerPtr& trans) {
  transformer = trans;

  if (trans->getName() == GROUP_TRANSFORMER) {
    repartition = true;
  } else if (trans->getName() == HASH_JOIN_TRANSFORMER) {
    upstreamSync = false;
  } else if (trans->getName() == REDUCE_BY_KEY_TRANSFORMER) {
    repartition = false;
  } else if (trans->getName() == REDUCE_TRANSFORMER) {
    repartition = true;
    aggregate = true;
  }
}

const TransformerPtr& RddLocal::getTransformer() const {
  return transformer;
}

const shared_ptr<FileDescriptorProto>& RddLocal::getKeyMetadata() const {
  return keyMetadata;
}

const shared_ptr<FileDescriptorProto>& RddLocal::getValueMetadata() const {
  return valueMetadata;
}

void RddLocal::setReplicatedRdd(const bool& isReplicatedRdd) {
  replicatedRdd = isReplicatedRdd;
  if (replicatedRdd) {
    repartition = false;
  }
}

bool RddLocal::isReplicatedRdd() {
  return replicatedRdd;
}

void RddLocal::setRepartition(const bool& isRepartition) {
  repartition = isRepartition;
}

bool RddLocal::isRepartition() {
  return repartition;
}

bool RddLocal::isAggregate() {
  return aggregate;
}

void RddLocal::setUpstreamSync(const bool& isUpstreamSync) {
  upstreamSync = isUpstreamSync;
}

bool RddLocal::isUpstreamSync() const {
  return upstreamSync;
}

void RddLocal::setDelegateRdd(const bool& isDelegateRdd) {
  delegateRdd = isDelegateRdd;
}

void RddLocal::addPartitionActor(const size_t& partition, const ActorId& actorId) {
  partitionActors[partition].CopyFrom(actorId);
}

const ActorId& RddLocal::getPartitionActor(const size_t& partition) {
  return partitionActors[partition];
}

void RddLocal::setPartitionState(const size_t& partition, const RddState& state) {
  partitionStates[partition] = state;
}

const RddState& RddLocal::getPartitionState(const size_t& partition) {
  return partitionStates[partition];
}

bool RddLocal::isDelegateRdd() const {
  return delegateRdd;
}

void RddLocal::setTransformed(const bool& isTransformed) {
  transformed = isTransformed;
}

bool RddLocal::isTransformed() const {
  return transformed;
}

void RddLocal::addLocalPartition(const size_t& partition, BaseRddPartition* rddPartition) {
  localPartitionMap[partition] = rddPartition;
}

BaseRddPartition* RddLocal::getLocalPartition(const size_t& partition) {
  auto it = localPartitionMap.find(partition);
  if (it == localPartitionMap.end()) {
    return NULL;
  } else {
    return it->second;
  }
}

const std::vector<BaseRddPartition*> RddLocal::getUpstreamPartition(const size_t& partition) {
  std::vector<BaseRddPartition*> partitions;
  for (int32_t i = 0; i < upstreamRddLocal.size(); ++ i) {
    auto partitionRdd = upstreamRddLocal.at(i)->getLocalPartition(partition);
    partitions.push_back(partitionRdd);
  }
  return partitions;
}

const std::vector<PairRddPartition*> RddLocal::getDownstreamPartition(const size_t& partition) {
  std::vector<PairRddPartition*> partitions;
  for (int32_t i = 0; i < downstreamRddLocal.size(); ++ i) {
    auto partitionRdd = downstreamRddLocal.at(i)->getLocalPartition(partition);
    partitions.push_back(dynamic_cast<PairRddPartition*>(partitionRdd));
  }
  return partitions;
}

void RddLocal::setMainRddOperator(op::RddOperator* rddOp) {
  rddOperator = rddOp;
  if (aggregate) {
    auto it = localPartitions.begin();
    for (; it != localPartitions.end(); ++ it) {
      transform::TransformerContext ctx;
      ctx.setTransformerMsg(transMsg);
      ctx.setRddOperator(rddOperator);
      getLocalPartition(* it)->setAggrTransformerContext(ctx);
    }
  }
}

void RddLocal::addRddOperator(const ActorMessagePtr& msg, const op::RddOperator* rddOperator) {
  auto it = localPartitions.begin();
  for (; it != localPartitions.end(); ++ it) {
    transform::TransformerContext ctx;
    ctx.setTransformerMsg(msg);
    ctx.setRddOperator(rddOperator);
    getLocalPartition(* it)->addTransformerContext(ctx);
  }
}

const vector<size_t>& RddLocal::getLocalPartitions() const {
  return localPartitions;
}

void RddLocal::setRddStoreListener(idgs::store::StoreListener* listener) {
  rddStoreListener = listener;
}

idgs::store::StoreListener* RddLocal::getRddStoreListener() {
  return rddStoreListener;
}

void RddLocal::addUpstreamRddLocal(const std::shared_ptr<RddLocal>& rddLocal) {
  upstreamRddLocal.push_back(rddLocal);
}

const std::vector<std::shared_ptr<RddLocal>>& RddLocal::getUpstreamRddLocal() const {
  return upstreamRddLocal;
}

void RddLocal::addDownstreamRddLocal(const std::shared_ptr<RddLocal>& rddLocal) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  downstreamRddLocal.push_back(rddLocal);
}

const std::vector<std::shared_ptr<RddLocal>>& RddLocal::getDownstreamRddLocal() const {
  return downstreamRddLocal;
}

void RddLocal::removeDownstreamRddLocal(const std::shared_ptr<RddLocal>& rddLocal) {
  auto& v = downstreamRddLocal;
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);

  for (auto it = v.begin(); it != v.end(); ++it) {
    if ((* it) == rddLocal) {
      v.erase(it);
      break;
    }
  }

  if (v.empty()) {
    auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
    if (rddId.member_id() == localMemberId) {
      BaseRddActor* actor = dynamic_cast<BaseRddActor*>(idgs_application()->getActorframework()->getActor(rddId.actor_id()));
      if (actor) {
        actor->onDownstreamRemoved();
      }
    }
  }

}

void RddLocal::onDestroy() {
  // destroy all local partition actors
  for (auto& p: localPartitionMap) {
    p.second->terminate();
  }

  // remove refcount in upstream rdd
  for (auto up : upstreamRddLocal) {
    up->removeDownstreamRddLocal(shared_from_this());
  }

  partitionActors.clear();
  partitionStates.clear();

  localPartitionMap.clear();
  localPartitions.clear();

  upstreamRddLocal.clear();
  downstreamRddLocal.clear();

  if (rddOperator) {
    if (!rddOperator->paramOperators.empty()) {
      auto itOp = rddOperator->paramOperators.begin();
      for (; itOp != rddOperator->paramOperators.end(); ++ itOp) {
        if ((* itOp)) {
          delete (* itOp);
          * itOp = NULL;
        }
      }
      rddOperator->paramOperators.clear();
    }

    delete rddOperator;
    rddOperator = NULL;
  }

  if (rddStoreListener) {
    delete rddStoreListener;
    rddStoreListener = NULL;
  }
}

} // namespace rdd 
} // namespace idgs 
