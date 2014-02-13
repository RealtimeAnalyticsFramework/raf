
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/actor/actor.h"
#include <tbb/concurrent_queue.h>


namespace idgs {
namespace actor {

static std::vector<std::string> ACTOR_STATES_DSP =
     {"IDLE","ACTIVE","WAITING","TERMINATE"};

class StatefulActor: public Actor {
public:
  enum State {
    IDLE, ACTIVE, WAITING, TERMINATE
  };
  StatefulActor();
  virtual ~StatefulActor();

  void process(const ActorMessagePtr& msg) override;

//  const std::string& getActorName() const {
//    LOG(ERROR) << "empty actor name: " << idgs::util::demangle(typeid(*this).name());
//    static std::string name = "empty actor name";
//    return name;
//  }

  State getStatus() const;
  virtual void onDestroy() override;

  unsigned int getPendingMessageNumber();

protected:
  void putMessage(ActorMessagePtr msg);
  bool popMessage(ActorMessagePtr* msg);
  bool tryToEntry();
  bool leave();
  bool isRunning();
  bool isWaiting();
  bool isTerminated();
  void terminate();
  bool resume();

protected:
  tbb::concurrent_queue<ActorMessagePtr> queue;
  tbb::atomic<State> _m_state;
  ///tbb::atomic<int> _lock;
  void setStatus(const State state);
  void wait();
};

} // namespace actor
} // namespace idgs
