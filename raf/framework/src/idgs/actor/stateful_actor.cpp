
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/application.h"

namespace idgs {
namespace actor {

StatefulActor::StatefulActor() {
  state.store(IDLE);
  ActorManager* af = idgs_application()->getRpcFramework()->getActorManager();
  actorId = (af->generateActorId(this));
}

StatefulActor::~StatefulActor() {
  state.store(TERMINATE);
  // clear actor inner message queue
  msg_queue.clear();
}

void StatefulActor::onDestroy() {
  DVLOG(2) << "Actor " << this->getActorId() << " get message " << OP_DESTROY;
  state.store(TERMINATE);
  msg_queue.clear();

  // unregister
  ActorManager* af = idgs_application()->getRpcFramework()->getActorManager();
  af->unregisterSessionActor(this->getActorId());

  // free memory
  delete this;
}


void StatefulActor::putMessage(const ActorMessagePtr& msg) {
  if (state.load() == TERMINATE) {
    LOG(WARNING) << "Actor is already terminated.";
    return;
  }
  msg_queue.push(msg);
}

bool StatefulActor::popMessage(ActorMessagePtr* msg) {
  return msg_queue.try_pop(*msg);
}

bool StatefulActor::tryToEntry() {
  State expectedState = IDLE;
#if !defined(RETRY_LOCK_TIMES)
#define RETRY_LOCK_TIMES 5
#endif // !defined(RETRY_LOCK_TIMES)
  for (int i = 0; i < RETRY_LOCK_TIMES; ++i) {
    expectedState = IDLE;
    if (state.compare_exchange_strong(expectedState, ACTIVE)) {
      return true;
    }
  }
  return false;
}

void StatefulActor::leave() {
  State expectedState = ACTIVE;
  state.compare_exchange_strong(expectedState, IDLE);
}

unsigned int StatefulActor::getPendingMessageNumber() {
  return msg_queue.unsafe_size();
}

void StatefulActor::process(const ActorMessagePtr& msg) {
  parse(const_cast<ActorMessagePtr&>(msg));

  if (state.load() == TERMINATE) {
    DLOG(WARNING)<< "Can not send a message to Actor at TERMINATE state.";
    /// @todo may send error response back;
    return;
  }
  DVLOG(2) << "Send ActorMessage " << msg.get() << " at Actor state " << state.load();

  putMessage(msg);

  if (tryToEntry()) {
    DVLOG(2) << "Thread " << std::this_thread::get_id() << " get actor " << getActorId() << " lock";

    ActorMessagePtr msgPtr;
    if (state.load() == ACTIVE && popMessage(&msgPtr)) {
      do {
        auto status = innerProcess(msgPtr);
        if (status == ProcessStatus::TERMINATE) {
          return;
        }
      } while (state.load() == ACTIVE && popMessage(&msgPtr));
    }

    leave();
  }
}

} // namespace actor
} // namespace idgs

