
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include <iomanip>

#include "idgs/application.h"
#include "balancer/balancer_util.h"

using namespace idgs::pb;
using namespace idgs::actor;

namespace idgs {
namespace cluster {

PartitionManagerActor::PartitionManagerActor() : config(NULL), member_manager(NULL) {
  actorId = AID_PARTITION; /// override actor id
}

PartitionManagerActor::~PartitionManagerActor() {
  function_footprint();
}


void PartitionManagerActor::init(idgs::pb::ClusterConfig* cfg) {
  this->config = cfg;
  // create partition table
  createPartitionTable(config->partition_count(), config->max_replica_count());
  /// register actor
  idgs_application()->getRpcFramework()->getActorManager()->registerServiceActor(this->getActorId(), this);
}

void PartitionManagerActor::onDestroy() {
  // actor::StatefulActor::onDestroy(); // the parent will delete this
  idgs_application()->getRpcFramework()->getActorManager()->unregisterServiceActor(this->getActorId());
  std::list<PartitionListener*>().swap(this->listeners);

  (const_cast<idgs::actor::ActorMessageHandlerMap&>(getMessageHandlerMap())).clear();
  (const_cast<idgs::actor::ActorDescriptorPtr&>(getDescriptor())).reset();
}

// this method should be called by module initializer.
const ::idgs::actor::ActorDescriptorPtr& PartitionManagerActor::generateActorDescriptor() {
  static idgs::actor::ActorDescriptorPtr descriptor;
  if (descriptor) {
    return descriptor;
  }
  descriptor = std::make_shared<ActorDescriptorWrapper>();

  descriptor->setName(AID_PARTITION);
  descriptor->setDescription("Cluster partition management");
  descriptor->setType(idgs::pb::AT_STATELESS);

  // whole partition table event actor descriptor
  ::idgs::actor::ActorOperationDescriporWrapper whole_partition_table_evt;
  whole_partition_table_evt.setName(OID_WHOLE_PARTITION_TABLE);
  whole_partition_table_evt.setDescription("whole partition table sent by leading to new joined members");
  whole_partition_table_evt.setPayloadType("idgs.pb.WholePartitionEvent");
  descriptor->setInOperation(whole_partition_table_evt.getName(), whole_partition_table_evt);
  descriptor->setOutOperation(whole_partition_table_evt.getName(), whole_partition_table_evt);

  // delta partition event actor descriptor
  ::idgs::actor::ActorOperationDescriporWrapper delta_partition_evt;
  delta_partition_evt.setName(OID_DELTA_PARTITIONS);
  delta_partition_evt.setDescription("delta partition sent by leading to all");
  delta_partition_evt.setPayloadType("idgs.pb.DeltaPartitionEvent");
  descriptor->setInOperation(delta_partition_evt.getName(), delta_partition_evt);
  descriptor->setOutOperation(delta_partition_evt.getName(), delta_partition_evt);

  ::idgs::actor::ActorOperationDescriporWrapper partition_status_change_evt;
  partition_status_change_evt.setName(OID_PARTITION_STATE_CHANGED);
  partition_status_change_evt.setDescription("update partition status");
  partition_status_change_evt.setPayloadType("idgs.pb.PartitionStatusChangeEvent");
  descriptor->setInOperation(partition_status_change_evt.getName(), partition_status_change_evt);

  // consume itself
  descriptor->addConsumeActor(AID_PARTITION);

  return descriptor;
}

const idgs::actor::ActorMessageHandlerMap& PartitionManagerActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      { OID_WHOLE_PARTITION_TABLE, {
          static_cast<idgs::actor::ActorMessageHandler>(&PartitionManagerActor::handleWholePartitionTableEvent),
          &idgs::pb::WholePartitionEvent::default_instance()
      }},
      { OID_DELTA_PARTITIONS, {
          static_cast<idgs::actor::ActorMessageHandler>(&PartitionManagerActor::handleDeltaPartitionEvent),
          &idgs::pb::DeltaPartitionEvent::default_instance()
      }},
      { OID_PARTITION_STATE_CHANGED, {
          static_cast<idgs::actor::ActorMessageHandler>(&PartitionManagerActor::handlePartitionStatusChangedEvent),
          &idgs::pb::PartitionStatusChangeEvent::default_instance()
      }},
      { OID_LIST_PARTITIONS, {
          static_cast<idgs::actor::ActorMessageHandler>(&PartitionManagerActor::handleListPartitions),
          NULL
      }}
  };
  return handlerMap;
}

