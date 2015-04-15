/*
 * rdd_cluster_listener.cpp
 *
 *  Created on: Jan 21, 2015
 *      Author: root
 */

#include "rdd_member_event_listener.h"

#include "idgs/application.h"

#include "idgs/rdd/rdd_const.h"

#include "idgs/rdd/pb/rdd_internal.pb.h"

namespace idgs {
namespace rdd {

RddMemberEventListener::RddMemberEventListener() {
}

RddMemberEventListener::~RddMemberEventListener() {
}

void RddMemberEventListener::memberStatusChanged(const idgs::cluster::MemberWrapper& member) {
  if (member.isLocalStore()) {
    auto state = member.getState();
    if (state == idgs::pb::MS_ACTIVE || state == idgs::pb::MS_PREPARED || state == idgs::pb::MS_INACTIVE) {
      auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
      auto msg = std::make_shared<idgs::actor::ActorMessage>();
      msg->setOperationName(MEMBER_CHANGE_EVENT);
      msg->setSourceActorId(RDD_INTERNAL_SERVICE_ACTOR);
      msg->setSourceMemberId(localMemberId);
      msg->setDestActorId(RDD_INTERNAL_SERVICE_ACTOR);
      msg->setDestMemberId(localMemberId);
      msg->setPayload(std::make_shared<idgs::rdd::pb::RddRequest>());

      idgs::actor::sendMessage(msg);
    }
  }
}

} /* namespace rdd */
} /* namespace idgs */
