/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "pair_rdd_actor.h"

#include "idgs/application.h"

using namespace idgs::actor;

namespace idgs {
namespace rdd {

ActorDescriptorPtr PairRddActor::descriptor;

PairRddActor::PairRddActor(const std::string& rddname) {
  rddName = rddname;
}

PairRddActor::~PairRddActor() {
}

const ActorMessageHandlerMap& PairRddActor::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {CREATE_RDD,                     static_cast<idgs::actor::ActorMessageHandler>(&PairRddActor::handleRddCreate)},
      {CREATE_RDD_PARTITION_RESPONSE,  static_cast<idgs::actor::ActorMessageHandler>(&PairRddActor::handleCreateRddPartitionResponse)},
      {RDD_PARTITION_PREPARED,         static_cast<idgs::actor::ActorMessageHandler>(&PairRddActor::handleRddPartitionPrepared)},
      {RDD_TRANSFORM,                  static_cast<idgs::actor::ActorMessageHandler>(&PairRddActor::handleRddTransform)},
      {PARTITION_TRANSFORM_COMPLETE,   static_cast<idgs::actor::ActorMessageHandler>(&PairRddActor::handlePartitionTransformComplete)},
      {PARTITION_READY,                static_cast<idgs::actor::ActorMessageHandler>(&PairRddActor::handlePartitionReady)},
      {UPSTREAM_RDD_READY,             static_cast<idgs::actor::ActorMessageHandler>(&PairRddActor::handleUpstreamReady)},
      {UPSTREAM_RDD_ERROR,             static_cast<idgs::actor::ActorMessageHandler>(&PairRddActor::handleUpstreamError)},
      {RDD_ACTION_REQUEST,             static_cast<idgs::actor::ActorMessageHandler>(&PairRddActor::handleRddActionRequest)},
      {RDD_TRANSFORM_PREPARED,         static_cast<idgs::actor::ActorMessageHandler>(&PairRddActor::handleRddTransformPrepared)},
      {ACTION_MESSAGE_PROCESS,         static_cast<idgs::actor::ActorMessageHandler>(&PairRddActor::handleActionProcess)},
      {RDD_ACTION_RESPONSE,            static_cast<idgs::actor::ActorMessageHandler>(&PairRddActor::handleRddActionResponse)}
  };
  return handlerMap;
}

ActorDescriptorPtr PairRddActor::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = generateBaseActorDescriptor(PAIR_RDD_ACTOR);

  // in operation
  ActorOperationDescriporWrapper inCreateRdd;
  inCreateRdd.setName(CREATE_RDD);
  inCreateRdd.setDescription("Create RDD.");
  inCreateRdd.setPayloadType("idgs.rdd.pb.CreateRddRequest");
  descriptor->setInOperation(inCreateRdd.getName(), inCreateRdd);

  // out operation
  // out operation for CREATE_RDD
  ActorOperationDescriporWrapper createRddPartition;
  createRddPartition.setName(CREATE_RDD_PARTITION);
  createRddPartition.setDescription("Out for create RDD, to create RDD partition.");
  createRddPartition.setPayloadType("idgs.rdd.pb.CreateRddRequest");
  descriptor->setOutOperation(createRddPartition.getName(), createRddPartition);

  PairRddActor::descriptor = descriptor;
  return descriptor;
}

const ActorDescriptorPtr& PairRddActor::getDescriptor() const {
  return PairRddActor::descriptor;
}

void PairRddActor::handleRddCreate(const ActorMessagePtr& msg) {
  auto& members = idgs_application()->getMemberManager()->getMemberTable();
  auto it = members.begin();
  for (; it != members.end(); ++ it) {
    if (it->isLocalStore()) {
      ActorMessagePtr routeMsg = createActorMessage();
      routeMsg->setOperationName(CREATE_RDD_PARTITION);
      routeMsg->setDestMemberId(it->getId());
      routeMsg->setDestActorId(RDD_INTERNAL_SERVICE_ACTOR);
      routeMsg->setPayload(msg->getPayload());

      auto& attachment = msg->getRawAttachments();
      if (!attachment.empty()) {
        auto it = attachment.begin();
        for (; it != attachment.end(); ++ it) {
          routeMsg->setAttachment(it->first, it->second);
        }
      }

      postMessage(routeMsg);
    }
  }
}

} // namespace rdd
} // namespace idgs 
