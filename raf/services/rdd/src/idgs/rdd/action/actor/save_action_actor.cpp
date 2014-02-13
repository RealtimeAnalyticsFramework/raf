
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // defined(__GNUC__) || defined(__clang__) $

#include "idgs/rdd/action/actor/save_action_actor.h"

namespace idgs {
namespace rdd {
namespace action {
namespace actor {

SaveActionActor::SaveActionActor() {
  this->actorId = RDD_SAVE_ACTION_ACTOR;
  descriptor = SaveActionActor::generateActorDescriptor();
  ::idgs::util::singleton<idgs::actor::RpcFramework>::getInstance().getActorFramework()->Register(this->getActorId(), this);
}

std::shared_ptr<idgs::actor::ActorDescriptorWrapper> SaveActionActor::generateActorDescriptor() {
  static idgs::actor::ActorDescriptorPtr descriptor;
  if (descriptor)
    return descriptor;
  descriptor.reset(new idgs::actor::ActorDescriptorWrapper);

  descriptor->setName(RDD_INTERNAL_SERVICE_ACTOR);
  descriptor->setDescription("RDD Save Action Service");
  descriptor->setType(idgs::pb::AT_STATELESS);

  // in operation
  idgs::actor::ActorOperationDescriporWrapper op;
  op.setName(RDD_OP_SAVE_MESSAGE);
  op.setDescription("save rdd partition data");
  op.setPayloadType("idgs.rdd.pb.SaveActionInterResult");
  descriptor->setInOperation(op.getName(), op);
  return descriptor;
}

const idgs::actor::ActorMessageHandlerMap& SaveActionActor::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {RDD_OP_SAVE_MESSAGE, static_cast<idgs::actor::ActorMessageHandler>(&SaveActionActor::handleSave)}
  };
  return handlerMap;
}

void SaveActionActor::handleSave(const idgs::actor::ActorMessagePtr& msg) {
}

SaveActionActor::~SaveActionActor() {

}

} /// end namespace actor
} /// end namespace action
} /// end namespace rdd
} /// end namespace idgs