/**
 * override parent create actor message
 */
idgs::actor::ActorMessagePtr PartitionManagerActor::createActorMessage() const {
  auto actorMsg = Actor::createActorMessage();
  actorMsg->setDestActorId(AID_PARTITION);
  actorMsg->setDestMemberId(ALL_MEMBERS);
  return actorMsg;
}

void PartitionManagerActor::addListener(PartitionListener* listener) {
  listeners.push_back(listener);
}

void PartitionManagerActor::recalculate() {
  function_footprint();
  /// recalculate each member own node counts.
  for(size_t i = 0, size = partitions.size(); i < size; ++i) {
    int32_t member_id = getMemberId(i, 0);
    if(member_id == -1) {
      continue;
    }
    auto member = member_manager->getMember(member_id);
    if(member->isLocalStore()) {
//      member->setPartitionCount(0, member->getPartitionCount(0) + 1);
//      member->setPartitionCount(1, member->getPartitionCount(0) * (config->max_replica_count() - 1));
    }
  }
}

void PartitionManagerActor::memberStatusChanged(const MemberWrapper& member) {
  function_footprint();
  auto localMember = member_manager->getLocalMember();
  if(!localMember || !localMember->isLeading()) { // not leading, not handle this event.
    return;
  }
  auto& changedMember = const_cast<MemberWrapper&>(member);
  if(changedMember.getState() == idgs::pb::MS_JOINED) { /// join
    if(!changedMember.isLeading()) { /// not leading member join, multicast whole partition table to new joined member.
      ResultCode rs = multicastWholePartitionTable(changedMember.getId());
      if (rs != RC_SUCCESS) {
        LOG(ERROR)<< getErrorDescription(rs);
        return;
      }
    }
    if(changedMember.isLocalStore()) { /// local store member, balance partition table
      DeltaPartitionEvent evt;
      balance(changedMember, evt);
    }
    member_manager->mcastMemberStatus(changedMember.getId(), idgs::pb::MS_PREPARED);
  } else if(changedMember.getState() == idgs::pb::MS_INACTIVE) { /// leave
    if(changedMember.isLeading()) { /// leading leave
      recalculate();
    } else if(changedMember.isLocalStore()) {
      DeltaPartitionEvent evt;
      balance(changedMember, evt);
    }
  }
}

void PartitionManagerActor::handleWholePartitionTableEvent(const std::shared_ptr<ActorMessage>& actor_msg_ptr) {
  function_footprint();
  auto localMember = member_manager->getLocalMember();
  WholePartitionEvent* evt = dynamic_cast<WholePartitionEvent*>(actor_msg_ptr->getPayload().get());
  if(!localMember || localMember->getId() != evt->joinedmemberid()) {
    return;
  }
  /// check each member's partition count consistency
  if(evt->table().partition_size() != config->partition_count()) {
    LOG(FATAL) << "Partition count doesn't match, please check \'cluster.conf\'.";
  }
  for (int i = 0, size = evt->table().partition_size(); i < size; ++i) {
    partitions[i].setPartition(evt->table().partition(i));
  }
  VLOG(0) << "after joined member receive whole partition " << toString();
}

void PartitionManagerActor::balance(MemberWrapper& member, DeltaPartitionEvent& evt) {
  function_footprint();
  auto members = const_cast<std::vector<MemberWrapper>&>(member_manager->getMemberTable());
  auto balancer = BalancerUtil::createBalancer();
  balancer->setConfig(config);
  balancer->balance(members, partitions, member.getId(), evt);
  auto deltaPartitionEvent = std::make_shared<DeltaPartitionEvent>(evt);
  multicastDeltaPartitionEvent(deltaPartitionEvent);
}

void PartitionManagerActor::balanceOk(pb::DeltaPartitionEvent& evt) {
  function_footprint();
  VLOG(1) << "before balanced OK " << toString();
  for(int i = 0, size = evt.items_size(); i < size; ++i) {
    auto& item = evt.items(i);
    const size_t partitionId = item.part_id();
    const uint32_t pos = item.position();
    const int32_t memberId = item.new_mid();
    partitions[partitionId].setMemberId(pos, memberId);
    if(item.has_state()) {
      partitions[partitionId].setState(pos, item.state());
    }
  }
  VLOG(1) << "after balanced OK " << toString();
}

