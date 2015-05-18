
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/cluster/member_manager_actor.h"

#include "idgs/application.h"

using namespace idgs::pb;
using namespace idgs::actor;

namespace idgs {
namespace cluster {

MemberManagerActor::MemberManagerActor() : localMemberIndex(-1), config(NULL) {
  actorId = AID_MEMBER; /// special stateful actor id
}

MemberManagerActor::~MemberManagerActor() {
  function_footprint();

}

void MemberManagerActor::onDestroy() {
  // actor::StatefulActor::onDestroy(); // the parent will delete this
  idgs_application()->getRpcFramework()->getActorManager()->unregisterServiceActor(this->getActorId());
  std::list<MemberEventListener*>().swap(this->listeners);

  (const_cast<idgs::actor::ActorMessageHandlerMap&>(getMessageHandlerMap())).clear();
  (const_cast<idgs::actor::ActorDescriptorPtr&>(getDescriptor())).reset();
}

void MemberManagerActor::init(idgs::pb::ClusterConfig* cfg) {
  this->config = cfg;
  // reserve member size
  reserveMemberSize(config->reserved_member_size());

  // register actor framework
  idgs_application()->getRpcFramework()->getActorManager()->registerServiceActor(this->getActorId(), this);
}

const idgs::actor::ActorMessageHandlerMap& MemberManagerActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      { OID_MEMBER, {
          static_cast<idgs::actor::ActorMessageHandler>(&MemberManagerActor::handleJoinEvent),
          &idgs::pb::Member::default_instance()
      }},
      { OID_DELTA_MEMBER_AND_JOIN_POSITION, {
          static_cast<idgs::actor::ActorMessageHandler>(&MemberManagerActor::handleDeltaMemberEvent),
          &idgs::pb::DeltaMemberEvent::default_instance()
      }},
      { OID_WHOLE_MEMBERSHIP_TABLE, {
          static_cast<idgs::actor::ActorMessageHandler>(&MemberManagerActor::handleWholeMemberTableEvent),
          &idgs::pb::WholeMembershipTableEvent::default_instance()
      }},
      { OID_CPG_CONFIG_CHANGE, {
          static_cast<idgs::actor::ActorMessageHandler>(&MemberManagerActor::handleClusterChange),
          &idgs::pb::ClusterChangeEvent::default_instance()
      }},

      { OID_MEMBER_FLAGS, {
          static_cast<idgs::actor::ActorMessageHandler>(&MemberManagerActor::handleMemberFlagsEvent),
          &idgs::pb::MemberFlagsEvent::default_instance()
      }},
      { OID_MEMBER_STATUS, {
          static_cast<idgs::actor::ActorMessageHandler>(&MemberManagerActor::handleMemberStatusEvent),
          &idgs::pb::MemberStatusEvent::default_instance()
      }},
      { OID_LIST_MEMBERS, {
          static_cast<idgs::actor::ActorMessageHandler>(&MemberManagerActor::handleListMembers),
          NULL
      }},
      { OID_GET_CLUSTER_CFG, {
          static_cast<idgs::actor::ActorMessageHandler>(&MemberManagerActor::handleGetClusterConfig),
          NULL
      }}
  };
  return handlerMap;
}

