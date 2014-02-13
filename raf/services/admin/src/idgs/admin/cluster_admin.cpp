
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "cluster_admin.h"
#include "admin_util.h"
#include "idgs/admin/actor/admin_service_actor.h"
#include "idgs/application.h"
#include "idgs/cluster/cluster_framework.h"
#include "idgs/util/backtrace.h"


using namespace std;
using namespace idgs::admin::actor;
using namespace idgs::cluster;

namespace idgs {
namespace admin {
namespace cluster {

bool ClusterAdmin::init() {
  memberShip->init();
  manageableNodes.push_back(memberShip);

  partitionMgr->init();
  manageableNodes.push_back(partitionMgr);

  actorNode->init();
  manageableNodes.push_back(actorNode);
  return true;
}

bool PartionManagableNode::init() {
  attributesPath.push_back(PARTITION_TABLE);

  opMap[PARTITION_TABLE] = [this] (OperationContext& context) {
    processGetPartitionTableReq(context);
  };

  return true;
}

bool PartionManagableNode::processGetPartitionTableReq(OperationContext& context) {
  if (!checkOperationName(context, ADMIN_GET_REQUEST)) {
    return false;
  }

  idgs::Application& app = ::idgs::util::singleton<idgs::Application>::getInstance();
  PartitionTableMgr* pm = app.getPartitionManager();

  idgs::pb::PartitionTable table;
  pm->genPartitionTable(table);

  DVLOG(4) << "get Partition Table:\n" << table.DebugString();

  string jsonBody = protobuf::JsonMessage::toJsonString(&table);
  DVLOG(3) << "get json for partition table :\n" << jsonBody;

  std::shared_ptr<idgs::actor::ActorMessage> resposne =
      idgs::admin::util::createAdminResponse(context,
          ::idgs::admin::pb::Success,
           jsonBody);

  idgs::actor::sendMessage(resposne);

  return true;
}

//////////////////// MembershipManagableNode ///////////////////////////

bool MembershipManagableNode::init() {
  attributesPath.push_back(ALL_MEMBERS_ID);
  attributesPath.push_back(MEMBER_INFO);

  opMap[ALL_MEMBERS_ID] = [this] (OperationContext& context) {
    processAllMembersInfoReq(context);
  };

  opMap[MEMBER_INFO] = [this] (OperationContext& context) {
    processMemberInfoReq(context);
  };

  return true;
}

bool MembershipManagableNode::processAllMembersInfoReq(OperationContext& context) {
  if (!checkOperationName(context, ADMIN_GET_REQUEST)) {
    return false;
  }

  idgs::Application& app = ::idgs::util::singleton<idgs::Application>::getInstance();
  idgs::pb::MembershipTable table;
  app.getMemberManager()->genMembershipTable(table);

  string memsInfoJson = protobuf::JsonMessage::toJsonString(&table);

  std::shared_ptr<idgs::actor::ActorMessage> resposne =
      idgs::admin::util::createAdminResponse(context,
          ::idgs::admin::pb::Success,
          memsInfoJson);

  idgs::actor::sendMessage(resposne);

  return true;
}

bool MembershipManagableNode::processMemberInfoReq(OperationContext& context) {
  idgs::admin::AttributePathPtr& attr = context.attr;
  if (!checkOperationName(context, ADMIN_GET_REQUEST)) {
    return false;
  }

  string memInfoJson;
  idgs::Application& app = ::idgs::util::singleton<idgs::Application>::getInstance();

  std::string member_id;
  attr->getParameterValue(MEMBER_ID_PARAM, member_id);
  DVLOG(3) << "get member id: " << member_id;
  if (app.getMemberManager()->findMember(atoi(member_id.c_str())) == NULL) {
    std::shared_ptr<idgs::actor::ActorMessage> resposne =
        idgs::admin::util::createAdminResponse(context,
            ::idgs::admin::pb::Error,
            "can not find member " + member_id);

    idgs::actor::sendMessage(resposne);
    return false;
  }

  idgs::pb::MembershipTable table;
  app.getMemberManager()->genMembershipTable(table);
  const ::idgs::pb::Member& memberInfo = table.member(atoi(member_id.c_str()));
  memInfoJson = protobuf::JsonMessage::toJsonString(&memberInfo);

  std::shared_ptr<idgs::actor::ActorMessage> resposne =
      idgs::admin::util::createAdminResponse(
          context,
          ::idgs::admin::pb::Success,
          memInfoJson);
  idgs::actor::sendMessage(resposne);

  return true;
}

//////////////////////////////// ActorFramework nodes ////////////////////////////////

bool ActorframeworkNode::init() {
  attributesPath.push_back(ALL_STATEFUL_ACTORS);
  attributesPath.push_back(ALL_STATELESS_ACTORS);
  attributesPath.push_back(ACTOR);

  opMap[ALL_STATEFUL_ACTORS] = [this] (OperationContext& context) {
    processActorsReq(context);
  };

  opMap[ALL_STATELESS_ACTORS] = [this] (OperationContext& context) {
    processActorsReq(context);
  };

  opMap[ACTOR] = [this] (OperationContext& context) {
    processActorsReq(context);
  };

  return true;
}

bool ActorframeworkNode::processActorsReq(OperationContext& context) {
  idgs::admin::AttributePathPtr& attr = context.attr;
  DVLOG(3) << "will process actors admin request:" << attr->getFullPath();
  if (!checkOperationName(context, ADMIN_GET_REQUEST)) {
    return false;
  }

  string memInfoJson;
  idgs::Application& app = ::idgs::util::singleton<idgs::Application>::getInstance();

  std::string member_id;
  attr->getParameterValue(MEMBER_ID_PARAM, member_id);
  DVLOG(3) << "get member id: " << member_id;
  if (app.getMemberManager()->findMember(atoi(member_id.c_str())) == NULL) {
    std::shared_ptr<idgs::actor::ActorMessage> resposne =
        idgs::admin::util::createAdminResponse(context,
            ::idgs::admin::pb::Error,
            "can not find member " + member_id);

    idgs::actor::sendMessage(resposne);
    return false;
  }

  int targetMemberId = atoi(member_id.c_str());
  uint32_t localMemberId = app.getMemberManager()->getLocalMemberId();
  if (localMemberId != targetMemberId) {
    idgs::actor::ActorMessagePtr& actorMsg = context.actorMsg;
    idgs::actor::ActorMessagePtr routMsg = actorMsg->createRouteMessage(targetMemberId, ADMIN_ACTOR_ID);
    DVLOG(3) << "Actor Admin request [" << actorMsg->toString() << "] will be routed to member " << targetMemberId;
    idgs::actor::sendMessage(routMsg);
    return false;
  }

  std::string body="";
  DVLOG(3) << "process attribute: " << attr->getAttributePath();
  if (attr->getAttributePath() == ALL_STATEFUL_ACTORS) {
    idgs::actor::StatefulActorMap& actorsMap = app.getActorframework()->getStatefulActors();
    body = statefulActos2Json(actorsMap);
  } else if (attr->getAttributePath() == ALL_STATELESS_ACTORS) {

  } else if (attr->getAttributePath() == ACTOR) {

  } else {

  }

  DVLOG(3) << "The response body is : \n" << body;

  std::shared_ptr<idgs::actor::ActorMessage> resposne =
      idgs::admin::util::createAdminResponse(context,
          ::idgs::admin::pb::Success,
           body);

  idgs::actor::sendMessage(resposne);

  return true;
}

std::string ActorframeworkNode::statefulActos2Json(idgs::actor::StatefulActorMap& actorsMap) {
  std::string json="";
  if (actorsMap.empty()) {
    return json;
  }

  map<std::string, std::string> actorCount;
  for (auto itr = actorsMap.begin(); itr != actorsMap.end(); ++itr) {
    std::string& value = actorCount[idgs::util::demangle(typeid((*(itr->second))).name())];
    if (value.empty()) {
      value.append(itr->first);
    } else {
      value.append(","+itr->first);
    }
  }

  std::vector<string> pairs;
  for (auto itr = actorCount.begin(); itr != actorCount.end(); ++itr) {
    std::string json;
    idgs::admin::util::toJsonItem(itr->first, itr->second, json);
    pairs.push_back(json);
  }

  std::string* ss = idgs::admin::util::aggregateJsonItems(pairs);
  std::string content;
  content = *ss;
  delete ss;

  return content;
}

}
}
}



