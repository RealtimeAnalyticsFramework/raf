
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/actor/actor_framework.h"

#include "idgs/actor/actor_message_queue.h"
#include "idgs/cluster/cluster_framework.h"
#include "idgs/util/backtrace.h"

using namespace std;
using namespace idgs::pb;
using namespace idgs::net;

namespace idgs {
namespace actor {
ActorFramework::ActorFramework() {
  actorId.store(1);
}

ActorFramework::~ActorFramework() {
  function_footprint();
}

void ActorFramework::destroy() {
  function_footprint();
  /// @todo
  // DVLOG(2) << endl << toString();

  for (auto it = statefulActorMap.begin(); it != statefulActorMap.end(); ++it) {
    if (it->second != NULL) {
      //delete it->second;
    }
  }
  statefulActorMap.clear();

  for (auto it = statelessActorMap.begin(); it != statelessActorMap.end(); ++it) {
    if (it->second != NULL) {
      // delete it->second;
    }
  }
  statelessActorMap.clear();
}

bool ActorFramework::Register(const string &actorId, StatefulActor *actor) {
  return statefulActorMap.insert(make_pair(actorId, actor));
}

bool ActorFramework::unRegisterStatefulActor(const string &actorId) {
  return statefulActorMap.erase(actorId);
}

bool ActorFramework::Register(const string &actorId, StatelessActor *actor) {
  return statelessActorMap.insert(make_pair(actorId, actor));
}

bool ActorFramework::unRegisterStatelessActor(const string &id) {
  return statelessActorMap.erase(id);
}

int ActorFramework::sendMessage(ActorMessagePtr& msg) const {
  function_footprint();
  DLOG_IF(FATAL, (msg->getDestMemberId() == UNKNOWN_MEMBER || msg->getSourceMemberId() == UNKNOWN_MEMBER))
                                                                                                              << "Unknown member: "
                                                                                                              << msg->toString();

  DVLOG(2) << "actor framework send actor message: " << msg->toString();
  try {
    static NetworkModel* network = ::idgs::util::singleton<RpcFramework>::getInstance().getNetwork();
    static int32_t localMemberId =
        ::idgs::util::singleton<idgs::cluster::ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();
    if (msg->getDestMemberId() == ANY_MEMBER) {
      msg->setDestMemberId(localMemberId);
    }
    if (msg->getDestMemberId() == localMemberId) {
      //          idgs::actor::relayMessage(msg);
      ActorWorker::processMessage(msg);
    } else {
      network->send(msg);
    }
  } catch (std::exception& e) {
    LOG(ERROR)<< "handle action error, " << e.what() << ", msg: " << msg->toString();
  } catch(...) {
    LOG(ERROR) << "handle action error, ";
    catchUnknownException();
  }
  return RC_SUCCESS;
}

int ActorFramework::postMessage(ActorMessagePtr& msg) const {
  function_footprint();
  DLOG_IF(FATAL, (msg->getDestMemberId() == UNKNOWN_MEMBER || msg->getSourceMemberId() == UNKNOWN_MEMBER))
                                                                                                              << "Unknown member: "
                                                                                                              << msg->toString();

  DVLOG(2) << "actor framework send actor message: " << msg->toString();
  try {
    static NetworkModel* network = ::idgs::util::singleton<RpcFramework>::getInstance().getNetwork();
    static int32_t localMemberId =
        ::idgs::util::singleton<idgs::cluster::ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();
    if (msg->getDestMemberId() == ANY_MEMBER) {
      msg->setDestMemberId(localMemberId);
    }
    if (msg->getDestMemberId() == localMemberId) {
      idgs::actor::relayMessage(msg);
    } else {
      network->send(msg);
    }
  } catch (std::exception& e) {
    LOG(ERROR)<< "handle action error, " << e.what() << ", msg: " << msg->toString();
  } catch(...) {
    LOG(ERROR) << "handle action error, ";
    catchUnknownException();
  }
  return RC_SUCCESS;
}

Actor* ActorFramework::getActor(const std::string &actorId) {
  Actor* actor = NULL;
  if ((*(actorId.begin())) == '@') {
    // stateful
    StatefulActorMap::const_accessor a;
    if (statefulActorMap.find(a, actorId)) {
      actor = a->second;
    }
  } else {
    // stateless
    StatelessActorMap::const_accessor a;
    if (statelessActorMap.find(a, actorId)) {
      actor = a->second;
    }
  }
  return actor;
}

string ActorFramework::generateActorId(Actor* actor) {
  u_int64_t id = actorId.fetch_and_add(1);

  stringstream stream;
  stream << '@' << id;
  return stream.str();
}

idgs::actor::StatefulActorMap& ActorFramework::getStatefulActors() {
  return statefulActorMap;
}

idgs::actor::StatelessActorMap& ActorFramework::getStatelessActors() {
  return statelessActorMap;
}

std::string ActorFramework::toString() {
  function_footprint();
  stringstream ss;

  if (!statelessActorMap.empty()) {
    ss << "============================== Named Actors ==============================" << endl;
    ss << "Name                          Class" << endl;
    ss << "--------------------------------------------------------------------------" << endl;
    for (auto itr = statelessActorMap.begin(); itr != statelessActorMap.end(); ++itr) {
      ss << itr->first << "\t\t" << idgs::util::demangle(typeid((*(itr->second))).name()) << endl;
    }
    ss << "==========================================================================" << endl;
  }

  if (!statefulActorMap.empty()) {
    map<string, uint32_t> actorCount;
    for (auto itr = statefulActorMap.begin(); itr != statefulActorMap.end(); ++itr) {
      actorCount[idgs::util::demangle(typeid((*(itr->second))).name())]++;
    }
    ss << "============================== Session Actors ============================" << endl;
    ss << "Count                         Class" << endl;
    ss << "--------------------------------------------------------------------------" << endl;
    for (auto itr = actorCount.begin(); itr != actorCount.end(); ++itr) {
      ss << itr->second << "\t\t\t" << itr->first << endl;
    }
    ss << "==========================================================================" << endl;
  }

  return ss.str();
}
} // namespace rpc
} // namespace idgs