void PartitionManagerActor::handleDeltaPartitionEvent(const std::shared_ptr<ActorMessage>& msg) {
  function_footprint();
  auto localMember = member_manager->getLocalMember();
  if(!localMember){
    return;
  }
  auto evt = dynamic_cast<DeltaPartitionEvent*>(msg->getPayload().get());
  balanceOk(*evt);
  firePartitionChangedEvent(*evt);
}

void PartitionManagerActor::handlePartitionStatusChangedEvent(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr) {
  auto evt = dynamic_cast<PartitionStatusChangeEvent*>(actor_msg_ptr->getPayload().get());
  auto& partitionWrapper = partitions.at(evt->partition_id());
  auto& partition = partitionWrapper.getPartition();
  for (int32_t i = 0; i < partition.cells_size(); ++ i) {
    if (evt->member_id() == partition.cells(i).member_id()) {
      partitionWrapper.setState(i, evt->state());
      break;
    }
  }
}

void PartitionManagerActor::handleListPartitions(const std::shared_ptr<idgs::actor::ActorMessage>& msg) {
  auto result = msg->createResponse();

  std::shared_ptr<idgs::pb::PartitionTable> cc = std::make_shared<idgs::pb::PartitionTable>();
  genPartitionTable(*cc);

  result->setPayload(cc);
  result->setOperationName("list_partitions");
  idgs::actor::sendMessage(result);
}


void PartitionManagerActor::genPartitionTable(pb::PartitionTable& table) {
  for (auto it = partitions.begin(); it != partitions.end(); ++it) {
    table.add_partition()->CopyFrom(it->getPartition());
  }
}

void PartitionManagerActor::removeListener(PartitionListener* listener) {
  for (auto it = listeners.begin(); it != listeners.end();) {
    if (*it == listener) {
      // delete *it;
      listeners.erase(it);
      break;
    } else {
      ++it;
    }
  }
}

void PartitionManagerActor::firePartitionChangedEvent(const idgs::pb::DeltaPartitionEvent& evt) {
  function_footprint();
  for(auto it = listeners.begin(); it != listeners.end(); ++it) {
    (*it)->partitionChanged(evt);
  }
}


void PartitionManagerActor::createPartitionTable(size_t partition_count, uint8_t backup_nodes) {
  for(size_t i = 0; i < partition_count; ++i) {
    PartitionWrapper p(backup_nodes);
    partitions.push_back(p);
  }
}

idgs::ResultCode PartitionManagerActor::multicastPartitionMessage(const std::string& message_type,
    std::shared_ptr<google::protobuf::Message> pay_load) {
  std::shared_ptr<idgs::actor::ActorMessage> actor_msg_ptr = createActorMessage();
  actor_msg_ptr->setOperationName(message_type);
  actor_msg_ptr->setPayload(pay_load);
  return idgs_application()->multicastMessage(actor_msg_ptr);
}

idgs::ResultCode PartitionManagerActor::multicastWholePartitionTable(uint32_t joinedMemberId) {
  auto evt = std::make_shared<idgs::pb::WholePartitionEvent>();
  evt->set_joinedmemberid(joinedMemberId);
  genPartitionTable(*evt->mutable_table());
  return multicastPartitionMessage(OID_WHOLE_PARTITION_TABLE, evt);
}

idgs::ResultCode PartitionManagerActor::multicastDeltaPartitionEvent(const std::shared_ptr<idgs::pb::DeltaPartitionEvent>& deltaPartitionEvent) {
  return multicastPartitionMessage(OID_DELTA_PARTITIONS, deltaPartitionEvent);
}

std::string PartitionManagerActor::toString() {
  std::stringstream s;
  s << std::endl << "=============== partition table ===============" << std::endl;
  for (int i = 0, size = partitions.size(); i < size; ++i) {
    s << std::setw(4) << i << " | ";
    auto& p = partitions.at(i);
    auto nodes = p.getPartition().cells_size();
    for(auto j = 0; j < nodes; ++j) {
      s << std::setw(3) << p.getMemberId(j) << "(" << p.getState(j) << ")" << " | ";
    }
    s << std::endl;
  }
  return s.str();
}

} /// namespace cluster
} /// namespace idgs
