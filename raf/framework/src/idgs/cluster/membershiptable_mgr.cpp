
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/cluster/membershiptable_mgr.h"

#include "idgs/application.h"

using namespace std;
using namespace idgs::pb;
using namespace idgs::actor;
using namespace idgs::pb;
using namespace idgs::actor;

namespace idgs {

namespace cluster {

MembershipTableMgr::MembershipTableMgr() :
    localIndex(-1), config(NULL) {
  joining.store(false);
  setActorId(AID_MEMBER);
}

MembershipTableMgr::~MembershipTableMgr() {
  function_footprint();
}

void MembershipTableMgr::onDestroy() {
  list<MemberEventListener*>().swap(this->listeners);
  function_footprint();
}

void MembershipTableMgr::init(idgs::pb::ClusterConfig* cfg) {
  this->config = cfg;
  // reserve member size
  reserveMemberSize(config->reserved_member_size());
  // register actor framework
  registerActorFramework();
}

const idgs::actor::ActorMessageHandlerMap& MembershipTableMgr::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {MEMBER,                         static_cast<idgs::actor::ActorMessageHandler>(&MembershipTableMgr::handleJoinedEvent)},
      {DELTA_MEMBER_AND_JOIN_POSITION, static_cast<idgs::actor::ActorMessageHandler>(&MembershipTableMgr::handleDeltaMemberEvent)},
      {WHOLE_MEMBERSHIP_TABLE,         static_cast<idgs::actor::ActorMessageHandler>(&MembershipTableMgr::handleWholeTableEvent)},
      {MEMBER_STATUS,                  static_cast<idgs::actor::ActorMessageHandler>(&MembershipTableMgr::handleMemberStatusEvent)},
  };
  return handlerMap;
}


void MembershipTableMgr::registerActorFramework() {
  this->descriptor = generateActorDescriptor();

  ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework()->Register(this->getActorId(), this);
  // end Register actor operation map
}

/**
 * override parent create actor message
 */
std::shared_ptr<ActorMessage> MembershipTableMgr::createActorMessage() {
  std::shared_ptr<ActorMessage> actor_msg_ptr = super::createActorMessage();
  actor_msg_ptr->setDestActorId(AID_MEMBER);
  actor_msg_ptr->setDestMemberId(ALL_MEMBERS);
  return actor_msg_ptr;
}


idgs::ResultCode MembershipTableMgr::multicastLocalMemberStatus(idgs::pb::MemberStatus status) {
  return multicastMemberStatus(getLocalMemberId(), status);
}

idgs::ResultCode MembershipTableMgr::multicastMemberStatus(uint32_t member_id, idgs::pb::MemberStatus status) {
  shared_ptr<MemberStatusEvent> evt_ptr(new MemberStatusEvent);
  evt_ptr->set_member_id(member_id);
  evt_ptr->set_status(status);
  return multicastMemberMessage(MEMBER_STATUS, evt_ptr);
}

idgs::ResultCode MembershipTableMgr::memberJoined(idgs::cluster::MemberWrapper& preJoinedMember) {
  shared_ptr<Member> msg(new Member(preJoinedMember.getMember()));
  return multicastMemberMessage(MEMBER, msg);
}

MemberWrapper* MembershipTableMgr::selectLeading(const std::vector<MemberWrapper*>& leftMembers) {
  for(auto it = members.begin(); it != members.end(); ++it) {/// select new leading
    bool isLeave = false;
    for(auto jt = leftMembers.begin(); jt != leftMembers.end(); ++jt) {
      if(it->getId() == (*jt)->getId()) { /// ignore leave member
        isLeave = true;
        break;
      }
    }
    if(isLeave) {
      continue;
    }
    if(it->isActive() || it->isPrepared()) {
      return &members.at(it->getId());
      break;
    }
  } /// end for
  return NULL;
}

