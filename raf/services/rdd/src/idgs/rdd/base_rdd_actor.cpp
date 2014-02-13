
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "base_rdd_actor.h"

#include "idgs/cluster/cluster_framework.h"

#include "idgs/store/datastore_const.h"
#include "idgs/store/data_map.h"
#include "idgs/rdd/rdd_module.h"

using namespace idgs::pb;
using namespace idgs::actor;
using namespace idgs::util;
using namespace idgs::cluster;
using namespace idgs::rdd::pb;
using namespace idgs::rdd::action;
using namespace google::protobuf;

namespace idgs {
namespace rdd {

size_t BaseRddActor::partitionSize;
int32_t BaseRddActor::localMemberId;

BaseRddActor::BaseRddActor() : metadata(new idgs::store::pb::MetadataPair),
    dependingRddResponse(0), partitionResponse(0) {
  ClusterFramework& cluster = singleton<ClusterFramework>::getInstance();
  BaseRddActor::partitionSize = cluster.getPartitionCount();
  BaseRddActor::localMemberId = cluster.getMemberManager()->getLocalMemberId();

  rddInfo.setActorId(localMemberId, getActorId());
  rddInfo.setState(pb::INIT);
}

BaseRddActor::~BaseRddActor() {
  dependedRdds.clear();
  dependingRdds.clear();
  rddAction.clear();
}

idgs::actor::ActorDescriptorPtr BaseRddActor::generateBaseActorDescriptor(const std::string& actorName) {
  ActorDescriptorPtr descriptor(new ActorDescriptorWrapper);
  descriptor->setName(actorName);
  descriptor->setDescription("RDD");
  descriptor->setType(AT_STATEFUL);

  // in operation
  ActorOperationDescriporWrapper inCreateRddPartitionResponse;
  inCreateRddPartitionResponse.setName(CREATE_RDD_PARTITION_RESPONSE);
  inCreateRddPartitionResponse.setDescription("The response of creating partition of RDD.");
  inCreateRddPartitionResponse.setPayloadType("idgs.rdd.pb.CreateRddPartitionResponse");
  descriptor->setInOperation(inCreateRddPartitionResponse.getName(), inCreateRddPartitionResponse);

  ActorOperationDescriporWrapper inDependingPartitionResposne;
  inDependingPartitionResposne.setName(PARTITION_PREPARED_RESPONSE);
  inDependingPartitionResposne.setDescription("The response of depending partition request.");
  inDependingPartitionResposne.setPayloadType("idgs.rdd.pb.RddResponse");
  descriptor->setInOperation(inDependingPartitionResposne.getName(), inDependingPartitionResposne);

  ActorOperationDescriporWrapper inGetPartitionActor;
  inGetPartitionActor.setName(GET_PARTITION_ACTOR);
  inGetPartitionActor.setDescription("To get the partition member id and actor id of RDD.");
  inGetPartitionActor.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setInOperation(inGetPartitionActor.getName(), inGetPartitionActor);

  ActorOperationDescriporWrapper inGetPartitionActorResponse;
  inGetPartitionActorResponse.setName(GET_PARTITION_ACTOR_RESPONSE);
  inGetPartitionActorResponse.setDescription("Receive partition actor info");
  inGetPartitionActorResponse.setPayloadType("idgs.rdd.pb.RddActorInfo");
  descriptor->setInOperation(inGetPartitionActorResponse.getName(), inGetPartitionActorResponse);

  ActorOperationDescriporWrapper inReceiveDependingRdd;
  inReceiveDependingRdd.setName(RECEIVE_DEPENDING_RDD);
  inReceiveDependingRdd.setDescription("Receive RDD id from depending RDDs.");
  inReceiveDependingRdd.setPayloadType("idgs.pb.ActorId");
  descriptor->setInOperation(inReceiveDependingRdd.getName(), inReceiveDependingRdd);

  ActorOperationDescriporWrapper inRddTransform;
  inRddTransform.setName(RDD_TRANSFORM);
  inRddTransform.setDescription("Taking transform of RDD.");
  inRddTransform.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setInOperation(inRddTransform.getName(), inRddTransform);

  ActorOperationDescriporWrapper inRddReady;
  inRddReady.setName(RDD_READY);
  inRddReady.setDescription("Collect each depending rdds when they are ready.");
  inRddReady.setPayloadType("idgs.rdd.pb.RddStateTracing");
  descriptor->setInOperation(inRddReady.getName(), inRddReady);

  ActorOperationDescriporWrapper inRddPartitionReady;
  inRddPartitionReady.setName(RDD_PARTITION_READY);
  inRddPartitionReady.setDescription("Collect each partition when partition process done.");
  inRddPartitionReady.setPayloadType("idgs.rdd.pb.PartitionProcessResult");
  descriptor->setInOperation(inRddPartitionReady.getName(), inRddPartitionReady);

  ActorOperationDescriporWrapper inActionMessageProcess;
  inActionMessageProcess.setName(ACTION_MESSAGE_PROCESS);
  inActionMessageProcess.setDescription("Process action message.");
  inActionMessageProcess.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setInOperation(inActionMessageProcess.getName(), inActionMessageProcess);

  ActorOperationDescriporWrapper inRddActionRequest;
  inRddActionRequest.setName(RDD_ACTION_REQUEST);
  inRddActionRequest.setDescription("The action of count data size.");
  inRddActionRequest.setPayloadType("idgs.rdd.pb.ActionRequest");
  descriptor->setInOperation(inRddActionRequest.getName(), inRddActionRequest);

  ActorOperationDescriporWrapper inActionResponse;
  inActionResponse.setName(RDD_ACTION_RESPONSE);
  inActionResponse.setDescription("Collect each partition of action when partition process done.");
  inActionResponse.setPayloadType("idgs.rdd.pb.ActionResponse");
  descriptor->setInOperation(inActionResponse.getName(), inActionResponse);

  ActorOperationDescriporWrapper inSendRddInfoResponse;
  inSendRddInfoResponse.setName(SEND_RDD_INFO_RESPONSE);
  inSendRddInfoResponse.setDescription("The response of receive information of rdd and all it's partition.");
  inSendRddInfoResponse.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setInOperation(inSendRddInfoResponse.getName(), inSendRddInfoResponse);

  ActorOperationDescriporWrapper inTransformComplete;
  inTransformComplete.setName(PARTITION_TRANSFORM_COMPLETE);
  inTransformComplete.setDescription("When transform complete.");
  inTransformComplete.setPayloadType("idgs.rdd.pb.PartitionProcessResult");
  descriptor->setInOperation(inTransformComplete.getName(), inTransformComplete);

  ActorOperationDescriporWrapper inRddStateRequest;
  inRddStateRequest.setName(RDD_STATE_REQUEST);
  inRddStateRequest.setDescription("Get current RDD state.");
  inRddStateRequest.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setInOperation(inRddStateRequest.getName(), inRddStateRequest);

  ActorOperationDescriporWrapper inRddStateResponse;
  inRddStateResponse.setName(RDD_STATE_RESPONSE);
  inRddStateResponse.setDescription("Get current RDD state.");
  inRddStateResponse.setPayloadType("idgs.rdd.pb.RddStateTracing");
  descriptor->setInOperation(inRddStateResponse.getName(), inRddStateResponse);

  ActorOperationDescriporWrapper inInsertRddNameResponse;
  inInsertRddNameResponse.setName(idgs::store::DATA_STORE_INSERT_RESPONSE);
  inInsertRddNameResponse.setDescription("The response of insert rdd name.");
  inInsertRddNameResponse.setPayloadType("idgs.store.pb.InsertResponse");
  descriptor->setInOperation(inInsertRddNameResponse.getName(), inInsertRddNameResponse);

  // out operation
  // out descriptor for message CREATE_RDD_PARTITION_RESPONSE
  ActorOperationDescriporWrapper outSendRddInfo;
  outSendRddInfo.setName(SEND_RDD_INFO);
  outSendRddInfo.setDescription("out message for CREATE_RDD_PARTITION_RESPONSE");
  outSendRddInfo.setPayloadType("idgs.rdd.pb.RddActorInfo");
  descriptor->setOutOperation(outSendRddInfo.getName(), outSendRddInfo);

  // out descriptor for message GET_PARTITION_ACTOR
  ActorOperationDescriporWrapper outGetPartitionActorResponse;
  outGetPartitionActorResponse.setName(GET_PARTITION_ACTOR_RESPONSE);
  outGetPartitionActorResponse.setDescription("out message for GET_PARTITION_ACTOR");
  outGetPartitionActorResponse.setPayloadType("idgs.rdd.pb.RddActorInfo");
  descriptor->setOutOperation(outGetPartitionActorResponse.getName(), outGetPartitionActorResponse);

  // out descriptor for message GET_PARTITION_ACTOR_RESPONSE
  ActorOperationDescriporWrapper outCreateRddPartition;
  outCreateRddPartition.setName(CREATE_RDD_PARTITION);
  outCreateRddPartition.setDescription("out message for GET_PARTITION_ACTOR_RESPONSE");
  outCreateRddPartition.setPayloadType("idgs.rdd.pb.CreateRddPartitionRequest");
  descriptor->setOutOperation(outCreateRddPartition.getName(), outCreateRddPartition);

  // no out descriptor for message RECEIVE_DEPENDING_RDD

  // out descriptor for message RDD_TRANSFORM, RDD_ACTION_REQUEST
  descriptor->setOutOperation(inRddTransform.getName(), inRddTransform);

  // out descriptor for message RDD_TRANSFORM, RDD_READY to see sub class

  // out descriptor for message RDD_PARTITION_READY, RDD_ACTION_REQUEST
  ActorOperationDescriporWrapper outActionMessageProcess;
  outActionMessageProcess.setName(ACTION_MESSAGE_PROCESS);
  outActionMessageProcess.setDescription("out message for RDD_PARTITION_READY");
  outActionMessageProcess.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setOutOperation(outActionMessageProcess.getName(), outActionMessageProcess);

  // out descriptor for message ACTION_MESSAGE_PROCESS
  descriptor->setOutOperation(inRddActionRequest.getName(), inRddActionRequest);

  // out descriptor for message RDD_ACTION_REQUEST, RDD_ACTION_RESPONSE
  descriptor->setOutOperation(inActionResponse.getName(), inActionResponse);

  // no out descriptor for message SEND_RDD_INFO_RESPONSE

  // out descriptor for message PARTITION_TRANSFORM_COMPLETE
  ActorOperationDescriporWrapper outCheckPartitionReady;
  outCheckPartitionReady.setName(CHECK_PARTITION_READY);
  outCheckPartitionReady.setDescription("out message for PARTITION_TRANSFORM_COMPLETE");
  outCheckPartitionReady.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setOutOperation(outCheckPartitionReady.getName(), outCheckPartitionReady);

  descriptor->setOutOperation(inRddStateRequest.getName(), inRddStateRequest);
  descriptor->setOutOperation(inRddStateResponse.getName(), inRddStateResponse);

  return descriptor;
}

void BaseRddActor::addDependingRdd(const ActorId& rddActorId, const string& rddName) {
  RddInfo rdd;
  rdd.setActorId(rddActorId.member_id(), rddActorId.actor_id());
  rdd.setState(pb::INIT);
  if (rddName == "") {
    rdd.setRddName(rddActorId.actor_id());
  } else {
    rdd.setRddName(rddName);
  }

  dependingRdds.push_back(rdd);
}

std::vector<RddInfo>&  BaseRddActor::getDependingRdd() {
  return dependingRdds;
}

void BaseRddActor::addDependedRdd(const ActorId& rddActorId) {
  dependedRdds.push_back(rddActorId);
}

std::vector<idgs::pb::ActorId>& BaseRddActor::getDependedRdd() {
  return dependedRdds;
}

const RddState& BaseRddActor::getRddState() const {
  return rddInfo.getState();
}

const RddActionPtr& BaseRddActor::getRddAction(const string& actionId) const {
  auto it = rddAction.find(actionId);
  return it->second;
}

void BaseRddActor::removeAction(const std::string& actionId) {
  rddAction.erase(actionId);
}

void BaseRddActor::setActionPartitionState(const string& actionId, const uint32_t& partition, RddState state) {
  auto it = rddAction.find(actionId);
  if (it != rddAction.end()) {
    it->second->setPartitionState(partition, state);
  }
}

void BaseRddActor::handleRddCreatePartitionResponse(const ActorMessagePtr& msg) {
  DVLOG(2) << "BaseRddActor : handle create store delegate response.";
  CreateRddPartitionResponse* response = dynamic_cast<CreateRddPartitionResponse*>(msg->getPayload().get());
  uint32_t partition = response->rdd_partition().partition();

  if (response->result_code() != RRC_SUCCESS) {
    LOG(ERROR)<< "partition " << response->rdd_partition().partition() << " is not ready, caused by " << RddResultCode_Name(response->result_code());
    rddInfo.setState(pb::ERROR);
    rddInfo.setPartitionState(partition, P_ERROR);
    return;
  } else {
    rddInfo.addPartitionInfo(partition, response->rdd_partition().actor_id(), P_CREATE);
  }

  auto partitionActors = rddInfo.getPartitionInfo();
  for (int32_t i = 0; i < partitionActors.size(); ++i) {
    if (partitionActors[i].getState() == P_INIT) {
      return;
    }
  }

  if (getActorName() == STORE_DELEGATE_RDD_ACTOR) {
    VLOG(1) << "Store delegate RDD \"" << getRddName() << "\" is ready.";

    rddInfo.setState(READY);
    processRddReady();

    return;
  }

  shared_ptr<RddActorInfo> rddActorInfo(new RddActorInfo);
  rddActorInfo->mutable_rdd_id()->set_actor_id(rddInfo.getActorId().actor_id());
  rddActorInfo->mutable_rdd_id()->set_member_id(rddInfo.getActorId().member_id());
  rddActorInfo->set_state(rddInfo.getState());
  for (int32_t i = 0; i < partitionActors.size(); ++i) {
    auto partActor = rddActorInfo->add_rdd_partition();
    partActor->set_partition(i);
    partActor->mutable_actor_id()->set_actor_id(partitionActors[i].getActorId().actor_id());
    partActor->mutable_actor_id()->set_member_id(partitionActors[i].getActorId().member_id());
  }

  for (int32_t i = 0; i < partitionActors.size(); ++i) {
    ActorMessagePtr msg = createActorMessage();
    msg->setOperationName(SEND_RDD_INFO);
    msg->setDestActorId(partitionActors[i].getActorId().actor_id());
    msg->setDestMemberId(partitionActors[i].getActorId().member_id());
    msg->setPayload(rddActorInfo);

    idgs::actor::postMessage(msg);
  }
}

void BaseRddActor::handlePartitionPreparedResposne(const idgs::actor::ActorMessagePtr& msg) {
  RddResponse* response = dynamic_cast<RddResponse*>(msg->getPayload().get());
  if (response->result_code() != RRC_SUCCESS) {
    LOG(ERROR)<< "Error in create RDD caused by " << RddResultCode_Name(response->result_code());
    rddInfo.setState(pb::ERROR);
    rddInfo.setPartitionState(response->partition(), P_ERROR);
    return;
  }

  DVLOG(2) << "RDD \"" << getRddName() << "\" receive response from partition " << response->partition();

  rddInfo.setPartitionState(response->partition(), P_PREPARED);

  for (size_t partition = 0; partition < partitionSize; ++partition) {
    if (rddInfo.getPartitionState(partition) != P_PREPARED) {
      return;
    }
  }

  DVLOG(1) << "RDD \"" << getRddName() << "\" all partition receive, RDD pareared.";

  if (rddInfo.getState() == pb::INIT) {
    VLOG(1) << "RDD \"" << getRddName() << "\" is prepared.";
    rddInfo.setState(pb::PREPARED);
    processRddPrepared();
  }
}

void BaseRddActor::handleRddProcess(const ActorMessagePtr& msg) {
  DVLOG(2) << "RDD \"" << getRddName() << "\" process.";
  rddInfo.setState(PROCESSING);
  auto partitionActors = rddInfo.getPartitionInfo();
  for (int32_t partition = 0; partition < partitionActors.size(); ++partition) {
    if (partitionActors[partition].getState() != P_READY) {
      const ActorId& actorid = partitionActors[partition].getActorId();

      ActorMessagePtr reqMsg = rawMsg->createRouteMessage(actorid.member_id(), actorid.actor_id());
      reqMsg->setOperationName(RDD_PARTITION_PROCESS);
      reqMsg->setSourceMemberId(localMemberId);
      reqMsg->setSourceActorId(getActorId());

      DVLOG(3) << "RDD \"" << getRddName() << "\" sending to partition " << partition << " with actor "
                  << actorid.actor_id() << " on " << actorid.member_id() << " to process";
      idgs::actor::postMessage(reqMsg);
    }
  }
}

void BaseRddActor::handleGetPartitionActor(const idgs::actor::ActorMessagePtr& msg) {
  shared_ptr<RddActorInfo> payload(new RddActorInfo);
  payload->set_state(getRddState());
  payload->mutable_rdd_id()->set_member_id(localMemberId);
  payload->mutable_rdd_id()->set_actor_id(getActorId());
  auto partitionActors = rddInfo.getPartitionInfo();
  for (int32_t partition = 0; partition < partitionActors.size(); ++partition) {
    const ActorId& actorid = partitionActors[partition].getActorId();
    auto rddPartition = payload->add_rdd_partition();
    rddPartition->set_partition(partition);
    rddPartition->mutable_actor_id()->set_actor_id(actorid.actor_id());
    rddPartition->mutable_actor_id()->set_member_id(actorid.member_id());
  }

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(GET_PARTITION_ACTOR_RESPONSE);
  respMsg->setPayload(payload);
  idgs::actor::sendMessage(respMsg);
}

void BaseRddActor::handleRddActionRequest(const idgs::actor::ActorMessagePtr& msg) {
  DVLOG(2) << "RDD \"" << getRddName() << "\" receive action request from actor " << msg->getSourceActorId() << " on "
              << msg->getSourceMemberId();
  ActionRequest* request = dynamic_cast<ActionRequest*>(msg->getPayload().get());

  DVLOG(3) << "action id : " << request->action_id() << ", operation : " << request->action_op_name()
              << ", current RDD state : " << rddInfo.getState();

  RddActionPtr action(new RddAction);
  action->setActionId(request->action_id());
  action->setActionOpName(request->action_op_name());
  action->setMessage(msg);

  rddAction[request->action_id()] = action;
  actionQueue.push(action);

  auto actorFramework = singleton<RpcFramework>::getInstance().getActorFramework();
  switch (rddInfo.getState()) {
    case INIT: {
      DVLOG(1) << "The state of RDD \"" << getRddName() << "\" is INIT";
      break;
    }
    case idgs::rdd::pb::PREPARED: {
      DVLOG(1) << "RDD \"" << getRddName() << "\" start transform.";
      shared_ptr<RddRequest> request(new RddRequest);
      auto rountMsg = msg->createRouteMessage(localMemberId, getActorId());
      rountMsg->setOperationName(RDD_TRANSFORM);
      rountMsg->setPayload(request);

      actorFramework->sendMessage(rountMsg);
      break;
    }
    case DEPENDENCY_PROCESSING: {
      DVLOG(1) << "The state of current RDD \"" << getRddName() << "\" is DEPENDENCY_PROCESSING";
      break;
    }
    case PROCESSING: {
      DVLOG(1) << "The state of current RDD \"" << getRddName() << "\" is PROCESSING";
      break;
    }
    case TRANSFORM_COMPLETE: {
      DVLOG(1) << "The state of current RDD \"" << getRddName() << "\" is TRANSFORM_COMPLETE";
      break;
    }
    case READY: {
      DVLOG(1) << "The state of current RDD \"" << getRddName() << "\" is READY";
      shared_ptr<RddRequest> request(new RddRequest);
      ActorMessagePtr message = createActorMessage();
      message->setDestActorId(getActorId());
      message->setDestMemberId(localMemberId);
      message->setOperationName(ACTION_MESSAGE_PROCESS);
      message->setPayload(request);
      actorFramework->sendMessage(message);
      break;
    }
    case ERROR: {
      LOG(ERROR)<< "Create current RDD \"" << getRddName() << "\" error, please check and rebuild RDD.";
      auto respMsg = msg->createResponse();
      respMsg->setOperationName(RDD_ACTION_RESPONSE);
      shared_ptr<ActionResponse> response(new ActionResponse);
      response->set_action_id(request->action_id());
      response->set_result_code(RRC_RDD_ERROR);
      respMsg->setPayload(response);
      actorFramework->sendMessage(respMsg);
      break;
    }
    default: {
      break;
    }
  }
}

void BaseRddActor::handleRddActionResponse(const ActorMessagePtr& msg) {
  ActionResponse* response = dynamic_cast<ActionResponse*>(msg->getPayload().get());
  string actionId = response->action_id();
  uint32_t partition = response->partition();
  setActionPartitionState(actionId, partition, READY);

  DVLOG(1) << "handle action response from partition " << partition;

  const RddActionPtr& action = getRddAction(actionId);
  if (response->result_code() != RRC_SUCCESS) {
    action->setActionResultCode(response->result_code());
    LOG(ERROR)<< "action error in partition " << partition << " cause by " << RddResultCode_Name(response->result_code());
  }

  for (int32_t i = 0; i < response->action_value_size(); ++i) {
    action->addActionResult(partition, response->action_value(i));
  }

  if (action->getState() != READY) {
    return;
  }

  DVLOG(1) << "all partition process action ready, aggregate result";
  ActorMessagePtr message = action->getMessage();
  auto respMsg = message->createResponse();
  respMsg->setOperationName(RDD_ACTION_RESPONSE);

  shared_ptr<ActionResponse> actionResponse(new ActionResponse);
  actionResponse->set_action_id(actionId);

  if (action->getActionResultCode() != RRC_SUCCESS) {
    actionResponse->set_result_code(action->getActionResultCode());
  } else {
    ActionMgr& actionMgr = idgs_rdd_module()->getActionManager();
    const ActionPtr& actionPtr = actionMgr.get(action->getActionOpName());

    if (action.get()) {
      RddResultCode code = actionPtr->aggregate(action->getMessage(), respMsg, action->getActionResult());
      actionResponse->set_result_code(code);
    } else {
      LOG(ERROR)<< "action " << action->getActionOpName() << " is not registered.";
      actionResponse->set_result_code(RRC_ACTION_NOT_FOUND);
    }
  }

  respMsg->setPayload(actionResponse);

  if (actionResponse->result_code() == RRC_SUCCESS) {
    respMsg->setAttachment(RDD_METADATA, metadata);
  }

  DVLOG(1) << "sending action response to client";
  idgs::actor::sendMessage(respMsg);
  removeAction(actionId);
}

void BaseRddActor::handleActionProcess(const ActorMessagePtr& msg) {
  DVLOG(3) << "handle action process";

  while (!actionQueue.empty()) {
    RddActionPtr action = actionQueue.front();
    actionQueue.pop();

    const ActorMessagePtr& actionMsg = action->getMessage();
    DVLOG(2) << actionMsg->toString();
    auto partitionActors = rddInfo.getPartitionInfo();
    for (int32_t partition = 0; partition < partitionActors.size(); ++partition) {
      if (action->getPartitionState(partition) != READY) {
        const ActorId& actorid = partitionActors[partition].getActorId();

        ActorMessagePtr reqMsg = actionMsg->createRouteMessage(actorid.member_id(), actorid.actor_id());
        reqMsg->setOperationName(actionMsg->getOperationName());
        reqMsg->setSourceActorId(getActorId());
        reqMsg->setSourceMemberId(localMemberId);

        DVLOG(2) << reqMsg->toString();
        idgs::actor::postMessage(reqMsg);
      }
    }
  }
}

void BaseRddActor::handleRddTransform(const ActorMessagePtr& msg) {
  DVLOG(2) << "RDD \"" << getRddName() << " handle transform.";
  switch (rddInfo.getState()) {
    case idgs::rdd::pb::INIT: {
      break;
    }
    case idgs::rdd::pb::PREPARED: {
      if (dependingRdds.empty()) {
        DVLOG(3) << "RDD \"" << getRddName() << "\" has no depending RDD, process work";
        handleRddProcess(msg);
      } else {
        rddInfo.setState(DEPENDENCY_PROCESSING);
        transformMsg = msg;
        int32_t readySize = 0;
        for (int32_t i = 0; i < dependingRdds.size(); ++i) {
          if (dependingRdds[i].getState() != READY) {
            shared_ptr<RddRequest> request(new RddRequest);
            ActorMessagePtr rddMsg = createActorMessage();
            rddMsg->setOperationName(RDD_TRANSFORM);
            rddMsg->setDestActorId(dependingRdds[i].getActorId().actor_id());
            rddMsg->setDestMemberId(dependingRdds[i].getActorId().member_id());
            rddMsg->setPayload(request);

            DVLOG(3) << "RDD \"" << getRddName() << "\" sending transform to depending RDD "
                        << dependingRdds[i].getActorId().actor_id() << " on "
                        << dependingRdds[i].getActorId().member_id();
            idgs::actor::postMessage(rddMsg);
          } else {
            ++readySize;
          }
        }

        if (readySize == dependingRdds.size()) {
          handleRddProcess(msg);
        }
      }
      break;
    }
    case idgs::rdd::pb::DEPENDENCY_PROCESSING: {
      break;
    }
    case idgs::rdd::pb::PROCESSING: {
      break;
    }
    case idgs::rdd::pb::READY: {
      processRddReady();
      break;
    }
    default: {
      break;
    }
  }
}

void BaseRddActor::handleRddPartitionReady(const ActorMessagePtr& msg) {
  PartitionProcessResult* result = dynamic_cast<PartitionProcessResult*>(msg->getPayload().get());

  DVLOG(3) << "RDD \"" << getRddName() << "\" receive partition ready response from partition " << result->partition();
  rddInfo.setPartitionState(result->partition(), P_READY);

  for (int32_t partition = 0; partition < partitionSize; ++partition) {
    if (rddInfo.getPartitionState(partition) != P_READY) {
      return;
    }
  }

  VLOG(1) << "RDD \"" << getRddName() << "\" is ready";

  rddInfo.setState(READY);
  processRddReady();
}

void BaseRddActor::handleDependingRddReady(const idgs::actor::ActorMessagePtr& msg) {
  DVLOG(2) << "depending rdd is ready" << dependingRdds.size();
  RddStateTracing* state = dynamic_cast<RddStateTracing*>(msg->getPayload().get());

  setDependingRddState(state->rdd_id(), state->state());

  for (int32_t i = 0; i < dependingRdds.size(); ++i) {
    if (dependingRdds[i].getState() != READY) {
      return;
    }
  }

  DVLOG(2) << "All depending rdds is ready, process it.";
  handleRddProcess(transformMsg);
}

void BaseRddActor::handleReceiveDependedRdd(const idgs::actor::ActorMessagePtr& msg) {
  ActorId* actorId = dynamic_cast<ActorId*>(msg->getPayload().get());
  addDependedRdd(*actorId);
}

void BaseRddActor::setDependingRddState(const ActorId& actorId, const pb::RddState& state) {
  for (int32_t i = 0; i < dependingRdds.size(); ++i) {
    if (dependingRdds[i].isTheRdd(actorId)) {
      dependingRdds[i].setState(state);
    }
  }
}

void BaseRddActor::handleGetPartitionActorResponse(const ActorMessagePtr& msg) {
  DVLOG(3) << "RDD \"" << getRddName() << "\" handle get delegate partition actor response";
  ++dependingRddResponse;
  RddActorInfo* result = dynamic_cast<RddActorInfo*>(msg->getPayload().get());

  for (int32_t j = 0; j < dependingRdds.size(); ++j) {
    if (dependingRdds[j].isTheRdd(result->rdd_id())) {
      for (int32_t i = 0; i < result->rdd_partition_size(); ++i) {
        uint32_t partition = result->rdd_partition(i).partition();
        ActorId actorId = result->rdd_partition(i).actor_id();
        dependingRdds[j].addPartitionInfo(partition, actorId);
      }

      break;
    }
  }

  if (dependingRdds.size() == dependingRddResponse) {
    for (size_t partition = 0; partition < partitionSize; ++partition) {
      shared_ptr<DependingRddPartition> payload(new DependingRddPartition);

      for (auto it = dependingRdds.begin(); it != dependingRdds.end(); ++it) {
        payload->add_depending_partition_actor_id(it->getPartitionInfo()[partition].getActorId().actor_id());
      }

      ActorMessagePtr reqMsg = createActorMessage();
      reqMsg->setOperationName(PARTITION_PREPARED_REQUEST);
      reqMsg->setDestActorId(rddInfo.getPartitionInfo()[partition].getActorId().actor_id());
      reqMsg->setDestMemberId(rddInfo.getPartitionInfo()[partition].getActorId().member_id());
      reqMsg->setPayload(payload);
      reqMsg->setAttachment(TRANSFORM_REQUEST, rawMsg->getPayload());

      idgs::actor::postMessage(reqMsg);
    }
  }
}

void BaseRddActor::handleSendRddInfoResponse(const idgs::actor::ActorMessagePtr& msg) {
  ++partitionResponse;
  DVLOG(3) << "RDD \"" << getRddName() << "\" receive partition ready response from partition " << partitionResponse
              << ", left " << (partitionSize - partitionResponse);
  if (partitionResponse == partitionSize) {
    DVLOG(2) << "Current RDD is prepared.";

    for (auto it = dependingRdds.begin(); it != dependingRdds.end(); ++it) {
      shared_ptr<RddRequest> payload(new RddRequest);
      auto message = createActorMessage();
      message->setDestActorId(it->getActorId().actor_id());
      message->setDestMemberId(it->getActorId().member_id());
      message->setOperationName(RDD_STATE_REQUEST);
      message->setPayload(payload);

      idgs::actor::postMessage(message);
    }
  }
}

void BaseRddActor::handleTransformComplete(const idgs::actor::ActorMessagePtr& msg) {
  PartitionProcessResult* response = dynamic_cast<PartitionProcessResult*>(msg->getPayload().get());
  DVLOG(3) << "RDD \"" << getRddName() << "\" received transform complete from partition " << response->partition();
  if (response->result_code() == RRC_SUCCESS) {
    rddInfo.setPartitionState(response->partition(), P_TRANSFORM_DONE);
  } else {
    LOG(ERROR)<< "error in handle partition " << response->partition() << ", caused by " << RddResultCode_Name(response->result_code());
  }

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(CHECK_PARTITION_READY);
  respMsg->setPayload(shared_ptr<RddRequest>(new RddRequest));

  idgs::actor::sendMessage(respMsg);
}

void BaseRddActor::handleRddStateRequest(const idgs::actor::ActorMessagePtr& msg) {
  shared_ptr<RddStateTracing> payload(new RddStateTracing);
  payload->mutable_rdd_id()->CopyFrom(rddInfo.getActorId());
  payload->set_state(rddInfo.getState());

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(RDD_STATE_RESPONSE);
  respMsg->setPayload(payload);

  idgs::actor::sendMessage(respMsg);
}

void BaseRddActor::handleRddStateResponse(const idgs::actor::ActorMessagePtr& msg) {
  RddStateTracing* state = dynamic_cast<RddStateTracing *>(msg->getPayload().get());

  if (state->state() == pb::INIT) {
    shared_ptr<RddRequest> payload(new RddRequest);
    auto message = createActorMessage();
    message->setDestActorId(state->rdd_id().actor_id());
    message->setDestMemberId(state->rdd_id().member_id());
    message->setOperationName(RDD_STATE_REQUEST);
    message->setPayload(payload);

    idgs::actor::sendMessage(message);
    return;
  } else if (state->state() == pb::ERROR) {
    return;
  }

  bool dependingPrepared = true;
  for (auto it = dependingRdds.begin(); it != dependingRdds.end(); ++it) {
    if (it->isTheRdd(state->rdd_id())) {
      it->setState(state->state());
    }

    if (it->getState() == pb::INIT || it->getState() == pb::ERROR) {
      dependingPrepared = false;
    }
  }

  if (dependingPrepared) {
    for (auto it = dependingRdds.begin(); it != dependingRdds.end(); ++it) {
      shared_ptr<RddRequest> payload(new RddRequest);
      ActorMessagePtr reqMsg = createActorMessage();
      reqMsg->setOperationName(GET_PARTITION_ACTOR);
      reqMsg->setDestActorId(it->getActorId().actor_id());
      reqMsg->setDestMemberId(it->getActorId().member_id());
      reqMsg->setPayload(payload);

      idgs::actor::postMessage(reqMsg);
    }
  }
}

void BaseRddActor::handleInsertRDDInfoResponse(const idgs::actor::ActorMessagePtr& msg) {
  idgs::store::pb::InsertResponse* response = dynamic_cast<idgs::store::pb::InsertResponse*>(msg->getPayload().get());
  if (response->result_code() != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR) << "save rdd name error, caused by " << StoreResultCode_Name(response->result_code());
  }
}

void BaseRddActor::processRddPrepared() {
  if (!actionQueue.empty()) {
    shared_ptr<RddRequest> request(new RddRequest);
    auto reqMsg = createActorMessage();
    reqMsg->setDestActorId(getActorId());
    reqMsg->setDestMemberId(localMemberId);
    reqMsg->setOperationName(RDD_TRANSFORM);
    reqMsg->setPayload(request);

    idgs::actor::sendMessage(reqMsg);
  }
}

void BaseRddActor::processRddReady() {
  DVLOG(2) << "send ready to depended rdds, size " << dependedRdds.size();
  if (getActorName() != STORE_DELEGATE_RDD_ACTOR) {
    shared_ptr<RddStateTracing> rddState(new RddStateTracing);
    rddState->set_state(READY);
    rddState->mutable_rdd_id()->set_actor_id(getActorId());
    rddState->mutable_rdd_id()->set_member_id(localMemberId);

    for (int32_t i = 0; i < dependedRdds.size(); ++i) {
      ActorMessagePtr message = createActorMessage();
      message->setOperationName(RDD_READY);
      message->setDestActorId(dependedRdds[i].actor_id());
      message->setDestMemberId(dependedRdds[i].member_id());
      message->setPayload(rddState);

      idgs::actor::postMessage(message);
    }
  }

  shared_ptr<RddRequest> request(new RddRequest);
  ActorMessagePtr message = createActorMessage();
  message->setDestActorId(getActorId());
  message->setDestMemberId(localMemberId);
  message->setOperationName(ACTION_MESSAGE_PROCESS);
  message->setPayload(request);
  idgs::actor::sendMessage(message);
}

void BaseRddActor::onDestroy() {
  // @todo double check and remove from RDD_NAME store.

  // @fixme notify upstream RDDs

  // @fixme notify downstream RDDs

  // @fixme destroy all partitions

}

std::shared_ptr<RddSnapshot> BaseRddActor::takeSnapShot() {
  shared_ptr<RddSnapshot> snap(new RddSnapshot);
  snap->setSelfInfo(rddInfo.getActorId(), rddInfo.getRddName());
  VLOG(2)<< "take a snapshot for RDD [RDD name:  " << snap->getSelfInfo().getRddName() << " || actor id:"
      << snap->getSelfInfo().getActorId().actor_id()
      << " || member id: " << snap->getSelfInfo().getActorId().member_id() << "]";
  std::vector<RddInfo>::iterator itr = dependingRdds.begin();
  for (; itr < dependingRdds.end(); ++ itr) {
    RddInfo& tmp = *itr;
    snap->addDependingRdd(tmp.getActorId(), tmp.getRddName());
  }

  return snap;
}


} // namespace rdd
} // namespace idgs 
