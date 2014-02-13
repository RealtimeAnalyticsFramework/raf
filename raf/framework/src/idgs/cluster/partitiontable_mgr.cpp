
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/cluster/cluster_framework.h"
#include <iomanip>
using namespace std;
using namespace idgs::pb;
using namespace idgs::actor;
using namespace idgs::pb;

namespace idgs {

namespace cluster {

PartitionTableMgr::PartitionTableMgr() :
    config(NULL), member_manager(NULL), partitionBalancer(new NaivePartitionBalancer) {
  setActorId(AID_PARTITION);
  ready.store(true);
}

PartitionTableMgr::~PartitionTableMgr() {
  function_footprint();
}

void PartitionTableMgr::onDestroy() {
  list<PartitionListener*>().swap(this->listeners);
  function_footprint();
}

void PartitionTableMgr::init(idgs::pb::ClusterConfig* cfg) {
  this->config = cfg;
  partitionBalancer->setConfig(this->config);
  // create partition table
  createPartitionTable(config->partition_count(), config->max_backup_count());
  // register actor framework
  registerActorFramework();
}

// this method should be called by module initializer.
std::shared_ptr<idgs::actor::ActorDescriptorWrapper> PartitionTableMgr::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor)
    return descriptor;
  descriptor.reset(new ::ActorDescriptorWrapper);

  descriptor->setName(AID_PARTITION);
  descriptor->setDescription("Cluster partition management");
  descriptor->setType(::AT_STATELESS);

  // whole partition table event actor descriptor
  ::idgs::actor::ActorOperationDescriporWrapper whole_partition_table_evt;
  whole_partition_table_evt.setName(WHOLE_PARTITION_TABLE);
  whole_partition_table_evt.setDescription("whole partition table sent by leading to new joined members");
  whole_partition_table_evt.setPayloadType("idgs.pb.WholePartitionEvent");
  descriptor->setInOperation(whole_partition_table_evt.getName(), whole_partition_table_evt);
  descriptor->setOutOperation(whole_partition_table_evt.getName(), whole_partition_table_evt);

  // delta partition event actor descriptor
  ::idgs::actor::ActorOperationDescriporWrapper delta_partition_evt;
  delta_partition_evt.setName(DELTA_PARTITIONS);
  delta_partition_evt.setDescription("delta partition sent by leading to all");
  delta_partition_evt.setPayloadType("idgs.pb.DeltaPartitionEvent");
  descriptor->setInOperation(delta_partition_evt.getName(), delta_partition_evt);
  descriptor->setOutOperation(delta_partition_evt.getName(), delta_partition_evt);

  // consume itself
  descriptor->addConsumeActor(AID_PARTITION);

  return descriptor;
}

const idgs::actor::ActorMessageHandlerMap& PartitionTableMgr::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {WHOLE_PARTITION_TABLE,  static_cast<idgs::actor::ActorMessageHandler>(&PartitionTableMgr::handleWholePartitionTableEvent)},
      {DELTA_PARTITIONS,       static_cast<idgs::actor::ActorMessageHandler>(&PartitionTableMgr::handleDeltaPartitionEvent)},
  };
  return handlerMap;
}

void PartitionTableMgr::registerActorFramework() {
  this->descriptor = generateActorDescriptor();
  // register actor
  ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework()->Register(this->getActorId(), this);
  // end register actor
}

/**
 * override parent create actor message
 */
std::shared_ptr<idgs::actor::ActorMessage> PartitionTableMgr::createActorMessage() {
  std::shared_ptr<ActorMessage> actor_msg_ptr = super::createActorMessage();
  // append destActorid and destMemberid
  actor_msg_ptr->setDestActorId(AID_PARTITION);
  actor_msg_ptr->setDestMemberId(ALL_MEMBERS);
  return actor_msg_ptr;
}

void PartitionTableMgr::addListener(PartitionListener* listener) {
  listeners.push_back(listener);
}

void PartitionTableMgr::reBalance() {
  function_footprint();
  for(size_t i = 0, size = partitions.size(); i < size; ++i) {
    int32_t member_id = getMemberId(i, 0);
    if(member_id == -1) {
      continue;
    }
    auto member = member_manager->getMember(member_id);
    if(member->isLocalStore()) {
      member->setPartitionCount(0, member->getPartition(0) + 1);
      member->setPartitionCount(1, member->getPartition(0) * config->max_backup_count());
    }
  }
  auto& memberTable = member_manager->getMemberTable();
  for(auto it = memberTable.begin(); it != memberTable.end(); ++it) {
    auto member = member_manager->getMember(it->getId());
    if (member->isLocalStore() && member->isInactive()) {
      DeltaPartitionEvent evt;
      balance(*member, evt);
    }
  }
}

