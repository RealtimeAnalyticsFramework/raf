
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once 

#include <tbb/concurrent_hash_map.h>

#include "idgs/actor/stateful_actor.h"
#include "idgs/actor/stateless_actor.h"

namespace idgs {
namespace actor {
  typedef tbb::concurrent_hash_map<std::string, StatefulActor*> StatefulActorMap;
  typedef tbb::concurrent_hash_map<std::string, StatelessActor*> StatelessActorMap;

class ActorFramework {
public:
  ActorFramework();
  ActorFramework(const ActorFramework&) = delete;
  ActorFramework(ActorFramework&&) = delete;
  ActorFramework& operator =(const ActorFramework&) = delete;
  ActorFramework& operator =(ActorFramework&&) = delete;
  ~ActorFramework();

  void destroy();

  // return true if register successfully.
  bool Register(const std::string &actorId, StatefulActor *actor);

  bool unRegisterStatefulActor(const std::string &actorId);

  // just for debug or admin monitor
  idgs::actor::StatefulActorMap& getStatefulActors();

  //return true if register successfully.
  bool Register(const std::string &actorId, StatelessActor *actor);

  bool unRegisterStatelessActor(const std::string &actorId);

  idgs::actor::StatelessActorMap& getStatelessActors();

  int sendMessage(ActorMessagePtr& msg) const;
  int postMessage(ActorMessagePtr& msg) const;

  std::string generateActorId(Actor* actor = NULL);

  Actor* getActor(const std::string &actorId);

  std::string toString();

private:
  StatefulActorMap statefulActorMap;
  StatelessActorMap statelessActorMap;
  tbb::atomic<u_int64_t> actorId;
};
// class ActorFramework
}// namespace rpc
} // namespace idgs