/// this method should be called by module initializer.
const ::idgs::actor::ActorDescriptorPtr& MemberManagerActor::generateActorDescriptor() {
  static idgs::actor::ActorDescriptorPtr descriptor;
  if (descriptor) {
    return descriptor;
  }
  descriptor = std::make_shared<ActorDescriptorWrapper>();
  descriptor->setName(AID_MEMBER);
  descriptor->setDescription("Cluster membership management");
  descriptor->setType(::AT_STATELESS);

  ::ActorOperationDescriporWrapper member_status_evt;
  member_status_evt.setName(OID_MEMBER_STATUS);
  member_status_evt.setDescription("The member's status , sent by member to all members");
  member_status_evt.setPayloadType("idgs.pb.MemberStatusEvent");
  descriptor->setInOperation(member_status_evt.getName(), member_status_evt);
  descriptor->setOutOperation(member_status_evt.getName(), member_status_evt);

  ::ActorOperationDescriporWrapper joined_member_evt;
  joined_member_evt.setName(OID_MEMBER);
  joined_member_evt.setDescription("The new joined member itself without id , sent by new joined member to leading");
  joined_member_evt.setPayloadType("idgs.pb.Member");
  descriptor->setInOperation(joined_member_evt.getName(), joined_member_evt);
  descriptor->setOutOperation(joined_member_evt.getName(), joined_member_evt);

  ::ActorOperationDescriporWrapper delta_joined_member_evt;
  delta_joined_member_evt.setName(OID_DELTA_MEMBER_AND_JOIN_POSITION);
  delta_joined_member_evt.setDescription("The new joined member with id , sent by leading to all members");
  delta_joined_member_evt.setPayloadType("idgs.pb.DeltaMemberEvent");
  descriptor->setInOperation(delta_joined_member_evt.getName(), delta_joined_member_evt);
  descriptor->setOutOperation(delta_joined_member_evt.getName(), delta_joined_member_evt);

  ::ActorOperationDescriporWrapper whole_table_evt;
  whole_table_evt.setName(OID_WHOLE_MEMBERSHIP_TABLE);
  whole_table_evt.setDescription("The membership table sent by leading to new joined member");
  whole_table_evt.setPayloadType("idgs.pb.WholeMembershipTableEvent");
  descriptor->setInOperation(whole_table_evt.getName(), whole_table_evt);
  descriptor->setOutOperation(whole_table_evt.getName(), whole_table_evt);

  // consume itself
  descriptor->addConsumeActor(AID_MEMBER);

  return descriptor;
}

/**
 * override parent create actor message
 */
idgs::actor::ActorMessagePtr MemberManagerActor::createActorMessage() const {
  auto actorMsg = Actor::createActorMessage();
  actorMsg->setDestActorId(AID_MEMBER);
  actorMsg->setDestMemberId(ALL_MEMBERS);
  return actorMsg;
}

idgs::ResultCode MemberManagerActor::mcastMemberFlags(uint32_t member_id, uint64_t flags) {
  std::shared_ptr<MemberFlagsEvent> evt_ptr = std::make_shared<MemberFlagsEvent>();
  evt_ptr->set_member_id(member_id);
  evt_ptr->set_flags(flags);
  return multicastMemberMessage(OID_MEMBER_STATUS, evt_ptr);
}


idgs::ResultCode MemberManagerActor::mcastLocalMemberStatus(idgs::pb::MemberState status) {
  return mcastMemberStatus(getLocalMemberId(), status);
}

idgs::ResultCode MemberManagerActor::mcastMemberStatus(uint32_t member_id, idgs::pb::MemberState status) {
  std::shared_ptr<MemberStatusEvent> evt_ptr = std::make_shared<MemberStatusEvent>();
  evt_ptr->set_member_id(member_id);
  evt_ptr->set_state(status);
  return multicastMemberMessage(OID_MEMBER_STATUS, evt_ptr);
}

idgs::ResultCode MemberManagerActor::multicastItself(idgs::cluster::MemberWrapper& preJoinedMember) {
  std::shared_ptr<Member> msg = std::make_shared<Member>(preJoinedMember.getMember());
  return multicastMemberMessage(OID_MEMBER, msg);
}

MemberWrapper* MemberManagerActor::checkLeadingLeave(const std::vector<MemberWrapper*>& leaveMembers) {
  MemberWrapper* leaveLeadingMember = NULL;
  for(auto it = leaveMembers.begin(); it != leaveMembers.end(); ++it) { // check if exists leading leave
    if ((*it)->isLeading()) {
      leaveLeadingMember = *it;
      break;
    }
  }
  return leaveLeadingMember;
}

MemberWrapper* MemberManagerActor::selectLeading(const std::vector<MemberWrapper*>& leaveMembers) {
  for(auto it = members.begin(); it != members.end(); ++it) {/// select new leading
    bool isLeave = false;
    for(auto jt = leaveMembers.begin(); jt != leaveMembers.end(); ++jt) {
      if(it->getId() == (*jt)->getId()) { /// ignore leave member
        isLeave = true;
        break;
      }
    }
    if(isLeave) {
      continue;
    }
    if(it->getState() == idgs::pb::MS_ACTIVE || it->getState() == idgs::pb::MS_PREPARED) {
      return &members.at(it->getId());
    }
  } /// end for
  return NULL;
}

