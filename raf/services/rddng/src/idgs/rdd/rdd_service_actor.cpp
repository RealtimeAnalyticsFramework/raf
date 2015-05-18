
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "rdd_service_actor.h"

#include "idgs/actor/rpc_framework.h"
#include "idgs/rdd/pb/rdd_internal.pb.h"
#include "idgs/rdd/pb/rdd_svc.pb.h"

using namespace idgs::pb;
using namespace idgs::actor;
using namespace idgs::rdd::pb;

namespace idgs {
namespace rdd {

RddServiceActor::RddServiceActor() {
  this->actorId = RDD_SERVICE_ACTOR;

  descriptor = RddServiceActor::generateActorDescriptor();
}

RddServiceActor::~RddServiceActor() {
}

const ActorMessageHandlerMap& RddServiceActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {CREATE_STORE_DELEGATE_RDD, {
          static_cast<ActorMessageHandler>(&RddServiceActor::handleToInternalService),
          &idgs::rdd::pb::CreateDelegateRddRequest::default_instance()
      }},
      {CREATE_RDD, {
          static_cast<ActorMessageHandler>(&RddServiceActor::handleToInternalService),
          &idgs::rdd::pb::CreateRddRequest::default_instance()
      }},
      {RDD_ACTION_REQUEST,  {
          static_cast<ActorMessageHandler>(&RddServiceActor::handleToInternalService),
          &idgs::rdd::pb::ActionRequest::default_instance()
      }},
      {RDD_DESTROY, {
          static_cast<ActorMessageHandler>(&RddServiceActor::handleToInternalService),
          &idgs::rdd::pb::DestroyRddRequest::default_instance()
      }},

      {OID_LIST_RDD,  {
          static_cast<ActorMessageHandler>(&RddServiceActor::handleToInternalService),
          NULL
      }},

  };
  return handlerMap;
}

ActorDescriptorPtr RddServiceActor::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = std::make_shared<ActorDescriptorWrapper>();

  descriptor->setName(RDD_SERVICE_ACTOR);
  descriptor->setDescription("RDD Service");
  descriptor->setType(AT_STATELESS);

  // in operation
  ActorOperationDescriporWrapper createDelegate;
  createDelegate.setName(CREATE_STORE_DELEGATE_RDD);
  createDelegate.setDescription("create store delegate RDD");
  createDelegate.setPayloadType("idgs.rdd.pb.CreateDelegateRddRequest");
  descriptor->setInOperation(createDelegate.getName(), createDelegate);

  ActorOperationDescriporWrapper createRdd;
  createRdd.setName(CREATE_RDD);
  createRdd.setDescription("create RDD");
  createRdd.setPayloadType("idgs.rdd.pb.CreateRddRequest");
  descriptor->setInOperation(createRdd.getName(), createRdd);

  ActorOperationDescriporWrapper rddActionRequest;
  rddActionRequest.setName(RDD_ACTION_REQUEST);
  rddActionRequest.setDescription("handle action for RDD");
  rddActionRequest.setPayloadType("idgs.rdd.pb.ActionRequest");
  descriptor->setInOperation(rddActionRequest.getName(), rddActionRequest);

  ActorOperationDescriporWrapper rddDestroy;
  rddDestroy.setName(RDD_DESTROY);
  rddDestroy.setDescription("destroy named RDD");
  rddDestroy.setPayloadType("idgs.rdd.pb.DestroyRddRequest");
  descriptor->setInOperation(rddDestroy.getName(), rddDestroy);

  // out operation
  // out operation for CREATE_STORE_DELEGATE_RDD
  descriptor->setOutOperation(createDelegate.getName(), createDelegate);

  // out operation for CREATE_RDD
  descriptor->setOutOperation(createRdd.getName(), createRdd);

  // out operation for RDD_ACTION_REQUEST
  descriptor->setOutOperation(rddActionRequest.getName(), rddActionRequest);

  // out operation for RDD_DESTROY
  descriptor->setOutOperation(rddDestroy.getName(), rddDestroy);

  return descriptor;
}

void RddServiceActor::handleToInternalService(const ActorMessagePtr& msg) {
  DVLOG(3) << "receive request " << msg->getOperationName() << " from client.";
  ActorMessagePtr routeMsg = msg->createRouteMessage(msg->getDestMemberId(), RDD_INTERNAL_SERVICE_ACTOR);
  sendMessage(routeMsg);
}

} // namespace rdd
} // namespace idgs 
