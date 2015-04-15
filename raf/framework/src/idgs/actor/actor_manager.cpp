
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/actor/actor_manager.h"
#include "idgs/application.h"


using namespace std;
using namespace idgs::pb;

namespace idgs {
namespace actor {
ActorManager::ActorManager() {
  actor_id_generator.store(1);
}

ActorManager::~ActorManager() {
  function_footprint();
}

void ActorManager::destroy() {
  function_footprint();

  LOG(INFO) << "Remain session actors: " << sessionActorMap.size();
  for (auto it = sessionActorMap.begin(); it != sessionActorMap.end(); ++it) {
    if (it->second != NULL) {
      LOG(INFO) << "deleting session actor: " << it->second->getActorName();
      // @fixme enable the below line.
      // delete it->second;
    }
  }
  sessionActorMap.clear();

  LOG(INFO) << "Remain service actors: " << serviceActorMap.size();
  for (auto it = serviceActorMap.begin(); it != serviceActorMap.end(); ++it) {
    if (it->second != NULL) {
      // @fixme enable the below line.
//       LOG(INFO) << "Unregister service actor: " << it->second->getActorName();
    }
  }
  serviceActorMap.clear();
}

bool ActorManager::registerSessionActor(const string &actorId, Actor *actor) {
  return sessionActorMap.insert(make_pair(actorId, actor));
}

bool ActorManager::unregisterSessionActor(const string &actorId) {
  DVLOG(2) << "unregister session actor: " << actorId;
  return sessionActorMap.erase(actorId);
}

bool ActorManager::registerServiceActor(const string &actorId, Actor *actor) {
  return serviceActorMap.insert(make_pair(actorId, actor));
}

bool ActorManager::unregisterServiceActor(const string &id) {
  DVLOG(2) << "unregister service actor: " << id;
  return serviceActorMap.erase(id);
}

int ActorManager::sendMessage(ActorMessagePtr msg) const {
  function_footprint();
  DLOG_IF(FATAL, (msg->getDestMemberId() == UNKNOWN_MEMBER || msg->getSourceMemberId() == UNKNOWN_MEMBER)) << "Unknown member: " << msg->toString();
  DLOG_IF(FATAL, !msg->getRpcMessage()->has_operation_name()) << "Unknown operation: " << msg->toString();
  try {
    auto app = idgs_application();
    static auto network = app->getRpcFramework()->getNetwork();
    static const int32_t& localMemberId = app->getMemberManager()->getLocalMemberId();
    if (msg->getDestMemberId() == ANY_MEMBER) {
      msg->setDestMemberId(localMemberId);
    }
    if (msg->getDestMemberId() == localMemberId) {
//      idgs::actor::relayMessage(msg);
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

int ActorManager::postMessage(ActorMessagePtr msg) const {
  function_footprint();
  DLOG_IF(FATAL, (msg->getDestMemberId() == UNKNOWN_MEMBER || msg->getSourceMemberId() == UNKNOWN_MEMBER)) << "Unknown member: " << msg->toString();
  DLOG_IF(FATAL, !msg->getRpcMessage()->has_operation_name()) << "Unknown operation: " << msg->toString();
  try {
    auto app = idgs_application();
    static auto network = app->getRpcFramework()->getNetwork();
    static const int32_t& localMemberId = app->getMemberManager()->getLocalMemberId();
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

Actor* ActorManager::getActor(const std::string &actorId) {
  Actor* actor = NULL;
  if ((*(actorId.begin())) == '#') {
    // stateful
    ActorMap::const_accessor a;
    if (sessionActorMap.find(a, actorId)) {
      actor = a->second;
    }
  } else {
    // stateless
    ActorMap::const_accessor a;
    if (serviceActorMap.find(a, actorId)) {
      actor = a->second;
    }
  }
  return actor;
}

string ActorManager::generateActorId(Actor* actor) {
  u_int64_t id = actor_id_generator.fetch_add(1);

  stringstream stream;
  stream << '#' << std::hex << id;
  return stream.str();
}

idgs::actor::ActorMap& ActorManager::getSessionActors() {
  return sessionActorMap;
}

idgs::actor::ActorMap& ActorManager::getServiceActors() {
  return serviceActorMap;
}

std::string ActorManager::toString() {
  function_footprint();
  stringstream ss;

  if (!serviceActorMap.empty()) {
    ss << "============================== Named Actors ==============================" << endl;
    ss << "Name                          Class" << endl;
    ss << "--------------------------------------------------------------------------" << endl;
    for (auto itr = serviceActorMap.begin(); itr != serviceActorMap.end(); ++itr) {
      ss << itr->first << "\t\t" << idgs::util::demangle(typeid((*(itr->second))).name()) << endl;
    }
    ss << "==========================================================================" << endl;
  }

  if (!sessionActorMap.empty()) {
    map<string, uint32_t> actorCount;
    for (auto itr = sessionActorMap.begin(); itr != sessionActorMap.end(); ++itr) {
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

int sendMessage(ActorMessagePtr msg) {
  static auto af = idgs_application()->getRpcFramework()->getActorManager();
  return af->sendMessage(msg);
}

int postMessage(ActorMessagePtr msg) {
  static auto af = idgs_application()->getRpcFramework()->getActorManager();
  return af->postMessage(msg);
}

} // namespace rpc
} // namespace idgs