void MemberManagerActor::handleLeaveMembers(const std::vector<MemberWrapper*>& leaveMembers) {
  function_footprint();
  if(!getLocalMember()) {
    return;
  }
  for(auto it = leaveMembers.begin(); it != leaveMembers.end(); ++it) { // local member exists in left member list, quit, do nothing
    MemberWrapper* leaveMember = *it;
    DVLOG(0) << "Member " << *leaveMember<< " leave";
    if(getLocalMemberId() == leaveMember->getId()) { /// left member itself do nothing
      return;
    }
  }
  /// check if exists leading leave.
  auto leaveLeadingMember = checkLeadingLeave(leaveMembers);
  if(leaveLeadingMember) { /// exists leading leave, select new leading.
    auto newLeadingMember = selectLeading(leaveMembers);
    if(!newLeadingMember) {
      DVLOG(0) << "no member can be selected as leading!";
      return;
    }
    DVLOG(0) << "member " << *newLeadingMember << " is selected as new leading";
    newLeadingMember->setLeading(true);
    setMemberStatus(leaveLeadingMember, MS_INACTIVE); /// will trigger recalculate member owning node counts on partition table.
    leaveLeadingMember->setLeading(false);
    setMemberStatus(leaveLeadingMember, MS_INACTIVE); /// will trigger recalculate member owning node counts on partition table.
    for(auto it = leaveMembers.begin(); it != leaveMembers.end(); ++it) {
      if(leaveLeadingMember == *it) {
        continue;
      }
      setMemberStatus(*it, MS_INACTIVE);
    }
  }
  else { /// normal member leave
    for(auto it = leaveMembers.begin(); it != leaveMembers.end(); ++it) { // local member exists in left member list, quit, do nothing
      setMemberStatus(*it, MS_INACTIVE);
    }
  }
  VLOG(0) << "after handle leave members" <<  toSimpleString();
}

idgs::ResultCode MemberManagerActor::multicastMemberMessage(const std::string& message_type, std::shared_ptr<google::protobuf::Message> payload) {
  std::shared_ptr<ActorMessage> actorMsg = createActorMessage();
  actorMsg->setOperationName(message_type);
  actorMsg->setPayload(payload);
  return idgs_application()->multicastMessage(actorMsg);
}

idgs::ResultCode MemberManagerActor::multicastWholeMemberTable(const MemberWrapper& joinedMember) {
  auto evt = std::make_shared<WholeMembershipTableEvent>();
  evt->mutable_joinedmember()->CopyFrom(joinedMember.getMember());
  genMembershipTable(*evt->mutable_table());
  return multicastMemberMessage(OID_WHOLE_MEMBERSHIP_TABLE, evt);
}

idgs::ResultCode MemberManagerActor::multicastDeltaMember(const std::shared_ptr<DeltaMemberEvent>& deltaMember) {
  return multicastMemberMessage(OID_DELTA_MEMBER_AND_JOIN_POSITION, deltaMember);
}

void MemberManagerActor::handleJoinEvent(const std::shared_ptr<ActorMessage>& actorMsg) {
  function_footprint();
  Member* payload = dynamic_cast<Member*>(actorMsg->getPayload().get());
  MemberWrapper joined_member;
  joined_member.setMember(*payload);
  if (joined_member.isLeading()) { // handle leading member join
    handleLeadingJoinEvent(joined_member);
  } else if (getLocalMember() && getLocalMember()->isLeading()) { // only leading handle normal member join
    handleNormalJoin(joined_member);
  }
}

void MemberManagerActor::handleLeadingJoinEvent(const MemberWrapper& joined_member) {
  function_footprint();
  auto leading = addMember(joined_member);
  // set local member index
  if(isLocalMember(joined_member)) {
    localMemberIndex = leading->getId();
  }
  // joined
  leading->setState(MS_JOINED);
  setMemberStatus(leading, MS_JOINED);
}

void MemberManagerActor::handleNormalJoin(const MemberWrapper& member) {
  // leading send whole member table to new join member.
  ResultCode rs;
  rs = multicastWholeMemberTable(member);
  if (rs != RC_SUCCESS) {
    LOG(ERROR)<< getErrorDescription(rs);
    return;
  }
  // leading send delta member to all members.
  auto deltaMember = std::make_shared<DeltaMemberEvent>();
  auto pos = findAddPos();
  DVLOG(0) << "leading find position " << pos << " for joined member " << member;
  deltaMember->set_position(pos);
  deltaMember->mutable_member()->CopyFrom(member.getMember());
  rs = multicastDeltaMember(deltaMember);
  if (rs != RC_SUCCESS) {
    LOG(ERROR)<< getErrorDescription(rs);
    return;
  }
}