void PartitionTableMgr::statusChanged(const MemberWrapper& member) {
  auto localMember = member_manager->getLocalMember();
  if(!localMember || !localMember->isLeading()) { // not leading, not handle this event.
    return;
  }
  auto& changedMember = const_cast<MemberWrapper&>(member);
  if(changedMember.isJoined()) { /// join
    if(!changedMember.isLeading()) { /// not leading member join, receive whole partition table firstly.
      auto wholePartitionTable = std::make_shared<WholePartitionEvent>();
      genPartitionTable(*wholePartitionTable->mutable_table());
      multicastPartitionMessage(WHOLE_PARTITION_TABLE, wholePartitionTable);
    }
    if(changedMember.isLocalStore()) { /// local store member, balance partition table
      DeltaPartitionEvent evt;
      balance(changedMember, evt);
    } else { /// not local store member, directly prepared
      member_manager->multicastMemberStatus(changedMember.getId(), PREPARED);
    }
  } else if(changedMember.isLeave()) { /// leave
    if(changedMember.isLeading()) { /// leading leave
      reBalance();
    } else {
      DeltaPartitionEvent evt;
      balance(changedMember, evt);
    }
  }
}

void PartitionTableMgr::handleWholePartitionTableEvent(const std::shared_ptr<ActorMessage>& actor_msg_ptr) {
  function_footprint();
  auto localMember = member_manager->getLocalMember();
  if(!localMember){
    return;
  }
  if(localMember->isJoined()) {
    WholePartitionEvent* evt = dynamic_cast<WholePartitionEvent*>(actor_msg_ptr->getPayload().get());
    for (int i = 0, size = evt->table().partition_size(); i < size; ++i) {
      partitions[i].setPartition(evt->table().partition(i));
    }
  }
  DVLOG(3) << " member " << *localMember << " received whole partition table: " << toString();
}

void PartitionTableMgr::balance(MemberWrapper& member, DeltaPartitionEvent& evt) {
  function_footprint();
  auto members = const_cast<std::vector<MemberWrapper>&>(member_manager->getMemberTable());
  partitionBalancer->balance(members, partitions, member, evt);
  auto deltaEvent = std::make_shared<DeltaPartitionEvent>(evt);
  multicastPartitionMessage(DELTA_PARTITIONS, deltaEvent);
}

void PartitionTableMgr::balanceOk(pb::DeltaPartitionEvent& evt) {
  function_footprint();
  for(int i = 0, size = evt.items_size(); i < size; ++i) {
    // assume finishing migrate
    const size_t partitionId = evt.items(i).partitionid();
    const uint32_t pos = evt.items(i).position();
    const int32_t memberId = evt.items(i).newmemberid();
    partitions[partitionId].setMemberId(pos, memberId);
    partitions[partitionId].setState(pos, true);
  }
}

void PartitionTableMgr::handleDeltaPartitionEvent(const std::shared_ptr<ActorMessage>& msg) {
  function_footprint();
  auto localMember = member_manager->getLocalMember();
  if(!localMember) {
    return;
  }
  auto evt = dynamic_cast<DeltaPartitionEvent*>(msg->getPayload().get());
  DVLOG(7) << evt->DebugString();
  fireEvent(*evt);
  balanceOk(*evt);
  DVLOG(3) << " member " << *localMember << " balance ok! ";
  DVLOG(3) << "after balanced, current partition table: " << toString();
  if(localMember->isJoined()) {
    member_manager->multicastMemberStatus(localMember->getId(), PREPARED);
  }
}

void PartitionTableMgr::genPartitionTable(pb::PartitionTable& table) {
  for (vector<PartitionWrapper>::iterator it = partitions.begin(); it != partitions.end(); ++it) {
    table.add_partition()->CopyFrom(it->getPartition());
  }
}

void PartitionTableMgr::removeListener(PartitionListener* listener) {
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

void PartitionTableMgr::fireEvent(const DeltaPartitionEvent& evt) {
  for(auto it = listeners.begin(); it != listeners.end(); ++it) {
    (*it)->partitionChanged(evt);
  }
}


void PartitionTableMgr::createPartitionTable(size_t partition_count, uint8_t backup_nodes) {
  for(size_t i = 0; i < partition_count; ++i) {
    PartitionWrapper p(backup_nodes);
    partitions.push_back(p);
  }
}

idgs::ResultCode PartitionTableMgr::multicastPartitionMessage(const std::string& message_type,
    std::shared_ptr<google::protobuf::Message> pay_load) {
  std::shared_ptr<ActorMessage> actor_msg_ptr = createActorMessage();
  actor_msg_ptr->setOperationName(message_type);
  actor_msg_ptr->setPayload(pay_load);
  return ::idgs::util::singleton<ClusterFramework>::getInstance().getClusterAdapter()->multicastMessage(actor_msg_ptr);
}

std::string PartitionTableMgr::toString() {
  stringstream s;
  s << "Partition table: " << endl;
  for (int i = 0, size = partitions.size(); i < size; ++i) {
    s << setw(3) << i << " | " << setw(2) << partitions.at(i).getMemberId(0) << " | " << setw(2)
      << partitions.at(i).getMemberId(1) << endl;
  }
  return s.str();
}

} // namespace cluster
} // namespace idgs
