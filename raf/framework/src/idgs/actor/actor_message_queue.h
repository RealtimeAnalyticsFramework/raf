
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <condition_variable>
#include <atomic>

#include "idgs/actor/actor_worker.h"
#include "idgs/util/singleton.h"

namespace idgs {
namespace actor {
void relayMessage(ActorMessagePtr& msg);
void relayTimerMessage(ActorMessagePtr& msg);

class ActorMessageQueue {
public:
  friend void relayMessage(ActorMessagePtr& msg);
  friend void relayTimerMessage(ActorMessagePtr& msg);

  ActorMessageQueue() :
      shutdown(false), maxIdleCount(0) {
    idleCount.store(0);
  }

  ~ActorMessageQueue() {
    function_footprint();
  }

  bool try_pop(ActorMessagePtr* result) {
    return queue.try_pop(*result);
  }

  void wait() {
    std::unique_lock < std::mutex > lock(mutex);
    idleCount.fetch_add(1);
    if (!shutdown) {
      cond_var.wait_for(lock, std::chrono::milliseconds(1000));
    }
    idleCount.fetch_sub(1);
  }
  void notify() {
    cond_var.notify_one();
  }
  void notifyAll() {
    cond_var.notify_all();
  }

  void markShutdown() {
    shutdown = true;
  }

  void setMaxIdleCount(int32_t c) {
    maxIdleCount = c;
  }

  int32_t getMaxIdleCount() const {
    return maxIdleCount;
  }

  int32_t getIdleThread() const {
    return idleCount.load();
  }

  int32_t getPendingMessages() const {
    return queue.unsafe_size();
  }

protected:
  void push(const ActorMessagePtr& source) {
    queue.push(source);
    if (idleCount.load() > maxIdleCount) {
      notify();
    }
  }

private:
  bool shutdown;
  tbb::concurrent_queue<ActorMessagePtr> queue;
  std::mutex mutex;
  std::condition_variable cond_var;

  std::atomic<int32_t> idleCount;
  int32_t maxIdleCount;

};
// class ActorMessageQueue

/// deliver traffic message
inline void relayMessage(ActorMessagePtr& msg) {
  static ActorMessageQueue& q = ::idgs::util::singleton<ActorMessageQueue>::getInstance();
  q.push(msg);
}

/// deliver timer message
inline void relayTimerMessage(ActorMessagePtr& msg) {
  static ActorMessageQueue& q = ::idgs::util::singleton<ActorMessageQueue>::getInstance();
  q.push(msg);
}

} // namespace actor
} // namespace idgs