void MemberManagerActor::handleWholeMemberTableEvent(const std::shared_ptr<ActorMessage>& actor_msg_ptr) {
  function_footprint();
  auto evt = dynamic_cast<WholeMembershipTableEvent*>(actor_msg_ptr->getPayload().get());
  MemberWrapper member;
  member.setMember(evt->joinedmember());
  if(!isLocalMember(member)) {
    return;
  }
  for (int i = 0, size = evt->table().member_size(); i < size; ++i) {
    MemberWrapper member;
    member.setMember(evt->table().member(i));
    members.push_back(member);
  }
  VLOG(0) << "joined member " << member << " receive whole membership table" <<  toSimpleString();
}

void MemberManagerActor::handleDeltaMemberEvent(const std::shared_ptr<ActorMessage>& msg) {
  function_footprint();
  if(members.empty()){
    return;
  }
  auto payload = dynamic_cast<DeltaMemberEvent*>(msg->getPayload().get());
  MemberWrapper member;
  member.setMember(payload->member());
  const uint32_t pos = payload->position();
  auto joined_member = addMember(member, pos);
  if(isLocalMember(*joined_member)) {
    localMemberIndex = pos;
  }
  setMemberStatus(joined_member, MS_JOINED);
}

void MemberManagerActor::handleMemberStatusEvent(const std::shared_ptr<ActorMessage>& actor_msg_ptr) {
  function_footprint();
  if (members.empty() || !getLocalMember()) {
    return;
  }
  auto payload = dynamic_cast<MemberStatusEvent*>(actor_msg_ptr->getPayload().get());
  uint32_t pos = payload->member_id();
  auto status = payload->state();
  auto member = getMember(pos);
  if (!member) {
    return;
  }
  setMemberStatus(member, status);
}

/**
 *  handle member flags event
 */
void MemberManagerActor::handleMemberFlagsEvent(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr) {
  function_footprint();
  if (members.empty() || !getLocalMember()) {
    return;
  }
  auto payload = dynamic_cast<MemberFlagsEvent*>(actor_msg_ptr->getPayload().get());
  uint32_t pos = payload->member_id();
  auto flags = payload->flags();
  auto member = getMember(pos);
  if (!member) {
    return;
  }
  member->setFlags(flags);
}

/// @brief CPG config change event
///
void MemberManagerActor::handleClusterChange(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr) {
  idgs::pb::ClusterChangeEvent* event = dynamic_cast<idgs::pb::ClusterChangeEvent*>(actor_msg_ptr->getPayload().get());

  if(event->left_list_size() > 0) { // handle member left
    auto local_member = getLocalMember();
    if(local_member) {
      std::vector<MemberWrapper*> leftMembers;
      for(int i = 0; i < event->left_list_size(); ++i) {
        auto member = findMember(event->left_list(i).nodeid(), event->left_list(i).pid());
        if(!member) {
          continue;
        }
        leftMembers.push_back(member);
      }
      handleLeaveMembers(leftMembers);
    }
  }
  if(event->joined_list_size() > 0) { // handle new member joined
    auto cfg_member = config->mutable_member();
    DVLOG(2) << "config member: " << cfg_member->DebugString();
    for (int j = 0; j < event->joined_list_size(); ++j) {
      if(event->joined_list_size() >= event->member_list_size() && j == 0) {
        cfg_member->set_flags(cfg_member->flags() | idgs::pb::MF_LEADING);
      }
      if(cfg_member->node_id() == event->joined_list(j).nodeid() && cfg_member->pid() == event->joined_list(j).pid()) {
        // send itself to leading
        MemberWrapper joinedMember;
        joinedMember.setMember(*cfg_member);
        DVLOG(0) << "joined member " << joinedMember << " multicast itself";
        multicastItself(joinedMember);
      }
    }
  }
}

///
/// list member table
///
void MemberManagerActor::handleListMembers(const std::shared_ptr<idgs::actor::ActorMessage>& msg) {
  auto result = msg->createResponse();

  std::shared_ptr<MembershipTable> mt = std::make_shared<MembershipTable>();
  genMembershipTable(*mt);

  result->setPayload(mt);
  result->setOperationName("member_list");
  idgs::actor::sendMessage(result);
}

