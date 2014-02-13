
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/actor/rpc_framework.h"


namespace idgs {
namespace actor {

StatefulActor::StatefulActor() {
  _m_state.store(IDLE);
  ActorFramework* af = ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework();
  actorId = (af->generateActorId(this));
}

StatefulActor::~StatefulActor() {
}

void StatefulActor::onDestroy() {
  DVLOG(2) << "Actor " << this->getActorId() << " get message " << OP_DESTROY;
  ActorFramework* af = ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework();
  // unregister
  af->unRegisterStatefulActor(this->getActorId());

  // clear actor inner message queue
  queue.clear();


  // free memory
  delete this;
}


void StatefulActor::putMessage(ActorMessagePtr msg) {
  queue.push(msg);
}

bool StatefulActor::popMessage(ActorMessagePtr* msg) {
  return queue.try_pop(*msg);
}

bool StatefulActor::tryToEntry() {
  return _m_state.compare_and_swap(ACTIVE, IDLE) == IDLE;
}

bool StatefulActor::leave() {
  return _m_state.compare_and_swap(IDLE, ACTIVE) == ACTIVE;
}

unsigned int StatefulActor::getPendingMessageNumber() {
  return queue.unsafe_size();
}

void StatefulActor::setStatus(const State state) {
  _m_state.store(state);
}

StatefulActor::State StatefulActor::getStatus() const {
  return _m_state.load();
}

void StatefulActor::wait() {
  _m_state.store(WAITING);
}

/// @todo may send response to all the pending message in the queue
void StatefulActor::terminate() {
  _m_state.store(TERMINATE);

}

bool StatefulActor::resume() {
  return _m_state.compare_and_swap(IDLE, WAITING) == WAITING;
}

bool StatefulActor::isRunning() {
  return _m_state.load() == ACTIVE;
}

bool StatefulActor::isWaiting() {
  return _m_state.load() == WAITING;
}

bool StatefulActor::isTerminated() {
  return _m_state.load() == TERMINATE;
}

void StatefulActor::process(const ActorMessagePtr& msg) {
  parse(const_cast<ActorMessagePtr&>(msg));

  if (handleSystemOperation(msg)) {
    DVLOG(1) << "received a system message.";
    return;
  }

  DVLOG(2) << "Send ActorMessage " << msg.get() << " to Actor " << getActorId() << " at Actor state " << getStatus();

  if (isTerminated()) {
    DLOG(WARNING)<< "Can not send a message " << msg.get() << " to Actor " << getActorId() << " at TERMINATE state ";
    /// @todo may send error response back;
    return;
  }

  if (isWaiting()) {
    if (resume()) {
      DVLOG(2) << "Send a message " << msg.get() << " to Actor " << getActorId() << " at Waiting state ";
      //StatefulActor* tempActorPtr = NULL;
    }
  }

  putMessage(msg);

  if (tryToEntry()) {
    DVLOG(2) << "Thread " << std::this_thread::get_id() << " get actor " << getActorId() << " lock";

    ActorMessagePtr msgPtr;
    if (popMessage(&msgPtr)) {
      do {
        innerProcess(msgPtr);
      } while (isRunning() && popMessage(&msgPtr));

      //Check Again
      if (popMessage(&msgPtr)) {
        do {
          innerProcess(msgPtr);
        } while (isRunning() && popMessage(&msgPtr));
      }
    }

    if (!leave()) {
      DLOG_IF(WARNING, _m_state != TERMINATE) << "Thread " << std::this_thread::get_id() << " leave the Actor "
                                                 << getActorId() << " not in Active State: " << _m_state;
    }
  }
}

} // namespace actor
} // namespace idgs

