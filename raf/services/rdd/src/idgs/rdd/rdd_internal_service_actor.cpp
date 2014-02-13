/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "idgs/rdd/rdd_internal_service_actor.h"









namespace idgs {
namespace rdd {


RddInternalServiceActor::RddInternalServiceActor() {
  this->actorId = RDD_INTERNAL_SERVICE_ACTOR;
  descriptor = RddInternalServiceActor::generateActorDescriptor();
}

RddInternalServiceActor::~RddInternalServiceActor() {
}

const idgs::actor::ActorMessageHandlerMap& RddInternalServiceActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {CREATE_STORE_DELEGATE_RDD, static_cast<idgs::actor::ActorMessageHandler>(&RddInternalServiceActor::handleCreateStoreDelegate)},
      {CREATE_DELEGATE_PARTITION, static_cast<idgs::actor::ActorMessageHandler>(&RddInternalServiceActor::handleCreateDelegatePartition)},
      {CREATE_RDD, static_cast<idgs::actor::ActorMessageHandler>(&RddInternalServiceActor::handleCreateRdd)},
      {CREATE_RDD_PARTITION, static_cast<idgs::actor::ActorMessageHandler>(&RddInternalServiceActor::handleCreateRddPartition)},
      {RDD_ACTION_REQUEST, static_cast<idgs::actor::ActorMessageHandler>(&RddInternalServiceActor::handleRddActionRequest)},
      {idgs::actor::OP_DESTROY, static_cast<idgs::actor::ActorMessageHandler>(&RddInternalServiceActor::handleDestroyRddRequest)}
  };
  return handlerMap;
}

idgs::actor::ActorDescriptorPtr RddInternalServiceActor::generateActorDescriptor() {
  static idgs::actor::ActorDescriptorPtr descriptor;
  if (descriptor)
    return descriptor;
  descriptor.reset(new idgs::actor::ActorDescriptorWrapper);

  descriptor->setName(RDD_INTERNAL_SERVICE_ACTOR);
  descriptor->setDescription("RDD Service");
  descriptor->setType(idgs::pb::AT_STATELESS);

  // in operation
  idgs::actor::ActorOperationDescriporWrapper createDelegate;
  createDelegate.setName(CREATE_STORE_DELEGATE_RDD);
  createDelegate.setDescription("create store delegate rdd");
  createDelegate.setPayloadType("idgs.rdd.pb.CreateDelegateRddRequest");
  descriptor->setInOperation(createDelegate.getName(), createDelegate);

  idgs::actor::ActorOperationDescriporWrapper createDelegatePartition;
  createDelegatePartition.setName(CREATE_DELEGATE_PARTITION);
  createDelegatePartition.setDescription("create rdd partition of store delegate");
  createDelegatePartition.setPayloadType("idgs.rdd.pb.CreateDelegatePartitionRequest");
  descriptor->setInOperation(createDelegatePartition.getName(), createDelegatePartition);

  idgs::actor::ActorOperationDescriporWrapper filter;
  filter.setName(CREATE_RDD);
  filter.setDescription("create transform rdd");
  filter.setPayloadType("idgs.rdd.pb.CreateRddRequest");
  descriptor->setInOperation(filter.getName(), filter);

  idgs::actor::ActorOperationDescriporWrapper rddPartition;
  rddPartition.setName(CREATE_RDD_PARTITION);
  rddPartition.setDescription("create rdd partition");
  rddPartition.setPayloadType("idgs.rdd.pb.CreateRddPartitionRequest");
  descriptor->setInOperation(rddPartition.getName(), rddPartition);

  idgs::actor::ActorOperationDescriporWrapper destroyRdd;
  destroyRdd.setName(idgs::actor::OP_DESTROY);
  destroyRdd.setDescription("destroy named rdd");
  destroyRdd.setPayloadType("idgs.rdd.pb.DestroyRddRequest");
  descriptor->setInOperation(destroyRdd.getName(), destroyRdd);

  // out operation
  // out operation for CREATE_STORE_DELEGATE_RDD and CREATE_RDD
  idgs::actor::ActorOperationDescriporWrapper createRddResponse;
  createRddResponse.setName(CREATE_RDD_RESPONSE);
  createRddResponse.setDescription("the response of create rdd.");
  createRddResponse.setPayloadType("idgs.rdd.pb.CreateDelegateRddResponse");
  descriptor->setOutOperation(createRddResponse.getName(), createRddResponse);

  // out operation for CREATE_DELEGATE_PARTITION and CREATE_RDD_PARTITION
  idgs::actor::ActorOperationDescriporWrapper createPartitionResponse;
  createPartitionResponse.setName(CREATE_RDD_PARTITION_RESPONSE);
  createPartitionResponse.setDescription("response of create rdd partition");
  createPartitionResponse.setPayloadType("idgs.rdd.pb.CreateRddPartitionResponse");
  descriptor->setOutOperation(createPartitionResponse.getName(), createPartitionResponse);

  idgs::actor::ActorOperationDescriporWrapper actionRequest;
  actionRequest.setName(RDD_ACTION_REQUEST);
  actionRequest.setDescription("handle action for rdd");
  actionRequest.setPayloadType("idgs.rdd.pb.ActionRequest");
  descriptor->setInOperation(actionRequest.getName(), actionRequest);

  return descriptor;
}

void RddInternalServiceActor::handleCreateStoreDelegate(const idgs::actor::ActorMessagePtr& msg) {
}

void RddInternalServiceActor::handleCreateDelegatePartition(const idgs::actor::ActorMessagePtr& msg) {
}

void RddInternalServiceActor::handleCreateRdd(const idgs::actor::ActorMessagePtr& msg) {
}

void RddInternalServiceActor::handleCreateRddPartition(const idgs::actor::ActorMessagePtr& msg) {
}

void RddInternalServiceActor::handleRddActionRequest(const idgs::actor::ActorMessagePtr& msg) {
}

void RddInternalServiceActor::handleDestroyRddRequest(const idgs::actor::ActorMessagePtr& msg) {
}


} // namespace rdd
} // namespace idgs