idgs::ResultCode MembershipTableMgr::memberLeft(const std::vector<MemberWrapper*>& leftMembers) {
  function_footprint();
  if (!getLocalMember()) {
    return RC_OK;
  }
  for(auto it = leftMembers.begin(); it != leftMembers.end(); ++it) { // local member exists in left member list, quit, do nothing
    if(getLocalMemberId() == (*it)->getId()) { ///left member itself, do nothing
      return RC_OK;
    }
  }
  MemberWrapper* leaveLeadingMember = NULL;
  for(auto it = leftMembers.begin(); it != leftMembers.end(); ++it) { // check if exists leading leave
    if ((*it)->isLeading()) {
      leaveLeadingMember = *it;
      break;
    }
  }
  if(leaveLeadingMember) { /// leading member leave
    MemberWrapper* newLeadingMember = selectLeading(leftMembers);
    if(!newLeadingMember) { // no member can be selected as leading,quit!
      return RC_OK;
    }
    DVLOG(0) << "Member " << *newLeadingMember << " is selected as leading";
    newLeadingMember->setIsleading(true);
    leaveLeadingMember->setIsleading(false);
    for(auto it = leftMembers.begin(); it != leftMembers.end(); ++it) {
      setMemberStatus(*it, INACTIVE);
    }
  } /// end if
  else { /// normal member leave
    for(auto it = leftMembers.begin(); it != leftMembers.end(); ++it) { // local member exists in left member list, quit, do nothing
      setMemberStatus(*it, INACTIVE);
    }
  }
  LOG(INFO) << toSimpleString();
  return RC_OK;
}

idgs::ResultCode MembershipTableMgr::multicastMemberMessage(const std::string& message_type, std::shared_ptr<google::protobuf::Message> payload) {
  std::shared_ptr<ActorMessage> actor_msg_ptr = createActorMessage();
  actor_msg_ptr->setOperationName(message_type);
  actor_msg_ptr->setPayload(payload);
  return ::idgs::util::singleton<ClusterFramework>::getInstance().getClusterAdapter()->multicastMessage(actor_msg_ptr);
}

// this method should be called by module initializer.
std::shared_ptr<ActorDescriptorWrapper> MembershipTableMgr::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor)
    return descriptor;
  descriptor.reset(new ::ActorDescriptorWrapper);
  descriptor->setName(AID_MEMBER);
  descriptor->setDescription("Cluster membership management");
  descriptor->setType(::AT_STATELESS);

  ::ActorOperationDescriporWrapper member_status_evt;
  member_status_evt.setName(MEMBER_STATUS);
  member_status_evt.setDescription("The member's status , sent by member to all members");
  member_status_evt.setPayloadType("idgs.pb.MemberStatusEvent");
  descriptor->setInOperation(member_status_evt.getName(), member_status_evt);
  descriptor->setOutOperation(member_status_evt.getName(), member_status_evt);

  ::ActorOperationDescriporWrapper joined_member_evt;
  joined_member_evt.setName(MEMBER);
  joined_member_evt.setDescription("The new joined member itself without id , sent by new joined member to leading");
  joined_member_evt.setPayloadType("idgs.pb.Member");
  descriptor->setInOperation(joined_member_evt.getName(), joined_member_evt);
  descriptor->setOutOperation(joined_member_evt.getName(), joined_member_evt);

  ::ActorOperationDescriporWrapper delta_joined_member_evt;
  delta_joined_member_evt.setName(DELTA_MEMBER_AND_JOIN_POSITION);
  delta_joined_member_evt.setDescription("The new joined member with id , sent by leading to all members");
  delta_joined_member_evt.setPayloadType("idgs.pb.DeltaMemberEvent");
  descriptor->setInOperation(delta_joined_member_evt.getName(), delta_joined_member_evt);
  descriptor->setOutOperation(delta_joined_member_evt.getName(), delta_joined_member_evt);

  ::ActorOperationDescriporWrapper whole_table_evt;
  whole_table_evt.setName(WHOLE_MEMBERSHIP_TABLE);
  whole_table_evt.setDescription("The membership table sent by leading to new joined member");
  whole_table_evt.setPayloadType("idgs.pb.WholeMembershipTableEvent");
  descriptor->setInOperation(whole_table_evt.getName(), whole_table_evt);
  descriptor->setOutOperation(whole_table_evt.getName(), whole_table_evt);

  // consume itself
  descriptor->addConsumeActor(AID_MEMBER);

  return descriptor;
}

