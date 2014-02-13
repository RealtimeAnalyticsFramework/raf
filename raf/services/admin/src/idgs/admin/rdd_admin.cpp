
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "rdd_admin.h"
#include "admin_util.h"
#include "idgs/admin/actor/admin_service_actor.h"
#include "idgs/application.h"


using namespace std;
using namespace idgs::actor;
using namespace idgs::admin::actor;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::admin::util;
using namespace idgs::pb;
using namespace idgs::cluster;
using namespace idgs::util;

namespace idgs {
namespace admin {
namespace rdd {
bool RddAdmin::init() {
  dNode->init();
  manageableNodes.push_back(dNode);
  return true;
}

bool RddDependencyNode::init() {
  attributesPath.push_back(RDD_DEPENDENCY);
  attributesPath.push_back(RDD_INFO);

  opMap[RDD_DEPENDENCY] = [this] (OperationContext& context) {
    processRddDependencyReq(context);
  };

  opMap[RDD_INFO] = [this] (OperationContext& context) {
    processRddInfoReq(context);
  };

  return true;
}


bool RddDependencyNode::processRddDependencyReq(OperationContext& context) {
  //RddActor* rdd = NULL;

  VLOG(2) << "**** processRddDependencyReq for request >>>> " << (context.actorMsg)->toString();
  string rddName;

  if (!checkOperationName(context, ADMIN_GET_REQUEST)) {
    return NULL;
  }

  if(!checkParameterValue(context, RDD_NAME_PARAM, rddName)) {
    return NULL;
  }

  RddResultCode code;
  shared_ptr<RddSnapshot> snapshot = RddSnapshot::restore(rddName, code);
  if (snapshot.get() == NULL) {
    LOG(ERROR) << "can not find snapshot for rdd : " << rddName;
    idgs::admin::util::sendAdminErrorResponse(
       context,
       "can not find Rdd for " + rddName);
    return false;
  }

  string tmp;
  rddSnapshot2Json(*snapshot, tmp);
  VLOG(2) << "get parent snapshot >>> " << tmp;
  tmp.clear();

  if (code != RRC_SUCCESS) {
    LOG(ERROR) << "can not find snapshot for rdd : " << rddName;
    idgs::admin::util::sendAdminErrorResponse(
       context,
       "can not find snapshot for RDD : " + rddName);
    return false;
  }

  map<string, shared_ptr<RddSnapshot> > childRdds;

  if (!getDependency(*(snapshot.get()), childRdds)) {
    idgs::admin::util::sendAdminErrorResponse(
      context,
      "can not get dependency for RDD : " + rddName);
    return false;
  }

  string dependency;
  dependency2Json(*snapshot, childRdds, dependency);

  ActorMessagePtr response = idgs::admin::util::createAdminResponse(
      context,
      ::idgs::admin::pb::Success,
      dependency);
  idgs::actor::sendMessage(response);

  return true;
}

bool RddDependencyNode::processRddInfoReq(OperationContext& context) {
  RddActor* rdd = NULL;
  string rddName;

  rdd = checkMsgAndLoadRddActor(context, rddName);
  if(!rdd) {
/*    LOG(ERROR) << "can not find RDD actor " << rddName;
    idgs::admin::util::sendAdminErrorResponse(
      context,
      "can not find RDD actor " + rddName);*/
    return false;
  }

  string dependingRddInfoJosn;
  string jsonBody;
  if (!dependingRddInfo2Json(rdd,dependingRddInfoJosn)) {
    LOG(ERROR) << "can not get RDD info for RDD : " << rddName;
    idgs::admin::util::sendAdminErrorResponse(
      context,
      "can not get RDD info for RDD : " + rddName);
    return false;
  }

  string _rddName;
  ::idgs::admin::util::toJsonItem("Rdd Name", rddName, _rddName);

  string _actorId;
  ::idgs::admin::util::toJsonItem("Actor Id", rdd->getActorId(), _actorId);

  string _localMemberId;
  ::idgs::admin::util::toJsonItem("Local Member Id", rdd->getLocalMemberId(), _localMemberId);

  string _pendingMessageNumber;
  ::idgs::admin::util::toJsonItem("Pending Message Number", rdd->getPendingMessageNumber(), _pendingMessageNumber);

  string _rddState;
  string& _tmpRddState = RDD_STATES_ARRAY[rdd->getRddState()];
  ::idgs::admin::util::toJsonItem("Rdd State", _tmpRddState, _rddState);

  string _actorState;
  string& _tmpActorState = ACTOR_STATES_DSP[rdd->getStatus()];
  ::idgs::admin::util::toJsonItem("Actor State", _tmpActorState, _actorState);

  string* tmp =
      AGGREGATE_JSON_ITEMS(_rddName, _actorId, _localMemberId, _pendingMessageNumber,
          _rddState, _actorState, dependingRddInfoJosn);
  jsonBody.append(*tmp);
  delete tmp;

  ActorMessagePtr response = idgs::admin::util::createAdminResponse(
      context,
      ::idgs::admin::pb::Success,
      jsonBody);
  idgs::actor::sendMessage(response);

  return true;
}

idgs::rdd::RddActor* RddDependencyNode::checkMsgAndLoadRddActor(OperationContext& context,
    std::string& rddName) {

  if (!checkOperationName(context, ADMIN_GET_REQUEST)) {
    return NULL;
  }

  if(!checkParameterValue(context, RDD_NAME_PARAM, rddName)) {
    return NULL;
  }


  RddActor* rdd = getRddActor(rddName, context);
  if(!rdd) {
    return NULL;
  }

  return rdd;
}

idgs::rdd::RddActor* RddDependencyNode::getRddActor(
    const std::string& rddName,
    OperationContext& context) {
  idgs::Application& app = ::idgs::util::singleton<idgs::Application>::getInstance();
  ActorFramework* af = app.getActorframework();

  RddResultCode code;
  shared_ptr<RddSnapshot> snapshot = RddSnapshot::restore(rddName, code);
  if (code != RRC_SUCCESS) {
    LOG(ERROR)<< "Error when loading RDD " << rddName << ", caused by " << RddResultCode_Name(code);
    idgs::admin::util::sendAdminErrorResponse(context, "can not find rdd actor for name : " + rddName);
    return NULL;
  }

  const ActorId& actorId = snapshot->getSelfInfo().getActorId();
  uint32_t localMemberId = singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();

  idgs::actor::ActorMessagePtr& actorMsg = context.actorMsg;
  if (localMemberId != actorId.member_id()) {
    ActorMessagePtr routMsg = actorMsg->createRouteMessage(actorId.member_id(), ADMIN_ACTOR_ID);
    VLOG(3)<< "get rdd request [" << actorMsg->toString() << "] will be routed to member " <<  actorId.member_id();
    idgs::actor::sendMessage(routMsg);
    return NULL;
  }

  Actor* actor = af->getActor(actorId.actor_id());
  if (actor == NULL) {
    LOG(WARNING) << " can not get RDD actor for [member id:"<<actorId.member_id() << ", actor id:" <<actorId.actor_id() <<"]";
    idgs::admin::util::sendAdminErrorResponse(context, "can not find rdd actor for name : " + rddName);
    return NULL;
  }

  return dynamic_cast<RddActor*>(actor);
}

bool RddDependencyNode::getDependency(RddSnapshot& rdd, map<string, shared_ptr<RddSnapshot>>& childRdds) {
  if (addChildRdd(rdd, childRdds) == RC_ERROR) {
    return false;
  }

  map<string, shared_ptr<RddSnapshot>>::iterator itr = childRdds.begin();
  map<string, shared_ptr<RddSnapshot>>::iterator end = childRdds.end();
  while(itr != end) {
    if (addChildRdd(*(itr->second), childRdds) == RC_SUCCESS) {
      end = childRdds.end();
      ++itr;
    } else {
      return false;
    }
  }

  return true;
}

idgs::ResultCode RddDependencyNode::addChildRdd(RddSnapshot& rdd, map<string, shared_ptr<RddSnapshot>>& childRdds) {
  std::vector<RddMetaInfo>& childs = rdd.getDependingRdd();
  if (childs.empty()) {
    LOG(WARNING) << "rdd " << rdd.getSelfInfo().getRddName() << " doesn't have child";
    return RC_SUCCESS;
  }

  std::vector<RddMetaInfo>::iterator itr = childs.begin();
  for (;itr < childs.end(); ++itr) {
    RddMetaInfo& dependingRdd = *itr;
    if (childRdds.find(dependingRdd.getRddName()) == childRdds.end()) {
      RddResultCode code;
      shared_ptr<RddSnapshot> snap = RddSnapshot::restore(dependingRdd.getRddName(), code);
      if (code != RRC_SUCCESS) {
        LOG(ERROR) << "can not get depending RDD " << dependingRdd.getRddName() << " for RDD " << rdd.getSelfInfo().getRddName();
        return RC_ERROR;
      }
      childRdds.insert(make_pair(dependingRdd.getRddName(), snap));
    }
  }

  return RC_SUCCESS;
}

bool RddDependencyNode::dependency2Json(RddSnapshot& parent, map<string, shared_ptr<RddSnapshot>>& childRdds, std::string& json) {

  assert(json.size() == 0);

  json.append("{");
  string parentJosn;
  if(!rddSnapshot2Json(parent, parentJosn)) {
    return false;
  }
  json.append(parentJosn);

  json.append(",");

  map<string, shared_ptr<RddSnapshot>>::iterator itr = childRdds.begin();
  for (; itr != childRdds.end(); ++itr) {
    string tmp;
    if(rddSnapshot2Json(*(itr->second), tmp)) {
      json.append(tmp);
      json.append(",");
    }
  }
  json.pop_back();
  json.append("}");
  return true;
}

bool RddDependencyNode::rddSnapshot2Json(RddSnapshot& snapshot, string& json) {
  assert(json.size() == 0);

  vector<RddMetaInfo>& dpdRdds = snapshot.getDependingRdd();
  vector<RddMetaInfo>::iterator itr = dpdRdds.begin();

  json.append("\"");
  json.append(snapshot.getSelfInfo().getRddName());
  json.append("\"");
  json.append(":");
  json.append("[");

  for(;itr < dpdRdds.end();++itr) {
    RddMetaInfo& rddMeta = *itr;

     string rddName;
     toJsonItem("rdd_name",rddMeta.getRddName(), rddName);

     string actorId;
     toJsonItem("actor_id",rddMeta.getActorId().actor_id(), actorId);

     string memberId;
     toJsonItem("member_id",rddMeta.getActorId().member_id(), memberId);

     string* tmp =
         AGGREGATE_JSON_ITEMS(rddName,actorId,memberId);
     json.append(*tmp);
     json.append(",");
     delete tmp;
   }

   if (!dpdRdds.empty()) {
     json.pop_back();
   }

   json.append("]");

   return true;

}

bool RddDependencyNode::dependingRddInfo2Json(RddActor* actor, string& json) {

  assert(actor != NULL);
  assert(json.size() == 0);

  vector<RddInfo>& rddsInfo = actor->getDependingRdd();
  vector<RddInfo>::iterator itr;

  json.append("\"");
  json.append("Depending RDD");
  json.append("\"");
  json.append(":");
  json.append("[");

  for(itr = rddsInfo.begin(); itr < rddsInfo.end(); ++itr) {
    RddInfo& rddInfo = *itr;

    string rddName;
    toJsonItem("rdd_name",rddInfo.getRddName(), rddName);

    string rddState;;
    string& _tmpRddState = RDD_STATES_ARRAY.at(rddInfo.getState());
    toJsonItem("rdd_state",_tmpRddState, rddState);

    string actorId;
    toJsonItem("actor_id",rddInfo.getActorId().actor_id(), actorId);

    string memberId;
    toJsonItem("member_id",rddInfo.getActorId().member_id(), memberId);

    string* tmp =
        AGGREGATE_JSON_ITEMS(rddName,rddState,actorId,memberId);
    json.append(*tmp);
    json.append(",");
    delete tmp;
  }
  if (!rddsInfo.empty()) {
    json.pop_back();
  }

  json.append("]");

  return true;
}

bool RddDependencyNode::rddInfo2Json(idgs::rdd::RddActor* actor, std::string& json) {
  assert(actor != NULL);
  assert(json.size() == 0);


}

}
}
}

