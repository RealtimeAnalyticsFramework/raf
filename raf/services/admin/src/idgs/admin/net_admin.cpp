
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "net_admin.h"
#include "admin_util.h"
#include "idgs/admin/actor/admin_service_actor.h"
#include "idgs/cluster/cluster_framework.h"


using namespace std;
using namespace idgs::actor;
using namespace idgs::net;
using namespace idgs::admin::actor;
using namespace idgs::cluster;
using namespace idgs::util;

namespace idgs {
namespace admin {
namespace net{
bool NetAdmin::init() {
  netStat->init();
  manageableNodes.push_back(netStat);
  return true;
}

bool NetworkStatisticsNode::init() {
  attributesPath.push_back(SINGLE_NETWORK_STATISTICS);

  opMap[SINGLE_NETWORK_STATISTICS] = [this] (OperationContext& context) {
    processSingleNetworkStaticReq(context);
  };

  return true;
}

bool NetworkStatisticsCore::processSingleNetworkStaticReq(AttributePathPtr& attr, string& json, size_t& targetMemberId) {
  std::string member_id;
  attr->getParameterValue(MEMBER_ID_PARAM, member_id);

  targetMemberId = atoi(member_id.c_str());

  uint32_t localMemberId = singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();

  if (localMemberId != targetMemberId) {
    return false;
  }

  static NetworkStatistics* stats = singleton<RpcFramework>::getInstance().getNetwork()->getNetworkStatistics();
  json = stats->toJsonString();
  return true;
}

bool NetworkStatisticsNode::processSingleNetworkStaticReq(OperationContext& context) {
  if (!checkOperationName(context, ADMIN_GET_REQUEST)) {
    return false;
  }

  string netStats;
  size_t targetMemberId=-1;

  if (!NetworkStatisticsCore::processSingleNetworkStaticReq(context.attr, netStats, targetMemberId)) {
    uint32_t localMemberId = singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();
    if (localMemberId != targetMemberId) {
      if (singleton<ClusterFramework>::getInstance().getMemberManager()->findMember(targetMemberId) == NULL) {
        LOG(ERROR) << "can not find target member id: " << targetMemberId;
        std::shared_ptr<idgs::actor::ActorMessage> resposne =
            idgs::admin::util::createAdminResponse(
                context,
                ::idgs::admin::pb::Error,
                 "ccan not find target member id: " + targetMemberId);
        idgs::actor::sendMessage(resposne);
        return false;
      }

      idgs::actor::ActorMessagePtr& actorMsg = context.actorMsg;
      ActorMessagePtr routMsg = actorMsg->createRouteMessage(targetMemberId, ADMIN_ACTOR_ID);
      VLOG(3) << "get Net Admin request [" << actorMsg->toString() << "] will be routed to member " << targetMemberId;
      idgs::actor::sendMessage(routMsg);
    }
    return false;
  }

  std::shared_ptr<idgs::actor::ActorMessage> resposne =
      idgs::admin::util::createAdminResponse(
          context,
          ::idgs::admin::pb::Success,
           netStats);
  idgs::actor::sendMessage(resposne);

  return true;
}

}// net
}// admin
}// idgs
