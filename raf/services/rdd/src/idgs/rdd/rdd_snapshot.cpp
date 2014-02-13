/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/

#include "rdd_snapshot.h"
#include "idgs/cluster/cluster_framework.h"

#include "idgs/rdd/rdd_const.h"
#include "protobuf/message_helper.h"
#include "idgs/store/datastore_const.h"
#include "idgs/store/data_store.h"

using namespace idgs::actor;
using namespace idgs::util;
using namespace idgs::cluster;
using namespace idgs::rdd::pb;
using namespace google::protobuf;
using namespace protobuf;

/// @todo remove dependencies on store.
using namespace idgs::store::pb;
using namespace idgs::store;

#define MAX_TRY_FIND_RDD_TIMES 10


namespace idgs {
namespace rdd {

void RddSnapshot::addDependingRdd(const idgs::pb::ActorId& rddActorId, const std::string& rddName) {
  idgs::rdd::RddMetaInfo dependingRdd;
  dependingRdd.setActorId(rddActorId);
  if (rddName == "") {
    dependingRdd.setRddName(rddActorId.actor_id());
  } else {
    dependingRdd.setRddName(rddName);
  }

  dependingRdds.push_back(dependingRdd);
}

std::vector<idgs::rdd::RddMetaInfo>& RddSnapshot::getDependingRdd() {
  return dependingRdds;
}

const idgs::rdd::RddMetaInfo& RddSnapshot::getSelfInfo() const {
  return selfInfo;
}

void RddSnapshot::setSelfInfo(const idgs::rdd::RddMetaInfo& selfInfo) {
  this->selfInfo = selfInfo;
}

void RddSnapshot::setSelfInfo(const idgs::pb::ActorId& rddActorId, const std::string& rddName) {
  selfInfo.setActorId(rddActorId);
  if (rddName == "") {
    selfInfo.setRddName(rddActorId.actor_id());
  } else {
    selfInfo.setRddName(rddName);
  }
}

void RddSnapshot::setSelfInfo(const int32_t& memberId, const std::string& actorId, const std::string& rddName) {
  selfInfo.setActorId(memberId, actorId);
  if (rddName == "") {
    selfInfo.setRddName(actorId);
  } else {
    selfInfo.setRddName(rddName);
  }
}

pb::RddResultCode RddSnapshot::save() {

  const string& rddName = selfInfo.getRddName();
  VLOG(2) << "begin to save [RDD name:  " << rddName << " || actor id:" << selfInfo.getActorId().actor_id()
      << " || member id: " << selfInfo.getActorId().member_id() << "]";
  uint32_t localMemberId = singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();

  auto key = singleton<MessageHelper>::getInstance().createMessage(RDD_NAME_KEY);
  if (!key) {
    LOG(ERROR)<< "system store RddName key is not registered";
    return pb::RRC_INVALID_KEY;
  }

  auto value = singleton<MessageHelper>::getInstance().createMessage(RDD_NAME);
  if (!value) {
    LOG(ERROR)<< "system store RddName value is not registered";
    return pb::RRC_INVALID_VALUE;
  }

  key->GetReflection()->SetString(key.get(), key->GetDescriptor()->FindFieldByName("rdd_name"), rddName);
  value->GetReflection()->SetInt32(value.get(), value->GetDescriptor()->FindFieldByName("member_id"), selfInfo.getActorId().member_id());
  value->GetReflection()->SetString(value.get(), value->GetDescriptor()->FindFieldByName("actor_id"), selfInfo.getActorId().actor_id());



  vector<RddMetaInfo>& dependingRddInfos = getDependingRdd();
  for (int i=0;i<dependingRddInfos.size();i++) {
    RddMetaInfo tmp = dependingRddInfos.at(i);
    auto dRddInfo =
        value->GetReflection()->AddMessage(value.get(), value->GetDescriptor()->FindFieldByName("depending_rdd_info"));
    if (!dRddInfo) {
      LOG(ERROR)<< "Can not create depending_rdd_info";
      return pb::RRC_INVALID_VALUE;
    }

    RddResultCode code;
    restore(tmp.getRddName(), code);
    if (code != RRC_SUCCESS) {
      LOG(ERROR)<<"can not get depending RDD [RDD name:  " << tmp.getRddName() << " || actor id:" << tmp.getActorId().actor_id()
        << " || member id: " << tmp.getActorId().member_id() << "] for RDD " << rddName ;
      return pb::RRC_INVALID_VALUE;
    }

    VLOG(2)<<"save depending RDD [RDD name:  " << tmp.getRddName() << " || actor id:" << tmp.getActorId().actor_id()
      << " || member id: " << tmp.getActorId().member_id() << "] for RDD " << rddName ;
    dRddInfo->GetReflection()->SetString(dRddInfo, dRddInfo->GetDescriptor()->FindFieldByName("rdd_name"),
        tmp.getRddName());
    dRddInfo->GetReflection()->SetInt32(dRddInfo, dRddInfo->GetDescriptor()->FindFieldByName("member_id"),
        tmp.getActorId().member_id());
    dRddInfo->GetReflection()->SetString(dRddInfo, dRddInfo->GetDescriptor()->FindFieldByName("actor_id"),
        tmp.getActorId().actor_id());
  }

  shared_ptr<InsertRequest> request(new InsertRequest);
  request->set_store_name("RddName");

  ActorMessagePtr msg = idgs::actor::Actor::createActorMessage(selfInfo.getActorId().actor_id());
  msg->setOperationName(OP_INSERT);
  msg->setDestMemberId(localMemberId);
  msg->setDestActorId(ACTORID_STORE_SERVCIE);
  msg->setPayload(request);
  msg->setAttachment(STORE_ATTACH_KEY, key);
  msg->setAttachment(STORE_ATTACH_VALUE, value);

  idgs::actor::sendMessage(msg);

  return pb::RRC_SUCCESS;
}

shared_ptr<RddSnapshot> RddSnapshot::restore(const string& rddName, RddResultCode& code) {
  shared_ptr<RddSnapshot> snapshot(NULL);

  auto key = singleton<MessageHelper>::getInstance().createMessage(RDD_NAME_KEY);
  if (!key) {
    LOG(ERROR)<< "load RDD actor error, rdd name: " << rddName << ", caused by Store: " << RDD_NAME_KEY << " not registered";
    code = pb::RRC_INVALID_VALUE;
    return snapshot;
  }

  auto value = singleton<MessageHelper>::getInstance().createMessage(RDD_NAME);
  if (!value) {
    LOG(ERROR)<< "load RDD actor error, rdd name: " << rddName << ", caused by Store:" << RDD_NAME << " not registered";
    code = pb::RRC_INVALID_VALUE;
    return snapshot;
  }

  key->GetReflection()->SetString(key.get(), key->GetDescriptor()->FindFieldByName("rdd_name"), rddName);

  StoreKey<google::protobuf::Message> storeKey(key);
  StoreValue<google::protobuf::Message> storeValue(value);
  ResultCode rc;
  int32_t try_find_rdd_times = 0;
  do {
    rc = singleton<DataStore>::getInstance().getData("RddName", storeKey, storeValue);
    if (try_find_rdd_times > 0) {
      sleep(1);
    }
  } while (rc != RC_SUCCESS && (++try_find_rdd_times) < MAX_TRY_FIND_RDD_TIMES);

  if (rc != RC_SUCCESS) {
    LOG(ERROR)<< "load RDD [rdd name: " << rddName << "] error, caused by " << getErrorDescription(rc);
    code = pb::RRC_RDD_NOT_FOUND;
    return snapshot;
  }



  auto ref = storeValue.get()->GetReflection();
  auto des = storeValue.get()->GetDescriptor();
  int32_t memberId = ref->GetInt32(*storeValue.get(), des->FindFieldByName("member_id"));
  string actorId = ref->GetString(*storeValue.get(), des->FindFieldByName("actor_id"));

  VLOG(2) << "Restore RDD [RDD name:  " << rddName << " || actor id:" << actorId << " || member id: " << memberId << "]";

  if (memberId < 0 || actorId == "Unknown Actor") {
    LOG(ERROR) << "load RDD actor " << rddName << " error, caused by error member id or actor id" << memberId << "," << actorId;
    code = pb::RRC_RDD_NOT_FOUND;
    return snapshot;
  }

  snapshot.reset(new RddSnapshot);
  snapshot->setSelfInfo(memberId, actorId, rddName);

  vector<RddMetaInfo>& dependingRdd = snapshot->getDependingRdd();

  const FieldDescriptor* dsrpDepRddInfo = des->FindFieldByName("depending_rdd_info");
  int size = ref->FieldSize(*storeValue.get(), dsrpDepRddInfo);
  VLOG(4) << "get depending_rdd_info size " << size;
  for (int i = 0;i<size; i++) {
    RddMetaInfo dRdd;
    const Message& m = ref->GetRepeatedMessage(*storeValue.get(), dsrpDepRddInfo, i);
    VLOG(4) << "get rdd info : " << m.DebugString();
    auto tmpRef = m.GetReflection();
    auto tmpDes = m.GetDescriptor();
    string cRddName = tmpRef->GetString(m, tmpDes->FindFieldByName("rdd_name"));
    int32_t memberId = tmpRef->GetInt32(m, tmpDes->FindFieldByName("member_id"));
    string actorId = tmpRef->GetString(m, tmpDes->FindFieldByName("actor_id"));
    dRdd.setActorId(memberId, actorId);
    dRdd.setRddName(cRddName);
    VLOG(2)<<"restore depending RDD [RDD name:  " << cRddName << " || actor id:" << actorId
      << " || member id: " << memberId << "] for RDD " << rddName ;
    dependingRdd.push_back(dRdd);
  }

  code = pb::RRC_SUCCESS;
  return snapshot;
}


} // namespace rdd 
} // namespace idgs 
