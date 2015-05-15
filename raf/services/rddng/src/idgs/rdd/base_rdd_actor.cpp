
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "base_rdd_actor.h"

#include "idgs/application.h"

#include "idgs/rdd/rdd_module.h"
#include "idgs/rdd/pb/rdd_internal.pb.h"

using namespace std;
using namespace idgs::actor;
using namespace idgs::rdd::action;
using namespace idgs::rdd::pb;

namespace idgs {
namespace rdd {

size_t BaseRddActor::partitionSize;
uint32_t BaseRddActor::localMemberId;

BaseRddActor::BaseRddActor() : rddLocal(NULL), active(true) {
  auto cluster = idgs_application()->getClusterFramework();
  partitionSize = cluster->getPartitionCount();
  localMemberId = cluster->getLocalMember()->getId();
}

BaseRddActor::~BaseRddActor() {
  while (!actionQueue.empty()) {
    actionQueue.pop();
  }
  rddActions.clear();
}

const std::string& BaseRddActor::getRddName() const {
  return rddName;
}

void BaseRddActor::setRddLocal(const std::shared_ptr<RddLocal>& rddlocal) {
  rddLocal = rddlocal;
}

ActorDescriptorPtr BaseRddActor::generateBaseActorDescriptor(const std::string& actorName) {
  ActorDescriptorPtr descriptor = make_shared<ActorDescriptorWrapper>();
  descriptor->setName(actorName);
  descriptor->setDescription("RDD");
  descriptor->setType(idgs::pb::AT_STATEFUL);

  // in operation
  ActorOperationDescriporWrapper createRddPartitionResponse;
  createRddPartitionResponse.setName(CREATE_RDD_PARTITION_RESPONSE);
  createRddPartitionResponse.setDescription("The response of creating partition of RDD.");
  createRddPartitionResponse.setPayloadType("idgs.rdd.pb.CreateRddPartitionResponse");
  descriptor->setInOperation(createRddPartitionResponse.getName(), createRddPartitionResponse);

  ActorOperationDescriporWrapper rddPartitionPrepared;
  rddPartitionPrepared.setName(RDD_PARTITION_PREPARED);
  rddPartitionPrepared.setDescription("RDD partition is prepared.");
  rddPartitionPrepared.setPayloadType("idgs.rdd.pb.RddResponse");
  descriptor->setInOperation(rddPartitionPrepared.getName(), rddPartitionPrepared);

  ActorOperationDescriporWrapper rddTransform;
  rddTransform.setName(RDD_TRANSFORM);
  rddTransform.setDescription("Taking transform of RDD.");
  rddTransform.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setInOperation(rddTransform.getName(), rddTransform);

  ActorOperationDescriporWrapper partitionTransformComplete;
  partitionTransformComplete.setName(PARTITION_TRANSFORM_COMPLETE);
  partitionTransformComplete.setDescription("handle transform complete from each partition.");
  partitionTransformComplete.setPayloadType("idgs.rdd.pb.RddResponse");
  descriptor->setInOperation(partitionTransformComplete.getName(), partitionTransformComplete);

  ActorOperationDescriporWrapper partitionReady;
  partitionReady.setName(PARTITION_READY);
  partitionReady.setDescription("partitions of RDD is ready.");
  partitionReady.setPayloadType("idgs.rdd.pb.RddResponse");
  descriptor->setInOperation(partitionReady.getName(), partitionReady);

  ActorOperationDescriporWrapper upstreamReady;
  upstreamReady.setName(UPSTREAM_RDD_READY);
  upstreamReady.setDescription("handle when downstream RDD is ready.");
  upstreamReady.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setInOperation(upstreamReady.getName(), upstreamReady);

  ActorOperationDescriporWrapper upstreamError;
  upstreamError.setName(UPSTREAM_RDD_ERROR);
  upstreamError.setDescription("handle when downstream RDD is error.");
  upstreamError.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setInOperation(upstreamError.getName(), upstreamError);

  ActorOperationDescriporWrapper rddActionRequest;
  rddActionRequest.setName(RDD_ACTION_REQUEST);
  rddActionRequest.setDescription("Handle action of current RDD.");
  rddActionRequest.setPayloadType("idgs.rdd.pb.ActionRequest");
  descriptor->setInOperation(rddActionRequest.getName(), rddActionRequest);

  ActorOperationDescriporWrapper rddTransformPrepared;
  rddTransformPrepared.setName(RDD_TRANSFORM_PREPARED);
  rddTransformPrepared.setDescription("Prepared to run action.");
  rddTransformPrepared.setPayloadType("idgs.rdd.pb.RddResponse");
  descriptor->setInOperation(rddTransformPrepared.getName(), rddTransformPrepared);

  ActorOperationDescriporWrapper actionProcess;
  actionProcess.setName(ACTION_MESSAGE_PROCESS);
  actionProcess.setDescription("Process action message.");
  actionProcess.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setInOperation(actionProcess.getName(), actionProcess);

  ActorOperationDescriporWrapper rddActionResponse;
  rddActionResponse.setName(RDD_ACTION_RESPONSE);
  rddActionResponse.setDescription("Receive action response from each partition.");
  rddActionResponse.setPayloadType("idgs.rdd.pb.ActionResponse");
  descriptor->setInOperation(rddActionResponse.getName(), rddActionResponse);

  // out operation
  // out descriptor for message CREATE_RDD_PARTITION_RESPONSE and RDD_STATE_RESPONSE
  ActorOperationDescriporWrapper partitionCreated;
  partitionCreated.setName(PARTITION_CREATED);
  partitionCreated.setDescription("All partition is created, send RDD info to all local store member and parse transformer request.");
  partitionCreated.setPayloadType("idgs.rdd.pb.RddActorInfo");
  descriptor->setOutOperation(partitionCreated.getName(), partitionCreated);

  // out descriptor for message RDD_ACTION_REQUEST, RDD_ACTION_PREPARED, RDD_TRANSFORM, DEPENDING_RDD_READY, MULTICAST_RDD_INFO_RESPONSE
  descriptor->setOutOperation(rddTransform.getName(), rddTransform);

  // out descriptor for message DEPENDING_RDD_READY
  descriptor->setOutOperation(partitionReady.getName(), partitionReady);

  // out descriptor for message RDD_ACTION_REQUEST, RDD_ACTION_PREPARED, PARTITION_TRANSFORM_COMPLETE and idgs::pb::PS_READY
  descriptor->setOutOperation(actionProcess.getName(), actionProcess);

  // out descriptor for message RDD_TRANSFORM, PARTITION_TRANSFORM_COMPLETE and idgs::pb::PS_READY
  descriptor->setOutOperation(upstreamReady.getName(), upstreamReady);

  // out descriptor for message DEPENDING_RDD_ERROR
  descriptor->setOutOperation(upstreamError.getName(), upstreamError);

  // out descriptor for message RDD_ACTION_REQUEST and RDD_ACTION_RESPONSE
  descriptor->setOutOperation(rddActionResponse.getName(), rddActionResponse);

  // out descriptor for message RDD_TRANSFORM, RDD_PARTITION_PREPARED, idgs::pb::PS_READY, DEPENDING_RDD_READY, DEPENDING_RDD_ERROR
  ActorOperationDescriporWrapper rddStateSync;
  rddStateSync.setName(RDD_STATE_SYNC);
  rddStateSync.setDescription("Muticast state of RDD to all member when state changed");
  rddStateSync.setPayloadType("idgs.rdd.pb.RddStateTracing");
  descriptor->setOutOperation(rddStateSync.getName(), rddStateSync);

  // out descriptor for message RDD_ACTION_REQUEST
  ActorOperationDescriporWrapper persistTypeSync;
  persistTypeSync.setName(PERSIST_TYPE_SYNC);
  persistTypeSync.setDescription("Muticast persist type of RDD to all member when persist type changed");
  persistTypeSync.setPayloadType("idgs.rdd.pb.PersistInfo");
  descriptor->setOutOperation(persistTypeSync.getName(), persistTypeSync);

  return descriptor;
}

void BaseRddActor::handleCreateRddPartitionResponse(const ActorMessagePtr& msg) {
  if (rddLocal->getRddState() == ERROR) {
    return;
  }

  CreateRddPartitionResponse* payload = dynamic_cast<CreateRddPartitionResponse*>(msg->getPayload().get());
  auto code = payload->result_code();

  DVLOG(3) << getRddName() << " received create RDD partition response from member "
          << msg->getSourceMemberId() << " result is " << RddResultCode_Name(code);

  if (code != RRC_SUCCESS) {
    LOG(ERROR) << "RDD " << getRddName() << " : partition created error.";
    processRddError();
    return;
  } else {
    for (size_t i = 0; i < payload->rdd_partition_size(); ++ i) {
      auto& partition = payload->rdd_partition(i);
      rddLocal->setPartitionState(partition.partition(), CREATED);
      rddLocal->addPartitionActor(partition.partition(), partition.actor_id());
    }
  }

  for (int partition = 0; partition < partitionSize; ++ partition) {
    if (rddLocal->getPartitionState(partition) != CREATED) {
      return;
    }
  }

  DVLOG(3) << "All partition of RDD " << getRddName() << " is created, the state is CREATED.";
  shared_ptr<RddActorInfo> rddActorInfo = make_shared<RddActorInfo>();
  rddActorInfo->set_rdd_name(rddLocal->getRddName());
  rddActorInfo->set_state(PREPARED);
  rddActorInfo->mutable_rdd_id()->CopyFrom(rddLocal->getRddId());
  for (size_t partition = 0; partition < partitionSize; ++ partition) {
    auto partitionInfo = rddActorInfo->add_rdd_partition();
    partitionInfo->set_partition(partition);
    partitionInfo->mutable_actor_id()->CopyFrom(rddLocal->getPartitionActor(partition));
  }

  multicastRddMessage(PARTITION_CREATED, rddActorInfo);
}

void BaseRddActor::handleRddPartitionPrepared(const ActorMessagePtr& msg) {
  if (rddLocal->getRddState() == ERROR) {
    return;
  }

  RddResponse* response = dynamic_cast<RddResponse*>(msg->getPayload().get());

  auto srcMemberId = msg->getSourceMemberId();
  auto code = response->result_code();

  DVLOG(3) << "RDD " << getRddName() << " received RDD partition prepared from member "
           << srcMemberId << " result is " << RddResultCode_Name(code);

  if (code != RRC_SUCCESS) {
    LOG(ERROR) << "RDD " << getRddName() << " : partition prepared error.";
    processRddError();
    return;
  }

  bool prepared = true;
  auto partitionMgr = idgs_application()->getPartitionManager();
  for (size_t partition = 0; partition < partitionSize; ++ partition) {
    if (srcMemberId == partitionMgr->getPartition(partition)->getPrimaryMemberId()) {
      rddLocal->setPartitionState(partition, PREPARED);
    } else if (prepared) {
      prepared = (rddLocal->getPartitionState(partition) == PREPARED);
    }
  }

  if (prepared) {
    if (rddLocal->isDelegateRdd()) {
      VLOG(1) << "Store delegate RDD " << getRddName() << " is ready, the state is READY.";
      rddLocal->setRddState(READY);
    } else {
      VLOG(1) << "RDD " << getRddName() << " is prepared, the state is PERPARED.";
      rddLocal->setRddState(PREPARED);
    }

    processRddStateChanged();

    if (!actionQueue.empty()) {
      auto message = createActorMessage();
      message->setOperationName(RDD_TRANSFORM);
      message->setDestActorId(getActorId());
      message->setDestMemberId(localMemberId);
      message->setPayload(make_shared<RddRequest>());
      sendMessage(message);
    }
  }
}

void BaseRddActor::handleRddTransform(const ActorMessagePtr& msg) {
  auto& state = rddLocal->getRddState();
  DVLOG(3) << "RDD " << getRddName() << " handle transform request, state is " << RddState_Name(state);
  if (state == ERROR) {
    processRddError();
    return;
  } else if (state == idgs::rdd::pb::INIT || state == CREATED) {
    postMessage(const_cast<ActorMessagePtr&>(msg));
    return;
  } else if (state == PREPARED) {
    DVLOG(3) << "RDD " << getRddName() << " call depending RDD to transform.";
    rddLocal->setRddState(PROCESSING);
    processRddStateChanged();

    auto& upstreamRddLocal = rddLocal->getUpstreamRddLocal();
    for (int32_t i = 0; i < upstreamRddLocal.size(); ++ i) {
      upstreamRddLocal[i]->setTransformed(false);
    }

    if (!actionQueue.empty() && rddLocal->getPersistType() == NONE) {
      for (size_t partition = 0; partition < partitionSize; ++ partition) {
        rddLocal->setPartitionState(partition, INIT);
      }

      shared_ptr<PersistInfo> payload = make_shared<PersistInfo>();
      payload->set_rdd_name(getRddName());
      payload->set_persist_type(ORDERED);

      multicastRddMessage(PERSIST_TYPE_SYNC, payload);
    } else {
      for (size_t partition = 0; partition < partitionSize; ++ partition) {
        rddLocal->setPartitionState(partition, PREPARED);
      }

      callUpstreamTransform();
    }
  } else if (state == READY) {
    VLOG(2) << "RDD " << getRddName() << " start to do transform.";
    rddLocal->setRddState(PROCESSING);
    processRddStateChanged();

    for (size_t partition = 0; partition < partitionSize; ++ partition) {
      rddLocal->setPartitionState(partition, PREPARED);
      auto& partitionActor = rddLocal->getPartitionActor(partition);
      auto routeMsg = msg->createRouteMessage(partitionActor.member_id(), partitionActor.actor_id());
      routeMsg->setSourceActorId(getActorId());
      routeMsg->setSourceMemberId(localMemberId);
      postMessage(routeMsg);
    }
  } else {
    return;
  }

  auto& downstreamRddLocal = rddLocal->getDownstreamRddLocal();
  if (!downstreamRddLocal.empty()) {
    auto it = downstreamRddLocal.begin();
    for (; it != downstreamRddLocal.end(); ++ it) {
      auto& actorId = (* it)->getRddId().actor_id();
      auto memberId = (* it)->getRddId().member_id();
      if (actorId == msg->getSourceActorId() && memberId == msg->getSourceMemberId()) {
        break;
      }

      auto message = createActorMessage();
      message->setOperationName(RDD_TRANSFORM);
      message->setDestActorId(actorId);
      message->setDestMemberId(memberId);
      message->setPayload(make_shared<RddRequest>());
      postMessage(message);
    }
  }
}

void BaseRddActor::handleRddTransformPrepared(const idgs::actor::ActorMessagePtr& msg) {
  auto srcMemberId = msg->getSourceMemberId();
  DVLOG(3) << "RDD " << getRddName() << " received RDD transform prepared from member " << srcMemberId;

  bool prepared = true;
  auto partitionMgr = idgs_application()->getPartitionManager();
  for (size_t partition = 0; partition < partitionSize; ++ partition) {
    if (srcMemberId == partitionMgr->getPartition(partition)->getPrimaryMemberId()) {
      rddLocal->setPartitionState(partition, PREPARED);
    } else if (prepared) {
      prepared = (rddLocal->getPartitionState(partition) == PREPARED);
    }
  }

  if (prepared) {
    callUpstreamTransform();
  }
}

void BaseRddActor::handlePartitionTransformComplete(const ActorMessagePtr& msg) {
  RddResponse* payload = dynamic_cast<RddResponse*>(msg->getPayload().get());
  auto code = payload->result_code();
  DVLOG(3) << "RDD " << getRddName() << " receive transform complete response from partition "
           << payload->partition() << " result is " << RddResultCode_Name(code) << ".";

  if (code != RRC_SUCCESS) {
    rddLocal->setPartitionState(payload->partition(), ERROR);
  } else {
    rddLocal->setPartitionState(payload->partition(), READY);
  }

  for (size_t partition = 0; partition < partitionSize; ++ partition) {
    if (rddLocal->getPartitionState(partition) != READY) {
      return;
    }
  }

  processRddReady();
}

void BaseRddActor::handlePartitionReady(const ActorMessagePtr& msg) {
  bool persisted = rddLocal->getPersistType() != NONE;
  auto state = persisted ? TRANSFORM_COMPLETE : READY;
  RddResponse* response = dynamic_cast<RddResponse*>(msg->getPayload().get());
  rddLocal->setPartitionState(response->partition(), state);
  DVLOG(3) << "RDD " << getRddName() << " received partition ready from partition " << to_string(response->partition());

  for (size_t partition = 0; partition < partitionSize; ++ partition) {
    if (rddLocal->getPartitionState(partition) != state) {
      return;
    }
  }

  VLOG(2) << "all partition of RDD " << getRddName() << " is ready, the state is READY.";
  if (persisted) {
    for (size_t partition = 0; partition < partitionSize; ++ partition) {
      auto& partitionActor = rddLocal->getPartitionActor(partition);
      auto message = createActorMessage();
      message->setOperationName(RDD_TRANSFORM);
      message->setDestActorId(partitionActor.actor_id());
      message->setDestMemberId(partitionActor.member_id());
      message->setPayload(make_shared<RddRequest>());

      sendMessage(message);
    }
  } else {
    processRddReady();
  }
}

void BaseRddActor::handleUpstreamReady(const ActorMessagePtr& msg) {
  DVLOG(3) << "RDD " << getRddName() << " receive depending RDD is ready";
  auto& upstreamRddLocal = rddLocal->getUpstreamRddLocal();
  bool upstreamReady = true;
  bool paramReady = true;
  for (int32_t i = 0; i < upstreamRddLocal.size(); ++ i) {
    auto& rddId = upstreamRddLocal[i]->getRddId();
    if (rddId.actor_id() == msg->getSourceActorId() &&
        rddId.member_id() == msg->getSourceMemberId()) {
      upstreamRddLocal[i]->setTransformed(true);
    } else {
      if (!upstreamRddLocal[i]->isTransformed()) {
        upstreamReady = false;
        if (i > 0) {
          paramReady = false;
        }
      }
    }
  }

  if (!rddLocal->isUpstreamSync()) {
    auto& fstDownRddLocal = upstreamRddLocal[0];
    if (!fstDownRddLocal->isTransformed() && paramReady) {
      auto message = createActorMessage();
      message->setOperationName(RDD_TRANSFORM);
      message->setDestActorId(fstDownRddLocal->getRddId().actor_id());
      message->setDestMemberId(fstDownRddLocal->getRddId().member_id());
      message->setPayload(make_shared<RddRequest>());

      DVLOG(2) << getRddName() << " call first down RDD " << fstDownRddLocal->getRddName() << " to do transform.";
      postMessage(message);
      return;
    }
  }

  if (upstreamReady) {
    DVLOG(3) << "All downstream RDD of " << getRddName() << " is ready";
    for (size_t partition = 0; partition < partitionSize; ++ partition) {
      auto& partitionActor = rddLocal->getPartitionActor(partition);
      auto message = createActorMessage();
      message->setOperationName(CHECK_PARTITION_READY);
      message->setDestActorId(partitionActor.actor_id());
      message->setDestMemberId(partitionActor.member_id());
      message->setPayload(make_shared<RddRequest>());

      sendMessage(message);
    }
  }
}

void BaseRddActor::handleUpstreamError(const idgs::actor::ActorMessagePtr& msg) {
  LOG(ERROR) << "RDD " << getRddName() << " is error.";
  processRddError();
}

void BaseRddActor::handleRddActionRequest(const ActorMessagePtr& msg) {
  ActionRequest* request = dynamic_cast<ActionRequest*>(msg->getPayload().get());
  DVLOG(3) << "RDD " << getRddName() << " receive action request, id : " << request->action_id()
           << " operation : " << request->action_op_name();

  auto& state = rddLocal->getRddState();
  VLOG(1) << "RDD " << getRddName() << " state is " << RddState_Name(state);
  if (state == ERROR) {
    LOG(ERROR)<< "RDD " << getRddName() << " is error, please check and rebuild RDD.";
    auto respMsg = msg->createResponse();
    respMsg->setOperationName(RDD_ACTION_RESPONSE);
    shared_ptr<ActionResponse> response = make_shared<ActionResponse>();
    response->set_action_id(request->action_id());
    response->set_result_code(RRC_RDD_ERROR);
    respMsg->setPayload(response);
    sendMessage(respMsg);

    return;
  } else if (state == PROCESSING) {
    LOG(ERROR)<< "RDD " << getRddName() << " is process, please try again.";
    auto respMsg = msg->createResponse();
    respMsg->setOperationName(RDD_ACTION_RESPONSE);
    shared_ptr<ActionResponse> response = make_shared<ActionResponse>();
    response->set_action_id(request->action_id());
    response->set_result_code(RRC_RDD_STATE_PROCESSING);
    respMsg->setPayload(response);
    sendMessage(respMsg);

    return;
  }

  RddActionPtr action = make_shared<RddAction>();
  action->setActionId(request->action_id());
  action->setActionOpName(request->action_op_name());
  action->setMessage(msg);

  rddActions.insert(pair<string, RddActionPtr>(request->action_id(), action));
  actionQueue.push(action);

  if (state == PREPARED) {
    DVLOG(3) << "RDD " << getRddName() << " call transform.";
    auto message = createActorMessage();
    message->setOperationName(RDD_TRANSFORM);
    message->setDestActorId(getActorId());
    message->setDestMemberId(localMemberId);
    message->setPayload(make_shared<RddRequest>());
    sendMessage(message);
  } else if (state == READY) {
    DVLOG(3) << "RDD " << getRddName() << " call action.";
    ActorMessagePtr message = createActorMessage();
    message->setOperationName(ACTION_MESSAGE_PROCESS);
    message->setDestActorId(getActorId());
    message->setDestMemberId(localMemberId);
    message->setPayload(make_shared<RddRequest>());
    sendMessage(message);
  }
}

void BaseRddActor::handleActionProcess(const ActorMessagePtr& msg) {
  auto state = rddLocal->getRddState();
  while (!actionQueue.empty()) {
    RddActionPtr action = actionQueue.front();
    VLOG(2) << "RDD " << getRddName() << " start to process action " << action->getActionId()
            << " with operation " << action->getActionOpName();

    actionQueue.pop();

    const ActorMessagePtr& actionMsg = action->getMessage();
    if (state == ERROR) {
      shared_ptr<ActionResponse> response = make_shared<ActionResponse>();
      response->set_result_code(RRC_RDD_ERROR);

      auto respMsg = actionMsg->createResponse();
      respMsg->setOperationName(RDD_ACTION_RESPONSE);
      respMsg->setPayload(response);

      sendMessage(respMsg);
    } else if (state == READY) {
      for (size_t partition = 0; partition < partitionSize; ++ partition) {
        if (action->getPartitionState(partition) != READY) {
          auto partitionActors = rddLocal->getPartitionActor(partition);

          ActorMessagePtr reqMsg = actionMsg->createRouteMessage(partitionActors.member_id(), partitionActors.actor_id());
          reqMsg->setSourceActorId(getActorId());
          reqMsg->setSourceMemberId(localMemberId);

          postMessage(reqMsg);
        }
      }
    }
  }
}

void BaseRddActor::handleRddActionResponse(const ActorMessagePtr& msg) {
  ActionResponse* response = dynamic_cast<ActionResponse*>(msg->getPayload().get());
  string actionId = response->action_id();
  uint32_t partition = response->partition();

  DVLOG(3) << "RDD " << getRddName() << " receive action response from partition " << to_string(partition);

  auto it = rddActions.find(actionId);
  const RddActionPtr& action = it->second;
  action->setPartitionState(partition, READY);

  if (response->result_code() != RRC_SUCCESS) {
    action->setActionResultCode(response->result_code());
    LOG(ERROR)<< "action error in partition " << partition << " cause by " << RddResultCode_Name(response->result_code());
  }

  for (int32_t i = 0; i < response->action_value_size(); ++ i) {
    action->addActionResult(partition, response->action_value(i));
  }

  if (action->getState() != READY) {
    return;
  }

  DVLOG(3) << " All partitions of RDD " << getRddName() << " process action ready, aggregate result";
  ActorMessagePtr message = action->getMessage();
  auto respMsg = message->createResponse();
  respMsg->setOperationName(RDD_ACTION_RESPONSE);

  shared_ptr<ActionResponse> actionResponse = make_shared<ActionResponse>();
  actionResponse->set_action_id(actionId);

  auto code = action->getActionResultCode();
  if (code == RRC_SUCCESS) {
    ActionMgr& actionMgr = *idgs_rdd_module()->getActionManager();
    const ActionPtr& actionPtr = actionMgr.get(action->getActionOpName());

    if (action.get()) {
      action::ActionContext ctx;
      ctx.initContext(&action->getMessage());
      ctx.setAggregateResult(action->getActionResult());

      RddResultCode code = actionPtr->aggregate(&ctx);
      actionResponse->set_result_code(code);

      auto& actionResult = ctx.getActionResult();
      if (actionResult) {
        respMsg->setAttachment(ACTION_RESULT, actionResult);
      }
    } else {
      LOG(ERROR)<< "action " << action->getActionOpName() << " is not registered.";
      code = RRC_ACTION_NOT_FOUND;
    }
  }

  actionResponse->set_result_code(code);
  respMsg->setPayload(actionResponse);
  respMsg->setAttachment(KEY_METADATA, rddLocal->getKeyMetadata());
  respMsg->setAttachment(VALUE_METADATA, rddLocal->getValueMetadata());

  VLOG(1) << "RDD " << getRddName() << " action " << actionId << " process done, send response to client.";
  rddActions.erase(actionId);
  idgs::actor::sendMessage(respMsg);
}

void BaseRddActor::callUpstreamTransform() {
  int32_t startIndex = 0;
  shared_ptr<RddRequest> payload = make_shared<RddRequest>();
  if (!rddLocal->isUpstreamSync()) {
    startIndex = 1;
  }

  auto& upstreamRddLocal = rddLocal->getUpstreamRddLocal();
  for (int32_t i = startIndex; i < upstreamRddLocal.size(); ++ i) {
    auto& rddId = upstreamRddLocal[i]->getRddId();
    auto message = createActorMessage();
    message->setOperationName(RDD_TRANSFORM);
    message->setSourceActorId(getActorId());
    message->setSourceMemberId(localMemberId);
    message->setDestActorId(rddId.actor_id());
    message->setDestMemberId(rddId.member_id());
    message->setPayload(payload);

    postMessage(message);
  }
}

void BaseRddActor::processRddReady() {
  if (rddLocal->getPersistType() == NONE) {
    rddLocal->setRddState(PREPARED);
  } else {
    rddLocal->setRddState(READY);
  }

  processRddStateChanged();

  if (!actionQueue.empty()) {
    ActorMessagePtr actionMsg = createActorMessage();
    actionMsg->setOperationName(ACTION_MESSAGE_PROCESS);
    actionMsg->setDestActorId(getActorId());
    actionMsg->setDestMemberId(localMemberId);
    actionMsg->setPayload(make_shared<RddRequest>());

    sendMessage(actionMsg);
  }

  auto& downstreamRddLocal = rddLocal->getDownstreamRddLocal();
  if (!downstreamRddLocal.empty()) {
    auto it = downstreamRddLocal.begin();
    for (; it != downstreamRddLocal.end(); ++ it) {
      auto& rddId = (* it)->getRddId();
      DVLOG(3) << "RDD " << getRddName() << " is ready, call downstream RDD("
               << to_string(rddId.member_id()) << ", " << rddId.actor_id() << ")";
      if (!rddId.has_actor_id() || rddId.member_id() == -3) {
        LOG(ERROR) << (* it)->getRddName() << " has no actor found.";
        continue;
      }

      ActorMessagePtr message = createActorMessage();
      message->setOperationName(UPSTREAM_RDD_READY);
      message->setDestActorId(rddId.actor_id());
      message->setDestMemberId(rddId.member_id());
      message->setPayload(make_shared<RddRequest>());

      sendMessage(message);
    }
  }
}

void BaseRddActor::processRddError() {
  rddLocal->setRddState(ERROR);
  processRddStateChanged();

  while (!actionQueue.empty()) {
    auto action = actionQueue.front();
    actionQueue.pop();

    shared_ptr<ActionResponse> response = make_shared<ActionResponse>();
    response->set_action_id(action->getActionId());
    response->set_result_code(RRC_RDD_ERROR);

    auto respMsg = action->getMessage()->createResponse();
    respMsg->setOperationName(RDD_ACTION_RESPONSE);
    respMsg->setPayload(response);

    sendMessage(respMsg);
  }

  auto& downstreamRddLocal = rddLocal->getDownstreamRddLocal();
  auto it = downstreamRddLocal.begin();
  for (; it != downstreamRddLocal.end(); ++ it) {
    auto& rddId = (* it)->getRddId();
    auto msg = createActorMessage();
    msg->setOperationName(UPSTREAM_RDD_ERROR);
    msg->setDestActorId(rddId.actor_id());
    msg->setDestMemberId(rddId.member_id());
    msg->setPayload(make_shared<RddRequest>());

    postMessage(msg);
  }
}

void BaseRddActor::processRddStateChanged() {
  shared_ptr<RddStateTracing> payload = make_shared<RddStateTracing>();
  payload->set_rdd_name(rddLocal->getRddName());
  payload->mutable_rdd_id()->set_actor_id(getActorId());
  payload->mutable_rdd_id()->set_member_id(localMemberId);
  payload->set_state(rddLocal->getRddState());

  multicastRddMessage(RDD_STATE_SYNC, payload);
}

void BaseRddActor::onDownstreamRemoved() {
  if (!active) {
    terminate();
  }
}

void BaseRddActor::onDestroy() {
  DVLOG(3) << "destroy RDD " << getRddName() << " with actor id " << getActorId();

  active = false;
  if (rddLocal->getDownstreamRddLocal().size()) {
    return;
  }
  state.store(TERMINATE);
  // clear actor inner message queue
  msg_queue.clear();

  // muticast internal service actor to remove rddLocal
  shared_ptr<DestroyRddRequest> request = make_shared<DestroyRddRequest>();
  request->set_rdd_name(rddLocal->getRddName());
  multicastRddMessage(REMOVE_RDD_LOCAL, request);

  idgs::actor::StatefulActor::onDestroy();
}

void BaseRddActor::multicastRddMessage(const std::string& opName, const idgs::actor::PbMessagePtr& payload) {
  auto cluster = idgs_application()->getClusterFramework();
  auto members = cluster->getMemberManager()->getMemberTable();
  auto it = members.begin();
  for (; it != members.end(); ++ it) {
    if (it->isAvailable() && it->isLocalStore()) {
      ActorMessagePtr message = createActorMessage();
      message->setOperationName(opName);
      message->setDestMemberId(it->getId());
      message->setDestActorId(RDD_INTERNAL_SERVICE_ACTOR);
      message->setPayload(payload);
      postMessage(message);
    }
  }
}

} // namespace rdd
} // namespace idgs 
