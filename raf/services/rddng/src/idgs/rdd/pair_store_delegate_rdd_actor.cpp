
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "pair_store_delegate_rdd_actor.h"

#include "idgs/application.h"

using namespace idgs::actor;

namespace idgs {
namespace rdd {

ActorDescriptorPtr PairStoreDelegateRddActor::descriptor;

PairStoreDelegateRddActor::PairStoreDelegateRddActor(const std::string& rddname) {
  rddName = rddname;
}

PairStoreDelegateRddActor::~PairStoreDelegateRddActor() {
}

const ActorMessageHandlerMap& PairStoreDelegateRddActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {CREATE_STORE_DELEGATE_RDD,      static_cast<idgs::actor::ActorMessageHandler>(&PairStoreDelegateRddActor::handleRddCreate)},
      {CREATE_RDD_PARTITION_RESPONSE,  static_cast<idgs::actor::ActorMessageHandler>(&PairStoreDelegateRddActor::handleCreateRddPartitionResponse)},
      {RDD_PARTITION_PREPARED,         static_cast<idgs::actor::ActorMessageHandler>(&PairStoreDelegateRddActor::handleRddPartitionPrepared)},
      {RDD_TRANSFORM,                  static_cast<idgs::actor::ActorMessageHandler>(&PairStoreDelegateRddActor::handleRddTransform)},
      {PARTITION_TRANSFORM_COMPLETE,   static_cast<idgs::actor::ActorMessageHandler>(&PairStoreDelegateRddActor::handlePartitionTransformComplete)},
      {PARTITION_READY,                static_cast<idgs::actor::ActorMessageHandler>(&PairStoreDelegateRddActor::handlePartitionReady)},
      {UPSTREAM_RDD_READY,            static_cast<idgs::actor::ActorMessageHandler>(&PairStoreDelegateRddActor::handleUpstreamReady)},
      {UPSTREAM_RDD_ERROR,            static_cast<idgs::actor::ActorMessageHandler>(&PairStoreDelegateRddActor::handleUpstreamError)},
      {RDD_ACTION_REQUEST,             static_cast<idgs::actor::ActorMessageHandler>(&PairStoreDelegateRddActor::handleRddActionRequest)},
      {RDD_TRANSFORM_PREPARED,         static_cast<idgs::actor::ActorMessageHandler>(&PairStoreDelegateRddActor::handleRddTransformPrepared)},
      {ACTION_MESSAGE_PROCESS,         static_cast<idgs::actor::ActorMessageHandler>(&PairStoreDelegateRddActor::handleActionProcess)},
      {RDD_ACTION_RESPONSE,            static_cast<idgs::actor::ActorMessageHandler>(&PairStoreDelegateRddActor::handleRddActionResponse)}
  };
  return handlerMap;
}

ActorDescriptorPtr PairStoreDelegateRddActor::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = generateBaseActorDescriptor(PAIR_STORE_DELEGATE_RDD_ACTOR);

  // in operation
  ActorOperationDescriporWrapper inCreateDelegateRdd;
  inCreateDelegateRdd.setName(CREATE_STORE_DELEGATE_RDD);
  inCreateDelegateRdd.setDescription("Create store delegate RDD.");
  inCreateDelegateRdd.setPayloadType("idgs.rdd.pb.CreateDelegateRddRequest");
  descriptor->setInOperation(inCreateDelegateRdd.getName(), inCreateDelegateRdd);

  // out operation
  // out operation for CREATE_STORE_DELEGATE_RDD
  ActorOperationDescriporWrapper inCreateDelegatePartition;
  inCreateDelegatePartition.setName(CREATE_DELEGATE_PARTITION);
  inCreateDelegatePartition.setDescription("Out for create store delegate RDD, to create delegate partition.");
  inCreateDelegatePartition.setPayloadType("idgs.rdd.pb.CreateDelegateRddRequest");
  descriptor->setOutOperation(inCreateDelegatePartition.getName(), inCreateDelegatePartition);

  PairStoreDelegateRddActor::descriptor = descriptor;
  return descriptor;
}

const idgs::actor::ActorDescriptorPtr& PairStoreDelegateRddActor::getDescriptor() const {
  return PairStoreDelegateRddActor::descriptor;
}

void PairStoreDelegateRddActor::handleRddCreate(const ActorMessagePtr& msg) {
  auto members = idgs_application()->getMemberManager()->getMemberTable();
  auto it = members.begin();
  for (; it != members.end(); ++ it) {
    if (it->isLocalStore()) {
      ActorMessagePtr routeMsg = createActorMessage();
      routeMsg->setOperationName(CREATE_DELEGATE_PARTITION);
      routeMsg->setDestMemberId(it->getId());
      routeMsg->setDestActorId(RDD_INTERNAL_SERVICE_ACTOR);
      routeMsg->setPayload(msg->getPayload());
      postMessage(routeMsg);
    }
  }
}

} // namespace rdd
} // namespace idgs 
