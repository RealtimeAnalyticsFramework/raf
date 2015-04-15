
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <list>

#include "idgs/cluster/member_event_listener.h"
#include "idgs/actor/stateful_actor.h"
#include "idgs/pb/cluster_event.pb.h"

namespace idgs {
namespace cluster {

class MemberManagerActor: public idgs::actor::StatefulActor {
public:
  MemberManagerActor();
  ~MemberManagerActor();

  virtual idgs::actor::ActorMessagePtr createActorMessage() const override;
  virtual const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

  /**
   * initialize
   */
  void init(idgs::pb::ClusterConfig* cfg);
  void onDestroy() override;

  const std::string& getActorName() const override{
    return getActorId();
  }

  virtual const idgs::actor::ActorDescriptorPtr& getDescriptor() const override {
    return generateActorDescriptor();
  }

  /**
   * reserve member table size
   */
  void reserveMemberSize(size_t size) {
    members.reserve(size);
  }

  /**
   * get member pointer by id
   * @return a member pointer
   * @param memberId member's id
   */
  MemberWrapper* getMember(size_t memberId);

  /// @brief CPG config change event
  ///
  void handleCpgConfigChange(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  /**
   * get member table
   */
  const std::vector<MemberWrapper>& getMemberTable() const { return members; }

  /**
   *  generate current membership table
   */
  void genMembershipTable(idgs::pb::MembershipTable& table);

  /**
   *  register member event listener
   */
  void addListener(MemberEventListener* listener);

  /**
   *  remove registered member event listener
   */
  void removeListener(MemberEventListener* listener);

  /**
   *  return current membership table size
   */
  size_t getMemberSize() const { return members.size(); }

  /**
   *  return local member, if not, return NULL.
   */
  const MemberWrapper* getLocalMember() const;

  /**
   *  return local member index
   */
  const int32_t& getLocalMemberId() const { return localMemberIndex; }

  /**
   *  return can be balanced member size
   */
  size_t getBalanceableMemberSize() const;

  /**
   * add a member in membership table, return member pointer.
   */
  MemberWrapper* addMember(const MemberWrapper& m);

  /**
   *  new join member multicast itself.
   */
  idgs::ResultCode multicastItself(idgs::cluster::MemberWrapper& joined_member);

  /**
   *  handle member leave
   */
  void handleLeaveMembers(const std::vector<MemberWrapper*>& leaveMembers);

  /**
   * check if exists leading leave
   * @return leave leading member pointer if exist, or @return NULL
   */
  MemberWrapper* checkLeadingLeave(const std::vector<MemberWrapper*>& leaveMembers);

  /**
   *  select new leading member
   */
  MemberWrapper* selectLeading(const std::vector<MemberWrapper*>& leaveMembers);

  /**
   *  multicast member status
   */
  idgs::ResultCode multicastMemberStatus(uint32_t member_id, idgs::pb::MemberState status);

  /**
   *  multicast local member status
   */
  idgs::ResultCode multicastLocalMemberStatus(idgs::pb::MemberState status);

  /**
   * find member by node id and process id
   */
  MemberWrapper* findMember(uint32_t node_id, uint32_t pid);

  /**
   * find member by member id
   */
  MemberWrapper* findMember(size_t memberId);

  /**
   * membership table to string
   */
  std::string toString();

  /**
   * membership table to string
   */
  std::string toSimpleString();
  idgs::pb::ClusterConfig* getClusterConfig() const {
    return config;
  }

private:
  /**
   *  generate actor descriptor
   */
  static const ::idgs::actor::ActorDescriptorPtr& generateActorDescriptor();

  /**
   * multicast member message
   */
  idgs::ResultCode multicastMemberMessage(const std::string& msg_type, std::shared_ptr<google::protobuf::Message> payload);

  /**
   *  leading multicast whole member table to new join member.
   */
  idgs::ResultCode multicastWholeMemberTable(const MemberWrapper& joinedMember);

  /**
   *  leading multicast delta member to all members.
   */
  idgs::ResultCode multicastDeltaMember(const std::shared_ptr<idgs::pb::DeltaMemberEvent>& deltaMember);

  /**
   *  handle member status event
   */
  void handleMemberStatusEvent(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  /**
   *  handle new member joined.
   */
  void handleJoinEvent(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  /**
   *  handle leading member joined message event
   */
  void handleLeadingJoinEvent(const MemberWrapper& joined_member);

  /**
   *  handle normal member join/member leave
   */
  void handleNormalJoin(const MemberWrapper& member);

  /**
   *  all member handle delta member message event
   */
  void handleDeltaMemberEvent(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  /**
   *  new joined member handle whole membership message event
   */
  void handleWholeMemberTableEvent(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);


  ///
  /// list member table
  ///
  void handleListMembers(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  ///
  /// get cluster config
  ///
  void handleGetClusterConfig(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  /**
   * leading find the joined position for new joined member
   */
  size_t findAddPos();

  /**
   * add member into membersip table at a position.
   */
  MemberWrapper* addMember(const MemberWrapper& delta_member, size_t pos);

  /**
   * whether local member
   */
  bool isLocalMember(const MemberWrapper& member);

  /**
   * modify member status, trigger status changed event.
   */
  void setMemberStatus(MemberWrapper* member, pb::MemberState status);

private:
  /**
   *  local member's index in membership table
   */
  int32_t localMemberIndex = -1;

  /**
   * cluster's configuration
   */
  idgs::pb::ClusterConfig* config;

  /**
   *  membership table
   */
  std::vector<MemberWrapper> members;

  /**
   *  member event listeners
   */
  std::list<MemberEventListener*> listeners;
}; /// end class MemberManagerActor

}  /// end namespace cluster
}  /// end namespace idgs