void MembershipTableMgr::handleJoinedEvent(const std::shared_ptr<ActorMessage>& actor_msg_ptr) {
  function_footprint();
  Member* payload = dynamic_cast<Member*>(actor_msg_ptr->getPayload().get());
  MemberWrapper joined_member;
  joined_member.setMember(*payload);
  if (joined_member.isLeading()) { // handle leading member join
    handleLeadingJoinEvent(joined_member);
  } else if (getLocalMember() && getLocalMember()->isLeading()) { // only leading handle normal member join
    handleNormalJoinEvent(joined_member);
  }
}

void MembershipTableMgr::handleLeadingJoinEvent(const MemberWrapper& joined_member) {
  function_footprint();
  auto leading = addMember(joined_member);
  // set local member index
  localIndex = leading->getId();
  // joined
  leading->setStatus(JOINED);
  setMemberStatus(leading, JOINED);
}

void MembershipTableMgr::handleNormalJoinEvent(const MemberWrapper& member) {
  function_footprint();
  queue.push(member);
  if(joining) {
    DVLOG(0) << "$$$$$$$$$$$$$$$$$$$$$$$$$$";
    DVLOG(0) << "$$$$$$$$$$$$$$$$$$$$$$$$$$";
    DVLOG(0) << "current joining member queue size: " << queue.unsafe_size();
    DVLOG(0) << "$$$$$$$$$$$$$$$$$$$$$$$$$$";
    DVLOG(0) << "$$$$$$$$$$$$$$$$$$$$$$$$$$";
    return;
  }
  while(!queue.empty()) {
    joining.store(true);
    MemberWrapper joined_member;
    queue.try_pop(joined_member);
    // leading generate whole member table and send to joining member.
    auto wholeMemberTable = std::make_shared<WholeMembershipTableEvent>();
    genMembershipTable(*wholeMemberTable->mutable_table());
    ResultCode rs;
    rs = multicastMemberMessage(WHOLE_MEMBERSHIP_TABLE, wholeMemberTable);
    if (rs != RC_SUCCESS) {
      LOG(ERROR)<< getErrorDescription(rs);
      return;
    }
    // leading send delta member to all members.
    auto deltaMember = std::make_shared<DeltaMemberEvent>();
    deltaMember->set_position(findAddPos());
    deltaMember->mutable_member()->CopyFrom(joined_member.getMember());
    rs = multicastMemberMessage(DELTA_MEMBER_AND_JOIN_POSITION, deltaMember);
    if (rs != RC_SUCCESS) {
      LOG(ERROR)<< getErrorDescription(rs);
      return;
    }
  }
}

void MembershipTableMgr::handleWholeTableEvent(const std::shared_ptr<ActorMessage>& actor_msg_ptr) {
  function_footprint();
  if(!members.empty()) {
    return;
  }
  auto evt = dynamic_cast<WholeMembershipTableEvent*>(actor_msg_ptr->getPayload().get());
  for (int i = 0, size = evt->table().member_size(); i < size; ++i) {
    MemberWrapper member;
    member.setMember(evt->table().member(i));
    members.push_back(member);
  }
  LOG(INFO)<< toSimpleString();
}

void MembershipTableMgr::handleDeltaMemberEvent(const std::shared_ptr<ActorMessage>& msg) {
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
    localIndex = pos;
  }
  setMemberStatus(joined_member, JOINED);
}

void MembershipTableMgr::handleMemberStatusEvent(const std::shared_ptr<ActorMessage>& actor_msg_ptr) {
  function_footprint();
  if (members.empty() || !getLocalMember()) {
    return;
  }
  auto payload = dynamic_cast<MemberStatusEvent*>(actor_msg_ptr->getPayload().get());
  uint32_t pos = payload->member_id();
  auto status = payload->status();
  auto member = getMember(pos);
  if (!member) {
    return;
  }
  setMemberStatus(member, status);
  if(member->isPrepared()) {
    DVLOG(0) << toSimpleString();
    joining.store(false);
  }
}

