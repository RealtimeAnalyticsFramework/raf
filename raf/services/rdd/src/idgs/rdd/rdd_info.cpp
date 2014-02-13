
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "rdd_info.h"
#include "idgs/cluster/cluster_framework.h"

using namespace idgs::pb;
using namespace idgs::cluster;
using namespace idgs::rdd::pb;

namespace idgs {
namespace rdd {

PartitionInfo::PartitionInfo() :
    state(P_INIT) {
}

PartitionInfo::~PartitionInfo() {
}

void PartitionInfo::setActorId(const ActorId& actorId) {
  this->actorId = actorId;
}

void PartitionInfo::setState(const PartitionState& state) {
  this->state = state;
}

const ActorId& PartitionInfo::getActorId() const {
  return actorId;
}

const PartitionState& PartitionInfo::getState() const {
  return state;
}

RddInfo::RddInfo() :
    state(INIT) {
  size_t partCount = ::idgs::util::singleton<ClusterFramework>::getInstance().getPartitionCount();
  partitionInfo.resize(partCount);
}

RddInfo::~RddInfo() {
  partitionInfo.clear();
}

void RddInfo::setActorId(const int32_t& memberId, const std::string& actorId) {
  this->actorId.set_actor_id(actorId);
  this->actorId.set_member_id(memberId);
}

void RddInfo::setState(const RddState& state) {
  this->state = state;
}

void RddInfo::addPartitionInfo(const uint32_t& partition, const ActorId& actorId, const PartitionState& state) {
  partitionInfo[partition].setState(state);
  partitionInfo[partition].setActorId(actorId);
}

void RddInfo::setPartitionState(const uint32_t& partition, const PartitionState& state) {
  partitionInfo[partition].setState(state);
}

const ActorId& RddInfo::getActorId() const {
  return actorId;
}

const RddState& RddInfo::getState() const {
  return state;
}

const PartitionState& RddInfo::getPartitionState(const uint32_t& partition) const {
  return partitionInfo[partition].getState();
}

const std::vector<PartitionInfo>& RddInfo::getPartitionInfo() const {
  return partitionInfo;
}

bool RddInfo::isTheRdd(const idgs::pb::ActorId& actorId) {
  return (this->actorId.member_id() == actorId.member_id() && this->actorId.actor_id() == actorId.actor_id());
}

} /* namespace rdd */
} /* namespace idgs */
