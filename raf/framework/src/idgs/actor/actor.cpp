
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/actor/actor.h"

#include "idgs/cluster/cluster_framework.h"
#include "protobuf/message_helper.h"
#include "idgs/actor/actor_message_queue.h"
#include "idgs/util/backtrace.h"

namespace idgs {
namespace actor {

//
// System operations.
//
const std::string OP_DESTROY = "DESTROY";
const std::string OP_ACTOR_NOT_FOUND = "ACTOR_NOT_FOUND";

std::shared_ptr<idgs::actor::ActorMessage> Actor::createActorMessage() const {
  std::shared_ptr<idgs::actor::ActorMessage> msg = std::make_shared<idgs::actor::ActorMessage>();
  msg->setSourceActorId(getActorId());
  int32_t localMemberId =
      ::idgs::util::singleton<idgs::cluster::ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();
  msg->setSourceMemberId(localMemberId);
  return msg;
}

ActorMessagePtr Actor::createActorMessage(const std::string& actorId) {
  ActorMessagePtr msg = std::make_shared<idgs::actor::ActorMessage>();
  msg->setSourceActorId(actorId);
  int32_t localMemberId =
      ::idgs::util::singleton<idgs::cluster::ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();
  msg->setSourceMemberId(localMemberId);
  return msg;
}

bool Actor::parse(ActorMessagePtr& msg) {
  try {
    if (getDescriptor()) {
      if (msg->getPayload().get()) {
        DVLOG(5) << "payload: " << msg->getPayload()->DebugString();
      } else {
        if (msg->getRpcMessage()->has_payload()) {
          // const idgs::actor::ActorDescriptorPtr& ad = getDescriptor();
          static idgs::actor::ActorDescriptorMgr& adm = idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance();
          const ActorOperationDescriporWrapper* op;
          const idgs::actor::ActorDescriptorPtr& ad = adm.getActorDescriptor(getActorName());
          if(ad) {
            op = ad->getResolvedInOperation(msg->getRpcMessage()->operation_name());
          } else {
            op = getDescriptor()->getResolvedInOperation(msg->getRpcMessage()->operation_name());
          }
          if (op) {
            auto payload = ::idgs::util::singleton<protobuf::MessageHelper>::getInstance().createMessage(
                op->getPayloadType());
            if (!payload.get()) {
              LOG(ERROR)<< "create paylaod type: " << op->getPayloadType() << " error, pls. check actor descriptor's in operation: " << op << "'s payload type is right?";
            }
            protobuf::ProtoSerdesHelper::deserialize(
                static_cast<protobuf::SerdesMode>(msg->getRpcMessage()->serdes_type()), msg->getRpcMessage()->payload(),
                payload.get());
            msg->setPayload(payload);
          } else {
            LOG(WARNING) << "No actor operation descriptor for actor: " << getActorName() << ", implementation class: "<< idgs::util::demangle(typeid(*this).name()) << ", operation: " << msg->getRpcMessage()->operation_name();
            LOG(WARNING) << getDescriptor()->toString();
          }
        } else {
          DLOG(WARNING) << "No payload for actor: " << idgs::util::demangle(typeid(*this).name()) << " operation: " << msg->getRpcMessage()->operation_name();
        }
      }
    } else {
      LOG_IF(WARNING, msg->getRpcMessage()->has_payload()) << "No actor descriptor for actor: " << idgs::util::demangle(typeid(*this).name());
    }
  } catch (std::exception& e) {
    LOG(WARNING) << "Exception: " << e.what();
  } catch (...) {
    catchUnknownException();
  }
  return true;
}

/// the business logic to handle the message
/// @param msg reference to the message
void Actor::process(const ActorMessagePtr& msg) {
  parse(const_cast<ActorMessagePtr&>(msg));
  if (handleSystemOperation(msg)) {
    DVLOG(1) << "received a system message.";
    return;
  }
  innerProcess(msg);
}

void Actor::innerProcess(const ActorMessagePtr& msg) {
  function_footprint();
  if (!msg) {
    DVLOG(1) << "received a null message.";
    return;
  }

  try {
    // process the message
    const std::string& opName = msg->getOperationName();

    const auto& handlerMap = getMessageHandlerMap();
    auto it = handlerMap.find(opName);
    if(likely(it != handlerMap.end())) {
      auto handler = it->second;
      // LOG(INFO) << "handler: " << reinterpret_cast<void*>(handler);
      (this->*handler)(msg);
      return;
    } else {
      it = handlerMap.find("*");
      if(likely(it != handlerMap.end())) {
        auto handler = it->second;
        // LOG(INFO) << "handler: " << reinterpret_cast<void*>(handler);
        (this->*handler)(msg);
        return;
      } else {
        // @todo should be LOG(ERROR)
        DVLOG(10)<< "Unknown operations: " << opName;
      }
    }
  } catch (std::exception& e) {
    LOG(ERROR) << "Exception: " << e.what();
  } catch (...) {
    catchUnknownException();
  }
}

bool Actor::handleSystemOperation(const ActorMessagePtr& msg) {
  if (msg->getRpcMessage()->operation_name() == OP_DESTROY) {
    onDestroy();
    return true;
  }
  return false;
}

void Actor::terminate() {
  VLOG(2) << "Terminate actor: " << getActorName() << "(" << getActorId() << ")";

  // socket_.close();
  ActorMessagePtr msg = createActorMessage();
  msg->setDestMemberId(msg->getSourceMemberId());
  msg->setDestActorId(msg->getSourceActorId());
  msg->setOperationName(idgs::actor::OP_DESTROY);

  DVLOG(5) << "Destroy TCP actor message: " << msg->toString();

  idgs::actor::relayMessage(msg);

}

} // namespace rpc
} // namespace idgs

