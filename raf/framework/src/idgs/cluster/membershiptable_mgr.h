
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <list>
#include "idgs/cluster/member_event_listener.h"
#include "idgs/cluster/member_wrapper.h"
#include "idgs/actor/stateless_actor.h"

#include <tbb/tbb.h>

namespace idgs {

namespace cluster {

class MembershipTableMgr: public idgs::actor::StatelessActor {

  typedef idgs::actor::StatelessActor super;

public:

  MembershipTableMgr();

  ~MembershipTableMgr();

  virtual const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

  void onDestroy() override;

  /**
   * Initialize membership manager
   */
  void init(idgs::pb::ClusterConfig* cfg);

  /**
   * Reserve member's size
   */
  void reserveMemberSize(size_t size) {
    members.reserve(size);
  }

  /**
   *  Override parent create actor message
   */
  std::shared_ptr<idgs::actor::ActorMessage> createActorMessage();

  /**
   * Get member by ID
   * @return a member pointer
   * @param memberId member's ID
   */
  MemberWrapper* getMember(size_t memberId);

  /**
   * get member table
   */
  const std::vector<MemberWrapper>& getMemberTable() const {
    return members;
  }

  /**
   *  Generate current membership table
   */
  void genMembershipTable(idgs::pb::MembershipTable& table);

  /**
   *  Register a member listener
   */
  void addListener(MemberEventListener* listener);

  /**
   *  Remove the registered member listener
   */
  void removeListener(MemberEventListener* listener);
  /**
   *  Return whole membership table size, including leaving members
   */
  size_t getMemberSize() const {
    return members.size();
  }

  /**
   *  Return local member pointer
   *  @return local member const pointer
   */
  const MemberWrapper* getLocalMember() const;

  /**
   *  Get local member ID
   *  @return the local member ID
   */
  int32_t getLocalMemberId() const {
    return localIndex;
  }

  /**
   *  Return active member size
   */
//			size_t getActiveMemberSize() const;
  /**
   *  Return inactive member size
   */
  size_t getBalanceableMemberSize() const;

  /**
   * Add a new joined member
   * @return added member pointer
   */
  MemberWrapper* addMember(const MemberWrapper& m);

  /**
   *  handle member joined
   */
  idgs::ResultCode memberJoined(idgs::cluster::MemberWrapper& joined_member);

  /**
   *  handle member left
   */
  idgs::ResultCode memberLeft(const std::vector<MemberWrapper*>& leftMembers);

  /**
   *  select new leading member
   */
  MemberWrapper* selectLeading(const std::vector<MemberWrapper*>& leftMembers);

  /**
   *  Multicast member status
   */
  idgs::ResultCode multicastMemberStatus(uint32_t member_id, idgs::pb::MemberStatus status);

  /**
   *  Multicast member status
   */
  idgs::ResultCode multicastLocalMemberStatus(idgs::pb::MemberStatus status);

  /**
   * Find member by node id and process id
   */
  MemberWrapper* findMember(uint32_t node_id, uint32_t pid);

  MemberWrapper* findMember(size_t memberId);

  /**
   * Membership table toString
   */
  std::string toString();

  /**
   * Membership table toString
   */
  std::string toSimpleString();

  /**
   *  Generate actor descriptor
   */
  static ::idgs::actor::ActorDescriptorPtr generateActorDescriptor();

private:

  /**
   *  local member's index in membership table
   */
  int32_t localIndex = -1;

  /**
   * Pointer indirect to Cluster's config file
   */
  pb::ClusterConfig* config;

  /**
   *  Membership table container
   */
  std::vector<MemberWrapper> members;

  /**
   *  All registered listeners
   */
  std::list<MemberEventListener*> listeners;

  /**
   * joined member queue
   */
  tbb::concurrent_queue<MemberWrapper> queue;

  /**
   * some member is joining, default false
   */
  tbb::atomic<bool> joining;

  /**
   *  Register ActorDescriptor and Actor
   */
  void registerActorFramework();

  /**
   * Multicast member related message
   * @param message_type message type
   * @param actor_msg actor message
   */
  idgs::ResultCode multicastMemberMessage(const std::string& msg_type, std::shared_ptr<google::protobuf::Message> payload);

  /**
   *  handle member status event
   */
  void handleMemberStatusEvent(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  /**
   *  Leading handle new member joined message event
   */
  void handleJoinedEvent(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  /**
   *  Leading handle leading member joined message event
   */
  void handleLeadingJoinEvent(const MemberWrapper& joined_member);

  /**
   *  Leading handle normal member joined message event
   */
  void handleNormalJoinEvent(const MemberWrapper& joined_member);

  /**
   *  All member handle delta member message event
   */
  void handleDeltaMemberEvent(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  /**
   *  New joined member handle whole membership message event
   */
  void handleWholeTableEvent(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  /**
   * Leading Find the joined position for new joined member
   */
  size_t findAddPos();

  /**
   * All member add the new joined member into membership table
   * @param pos joined position
   * @return pointer pointed to the new joined member
   */
  MemberWrapper* addMember(const MemberWrapper& delta_member, size_t pos);

  /**
   * judge whether is local member
   */
  bool isLocalMember(const MemberWrapper& member);

  /**
   * modify member status, trigger status changed event.
   */
  void setMemberStatus(MemberWrapper* member, pb::MemberStatus status);

};
// end class MembershipTableMgr
}// end namespace cluster
} // end namespace idgs
