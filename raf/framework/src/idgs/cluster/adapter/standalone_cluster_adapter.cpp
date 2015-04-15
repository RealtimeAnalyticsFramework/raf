/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "standalone_cluster_adapter.h"
#include "idgs/application.h"

#define LOCAL_NODE_ID     0

namespace idgs {
namespace cluster {

StandaloneClusterAdapter::StandaloneClusterAdapter() {
}

StandaloneClusterAdapter::~StandaloneClusterAdapter() {
}

///
/// Initialize corosync cluster engine
///
ResultCode StandaloneClusterAdapter::init(idgs::pb::ClusterConfig* config) {
  auto member = config->mutable_member();
  // set member's node id and process id
  member->set_node_id(LOCAL_NODE_ID);
  member->set_pid(getpid());

  return RC_OK;
}

///
/// Start a member
///
ResultCode StandaloneClusterAdapter::start() {
  LOG(INFO) << "starting standalone cluster";
  actor::ActorMessagePtr msg = std::make_shared<actor::ActorMessage>();
  msg->getRpcMessage();

  std::shared_ptr<google::protobuf::Message> ev = std::make_shared<idgs::pb::CpgConfigChangeEvent>();
  msg->setPayload(ev);
  msg->setOperationName(OID_CPG_CONFIG_CHANGE);
  msg->setSourceActorId(AID_MEMBER);
  msg->setDestActorId(AID_MEMBER);
  msg->setSourceMemberId(idgs::pb::ANY_MEMBER);
  msg->setDestMemberId(idgs::pb::ANY_MEMBER);


  idgs::pb::CpgConfigChangeEvent* event = dynamic_cast<idgs::pb::CpgConfigChangeEvent*>(ev.get());

  idgs::pb::CpgAddress selfMember;
  selfMember.set_nodeid(LOCAL_NODE_ID);
  selfMember.set_pid(getpid());
  selfMember.set_reason(0);

  // all member
  *event->add_member_list() = selfMember;

  // joining member
  *event->add_joined_list() = selfMember;

  auto memberManager = idgs_application()->getMemberManager();
  memberManager->handleCpgConfigChange(msg);

  return RC_OK;
}

///
/// Member quit from cluster engine
///
void StandaloneClusterAdapter::stop() {
}


///
/// Multicast actor message
/// @param actorMsg sent actor message
///
idgs::ResultCode StandaloneClusterAdapter::multicastMessage(const std::shared_ptr<idgs::actor::ActorMessage>& actorMsg) {
  idgs::actor::relayMessage(const_cast<std::shared_ptr<idgs::actor::ActorMessage>&>(actorMsg));
  return RC_OK;
}



} // namespace cluster
} // namespace idgs