bool MembershipTableMgr::isLocalMember(const MemberWrapper& member) {
  auto& cfgMember = this->config->member();
  return cfgMember.publicaddress().host() == member.getMember().publicaddress().host()
      && cfgMember.publicaddress().port() == member.getMember().publicaddress().port() ;
}

void MembershipTableMgr::setMemberStatus(MemberWrapper* member, pb::MemberStatus status) {
  member->setStatus(status);
  for(auto it = listeners.begin(); it != listeners.end(); ++it) {
    (*it)->statusChanged(*member);
  }
}

std::string MembershipTableMgr::toSimpleString() {
  stringstream s;
  const size_t size = getMemberSize();
  s << "\n=============== membership table ===============\n";
  for (size_t i = 0; i < size; ++i) {
    s << members[i];
    if(members[i].getId() == localIndex) {
      s << ", " << "local";
    }
    s << "\n";
  }
  return s.str();
}

std::string MembershipTableMgr::toString() {
  stringstream s;
  const size_t size = getMemberSize();
  s << "\n=============== membership table ===============\n";
  for (int i = 0; i < size; ++i) {
    s << members[i].toString();
  }
  return s.str();
}

void MembershipTableMgr::removeListener(MemberEventListener* listener) {
  list<MemberEventListener*>::iterator it;
  for (it = listeners.begin(); it != listeners.end();) {
    if (*it == listener) {
      listeners.erase(it);
      break;
    } else {
      ++it;
    }
  }
}

void MembershipTableMgr::addListener(MemberEventListener* listener) {
  listeners.push_back(listener);
}

size_t MembershipTableMgr::findAddPos() {
  const size_t len = members.size();
  int32_t pos = len;
  for (int i = 0; i < len; ++i) {
    if (members.at(i).isLeave()) {
      pos = i;
      break;
    }
  }
  if (pos == len) {
    members.resize(len + 1);
  }
  return pos;
}

MemberWrapper* MembershipTableMgr::addMember(const MemberWrapper& m) {
  return addMember(m, findAddPos());
}

MemberWrapper* MembershipTableMgr::addMember(const MemberWrapper& m, size_t pos) {
  if (pos >= members.size()) {
    members.resize(pos + 1);
  }
  members[pos] = m;
  members[pos].setId(pos);
//      DVLOG(5) << toString();
  return &members[pos];
}

MemberWrapper* MembershipTableMgr::getMember(size_t memberId) {
  if (members.empty() || memberId > members.size()) {
    return NULL;
  }
  return &members[memberId];
}

MemberWrapper* MembershipTableMgr::findMember(size_t memberId) {
  for (vector<MemberWrapper>::iterator it = members.begin(); it != members.end(); ++it) {
    if (it->getId() == memberId) {
      return &(*it);
    }
  }
  return NULL;
}

MemberWrapper* MembershipTableMgr::findMember(uint32_t node_id, uint32_t pid) {
  for (vector<MemberWrapper>::iterator it = members.begin(); it != members.end(); ++it) {
    if (it->getNodeId() == node_id && it->getPid() == pid) {
      return &(*it);
    }
  }
  return NULL;
}

void MembershipTableMgr::genMembershipTable(MembershipTable& mt) {
  vector<MemberWrapper>::iterator it;
  for (it = members.begin(); it != members.end(); ++it) {
    mt.add_member()->CopyFrom(it->getMember());
  }
}

size_t MembershipTableMgr::getBalanceableMemberSize() const {
  const size_t memberSize = members.size();
  size_t sum = 0;
  for (int i = 0; i < memberSize; ++i) {
    if (members.at(i).canBalanced()) {
      ++sum;
    }
  }
  return sum;
}

const MemberWrapper* MembershipTableMgr::getLocalMember() const {
  if (localIndex < 0 || members.empty() || localIndex >= members.size()) {
    return NULL;
  }
  return &members.at(localIndex);
}

} // end namespace cluster
} // end namespace idgs