///
/// get cluster config
///
void MemberManagerActor::handleGetClusterConfig(const std::shared_ptr<idgs::actor::ActorMessage>& msg) {
  auto result = msg->createResponse();

  std::shared_ptr<idgs::pb::ClusterConfig> cc = std::make_shared<idgs::pb::ClusterConfig>();
  cc->CopyFrom(*config);

  result->setPayload(cc);
  result->setOperationName("cluster_cfg");
  idgs::actor::sendMessage(result);
}


bool MemberManagerActor::isLocalMember(const MemberWrapper& member) {
  auto& cfgMember = this->config->member();
  return cfgMember.public_address().host() == member.getMember().public_address().host()
      && cfgMember.public_address().port() == member.getMember().public_address().port() ;
}

void MemberManagerActor::setMemberStatus(MemberWrapper* member, pb::MemberState status) {
  member->setState(status);
  for(auto it = listeners.begin(); it != listeners.end(); ++it) {
    (*it)->memberStatusChanged(*member);
  }
  VLOG(0) << "after member " << *member << toSimpleString();
}

std::string MemberManagerActor::toSimpleString() {
  std::stringstream s;
  const size_t size = getMemberSize();
  s << "\n=============== membership table ===============\n";
  for (size_t i = 0; i < size; ++i) {
    s << members[i];
    if(members[i].getId() == localMemberIndex) {
      s << ", " << "local";
    }
    s << "\n";
  }
  return s.str();
}

std::string MemberManagerActor::toString() {
  std::stringstream s;
  const size_t size = getMemberSize();
  s << "\n=============== membership table ===============\n";
  for (int i = 0; i < size; ++i) {
    s << members[i].toString();
  }
  return s.str();
}

void MemberManagerActor::removeListener(MemberEventListener* listener) {
  for (auto it = listeners.begin(); it != listeners.end();) {
    if (*it == listener) {
      listeners.erase(it);
      break;
    } else {
      ++it;
    }
  }
}

void MemberManagerActor::addListener(MemberEventListener* listener) {
  listeners.push_back(listener);
}

size_t MemberManagerActor::findAddPos() {
  const size_t len = members.size();
  int32_t pos = len;
  for (int i = 0; i < len; ++i) {
    if (members.at(i).getState() == idgs::pb::MS_INACTIVE) {
      pos = i;
      members.at(i).setState(MS_TAKEN);/// temporay hold this position
      break;
    }
  }
  if (pos == len) {
    members.resize(len + 1);
    members.at(len).setState(MS_TAKEN);
  }
  return pos;
}

MemberWrapper* MemberManagerActor::addMember(const MemberWrapper& m) {
  return addMember(m, findAddPos());
}

MemberWrapper* MemberManagerActor::addMember(const MemberWrapper& m, size_t pos) {
  if (pos >= members.size()) {
    members.resize(pos + 1);
  }
  members[pos] = m;
  members[pos].setId(pos);
  return &members[pos];
}

MemberWrapper* MemberManagerActor::getMember(size_t memberId) {
  if (members.empty() || memberId > members.size()) {
    return NULL;
  }
  return &members[memberId];
}

MemberWrapper* MemberManagerActor::findMember(size_t memberId) {
  for (auto it = members.begin(); it != members.end(); ++it) {
    if (it->getId() == memberId) {
      return &(*it);
    }
  }
  return NULL;
}

MemberWrapper* MemberManagerActor::findMember(uint32_t node_id, uint32_t pid) {
  for (auto it = members.begin(); it != members.end(); ++it) {
    if (it->getNodeId() == node_id && it->getPid() == pid) {
      return &(*it);
    }
  }
  return NULL;
}

void MemberManagerActor::genMembershipTable(MembershipTable& mt) {
  for (auto it = members.begin(); it != members.end(); ++it) {
    mt.add_member()->CopyFrom(it->getMember());
  }
}

size_t MemberManagerActor::getBalanceableMemberSize() const {
  size_t sum = 0;
  for (auto& m : members) {
    if (m.isAvailable()) {
      ++sum;
    }
  }
  return sum;
}

const MemberWrapper* MemberManagerActor::getLocalMember() const {
  if (localMemberIndex < 0 || members.empty() || localMemberIndex >= members.size()) {
    return NULL;
  }
  return &members.at(localMemberIndex);
}

} // end namespace cluster
} // end namespace idgs
