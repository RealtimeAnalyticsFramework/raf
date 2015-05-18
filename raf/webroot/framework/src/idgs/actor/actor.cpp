
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/actor/actor.h"

#include "idgs/application.h"

#include "protobuf/message_helper.h"

namespace idgs {
namespace actor {

//
// System operations.
//
const char OP_DESTROY[] = "DESTROY";
const char OP_ACTOR_NOT_FOUND[] = "ACTOR_NOT_FOUND";

std::shared_ptr<idgs::actor::ActorMessage> Actor::createActorMessage() const {
  std::shared_ptr<idgs::actor::ActorMessage> msg = std::make_shared<idgs::actor::ActorMessage>();
  msg->setSourceActorId(getActorId());
  int32_t localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
  msg->setSourceMemberId(localMemberId);
  return msg;
}

ActorMessagePtr Actor::createActorMessage(const std::string& actorId) {
  ActorMessagePtr msg = std::make_shared<idgs::actor::ActorMessage>();
  msg->setSourceActorId(actorId);
  int32_t localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
  msg->setSourceMemberId(localMemberId);
  return msg;
}

bool Actor::parse(ActorMessagePtr& msg) {
  // DVLOG_FIRST_N(4, 20) << "Actor " << getActorName() << "(" << getActorId() << ") parse message.";
  try {
    if (getDescriptor()) {
      if (msg->getPayload().get()) {
        DVLOG(7) << "payload: " << msg->getPayload()->DebugString();
      } else {
        if (msg->getRpcMessage()->has_payload() && !msg->getRpcMessage()->payload().empty()) {
          const auto& handlerMap = getMessageHandlerMap();
          auto it = handlerMap.find(msg->getRpcMessage()->operation_name());
          if(likely(it != handlerMap.end())) {
            const google::protobuf::Message* payload_template = it->second.payload;
            if ( payload_template) {
              std::shared_ptr<google::protobuf::Message> payload(payload_template->New());
              protobuf::ProtoSerdesHelper::deserialize(
                  static_cast<protobuf::SerdesMode>(msg->getRpcMessage()->serdes_type()), msg->getRpcMessage()->payload(),
                  payload.get());
              msg->setPayload(payload);
              return true;
            } else {
              DVLOG(2) << "No operation prototype for actor: " << getActorName() << ", operation: " << msg->getRpcMessage()->operation_name();
            }
          } else {
            LOG(WARNING) << "No operation descriptor for actor: " << getActorName() << ", operation: " << msg->getRpcMessage()->operation_name();
          }

          ///
          /// @todo remove following block
          ///
          // const idgs::actor::ActorDescriptorPtr& ad = getDescriptor();
          static auto adm = idgs_application()->getActorDescriptorMgr();
          const ActorOperationDescriporWrapper* aod;
          const idgs::actor::ActorDescriptorPtr& ad = adm->getActorDescriptor(getActorName());
          if(ad) {
            aod = ad->getResolvedInOperation(msg->getRpcMessage()->operation_name());
          } else {
            aod = getDescriptor()->getResolvedInOperation(msg->getRpcMessage()->operation_name());
          }
          if (aod) {
            static protobuf::MessageHelper helper;
            auto payload = helper.createMessage(aod->getPayloadType());
            if (!payload.get()) {
              LOG(ERROR)<< "create paylaod type: " << aod->getPayloadType() << " error, pls. check actor descriptor's in operation: " << aod << "'s payload type is right?";
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
          // no payload
          // DLOG(WARNING) << "No payload for actor: " << idgs::util::demangle(typeid(*this).name()) << " operation: " << msg->getRpcMessage()->operation_name();
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
  innerProcess(msg);
}

ProcessStatus Actor::innerProcess(const ActorMessagePtr& msg) {
  function_footprint();
  if (!msg) {
    DVLOG(1) << "received a null message.";
    return ProcessStatus::DEFAULT;
  }

  if (handleSystemOperation(msg)) {
    DVLOG(1) << "received a system message.";
    return ProcessStatus::TERMINATE;
  }

  try {
    // process the message
    const std::string& opName = msg->getOperationName();

    const auto& handlerMap = getMessageHandlerMap();
    auto it = handlerMap.find(opName);
    if(likely(it != handlerMap.end())) {
      auto handler = it->second.handler;
      // LOG(INFO) << "handler: " << reinterpret_cast<void*>(handler);
      (this->*handler)(msg);
      return ProcessStatus::DEFAULT;
    } else {
      it = handlerMap.find("*");
      if(likely(it != handlerMap.end())) {
        auto handler = it->second.handler;
        // LOG(INFO) << "handler: " << reinterpret_cast<void*>(handler);
        (this->*handler)(msg);
        return ProcessStatus::DEFAULT;
      } else {
        LOG(ERROR) << "Unknown operations: " << opName << ", message: " << msg->toString();
      }
    }
  } catch (std::exception& e) {
    LOG(ERROR) << "Exception: " << e.what();
    dump_exception_callstack();
  } catch (...) {
    catchUnknownException();
    dump_exception_callstack();
  }

  return ProcessStatus::DEFAULT;
}

bool Actor::handleSystemOperation(const ActorMessagePtr& msg) {
  if (msg && msg->getRpcMessage() && msg->getRpcMessage()->operation_name() == OP_DESTROY) {
    onDestroy();
    return true;
  }
  return false;
}

void Actor::terminate() {
  terminate(getActorId());
}

void Actor::terminate(const std::string& actor_id, int member_id) {
  VLOG(2) << "Terminate actor: " << getActorName() << "(" << getActorId() << ")";

  ActorMessagePtr msg = createActorMessage();
  msg->setDestMemberId(member_id);
  msg->setDestActorId(actor_id);
  msg->setOperationName(idgs::actor::OP_DESTROY);

  DVLOG(5) << "Destroy actor message: " << msg->toString();

  idgs::actor::relayMessage(msg);

}

} // namespace rpc
} // namespace idgs

