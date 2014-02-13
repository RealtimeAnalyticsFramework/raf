
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <thread>
#include "idgs/actor/stateful_actor.h"

namespace idgs {
namespace actor {
class ActorWorker {
public:

  ActorWorker(int id_) :
      id(id_), m_pthread(NULL) {
    m_is_finish.store(false);
  }
  ActorWorker() :
      id(-1), m_pthread(NULL) {
    m_is_finish.store(false);
  }

  ~ActorWorker() {
    terminateAndJoin();
  }

  ActorWorker(const ActorWorker& thread) {
    // function_footprint();
    if (this == &thread) {
      return;
    }
    this->id = thread.id;
    this->m_is_finish = thread.m_is_finish;
    this->m_pthread = thread.m_pthread;
  }
  ActorWorker(ActorWorker&& thread) {
    // function_footprint();
    this->id = thread.id;
    this->m_is_finish = std::move(thread.m_is_finish);
    this->m_pthread = std::move(thread.m_pthread);
    thread.m_pthread = NULL;
  }

  void start();
  void terminateAndJoin();
  void join();

  inline int getId() const {
    return id;
  }

  inline void setId(int id) {
    this->id = id;
  }

  bool operator==(const ActorWorker& thread) {
    return this->id == thread.id && this->m_is_finish == thread.m_is_finish && this->m_pthread == thread.m_pthread;
  }

  void markFinish() {
    m_is_finish.store(true);
  }

  static void processMessage(idgs::actor::ActorMessagePtr& msg);

private:
  int id;
  tbb::atomic<bool> m_is_finish;
  std::thread *m_pthread;
  void run();
  static void localProcess(std::shared_ptr<ActorMessage>& actorMsg);
  static void processTcpOrietatedMessage(std::shared_ptr<ActorMessage>& actorMsg);
};
} // namespace actor
} // namespace idgs
