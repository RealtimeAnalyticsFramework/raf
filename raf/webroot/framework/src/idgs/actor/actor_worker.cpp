
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/actor/actor_worker.h"

#include "idgs/application.h"


using namespace std;
using namespace idgs::pb;

namespace idgs {
namespace actor {
void ActorWorker::start() {
  if (m_pthread == NULL) {
//        DVLOG(2)<<"Thread " << id << " is created ";
    m_pthread = new std::thread(std::bind(&ActorWorker::run, this));
  }
}

void ActorWorker::terminateAndJoin() {
  m_is_finish.store(true);
  this->join();
}

void ActorWorker::join() {
  if (m_pthread != NULL) {
    DVLOG(2) << "Thread " << id << " is joining ";
    idgs_application()->getActorMessageQueue()->notifyAll();
    m_pthread->join();
    delete m_pthread;
    m_pthread = NULL;
  }
}

void ActorWorker::processTcpOrietatedMessage(std::shared_ptr<ActorMessage>& msgPtr) {
  DVLOG(7) << "buffer size: " << msgPtr->getRpcBuffer()->getBodyLength() << ", content: "
              << dumpBinaryBuffer(std::string(msgPtr->getRpcBuffer()->byteBuffer()->data(), msgPtr->getRpcBuffer()->getBodyLength()));

  auto ret = msgPtr->parseRpcBuffer();
  if (ret != idgs::RC_OK) {
    LOG(ERROR) << "Failed to parse message";
    return;
  }
  RpcMessage* newmsg = msgPtr->getRpcMessage().get();
  DLOG_IF(ERROR, newmsg->dest_actor().actor_id().empty()) << "Destination actor ID is empty: " << newmsg->DebugString();
  DLOG_IF(ERROR, newmsg->source_actor().actor_id().empty()) << "Source actor ID is empty: " << newmsg->DebugString();

  static const int32_t& localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();

  newmsg->mutable_client_actor()->set_actor_id(newmsg->source_actor().actor_id());
  newmsg->mutable_client_actor()->set_member_id(newmsg->source_actor().member_id());

  newmsg->mutable_source_actor()->set_actor_id(msgPtr->getTcpActorId());
  newmsg->mutable_source_actor()->set_member_id(localMemberId);

  /// destination is other member
  int32_t destMemberId = newmsg->dest_actor().member_id();

  if (destMemberId != localMemberId && destMemberId != ANY_MEMBER && destMemberId != ALL_MEMBERS) {
    static auto network = idgs_application()->getRpcFramework()->getNetwork();
    network->send(msgPtr);
    return;
  }

  localProcess(msgPtr);
}

void ActorWorker::processMessage(idgs::actor::ActorMessagePtr& msgPtr) {
  switch (msgPtr->getMessageOrietation()) {
  case ActorMessage::UDP_ORIENTED:
  case ActorMessage::INNER_TCP:
    if (msgPtr->parseRpcBuffer() != idgs::RC_OK) {
      LOG(ERROR) << "Failed to parse message";
      return;
    }
    localProcess(msgPtr);
    break;
  case ActorMessage::APP_ORIENTED:
    localProcess(msgPtr);
    break;
  case ActorMessage::OUTER_TCP:
    processTcpOrietatedMessage(msgPtr);
    break;
  default:
    LOG(ERROR)<< "unknown message orientation.";
  }
}

void ActorWorker::run() {
  set_thread_name("idgs_worker");

  idgs::actor::ActorMessageQueue* actor_queue = idgs_application()->getActorMessageQueue();
  std::shared_ptr<ActorMessage> msgPtr;
  while (!m_is_finish.load()) {
    try {
      if (actor_queue->try_pop(&msgPtr)) {
        DVLOG(7) << "Thread " << id << " Got message " << msgPtr->getMessageOrietation() << ", content:" << msgPtr->toString();
        processMessage(msgPtr);
        msgPtr.reset();
      } else {
        actor_queue->wait();
      }
    } catch (std::exception &e) {
      LOG(ERROR)<< "Got exception in worker thread, cause: " << e.what();
      dump_exception_callstack();
    } catch (...) {
      catchUnknownException();
    }
  }

  DVLOG(1) << "thread " << getId() << " exit.";
}

void ActorWorker::localProcess(std::shared_ptr<ActorMessage>& actorMsg) {
  DLOG_IF(ERROR, (actorMsg->getDestMemberId() == UNKNOWN_MEMBER || actorMsg->getSourceMemberId() == UNKNOWN_MEMBER))
    << "Unknown member: " << actorMsg->toString();

  if (actorMsg->getDestMemberId() == UNKNOWN_MEMBER || actorMsg->getSourceMemberId() == UNKNOWN_MEMBER) {
    return;
  }

  /// destination is local member.
  const string destActorId = actorMsg->getDestActorId();
  static ActorManager* af = idgs_application()->getRpcFramework()->getActorManager();
  Actor* actor = af->getActor(destActorId);
  if (actor != NULL) {
    try {
      actor->process(actorMsg);
    } catch (std::exception& e) {
      LOG(ERROR)<< actor->getDescriptor()->getName() << ", error" << e.what();
      dump_exception_callstack();
    } catch (...) {
      LOG(ERROR) << actor->getDescriptor()->getName() << ", error";
      catchUnknownException();
      dump_exception_callstack();
    }
  } else {
//    LOG(ERROR) << "Can not find actor " << destActorId << " for message " << actorMsg->toString();
    LOG(ERROR) << "Can not find actor " << destActorId;
  }
}

} // namespace actor
} // namespace idgs
