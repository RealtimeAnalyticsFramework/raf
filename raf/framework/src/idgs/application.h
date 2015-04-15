/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include "idgs/idgs_module.h"

#include "idgs/actor/actor_message_queue.h"

#include "idgs/cluster/cluster_framework.h"

#include "idgs/util/singleton.h"

namespace idgs {

struct ModuleInfo {
  Module* module = NULL;
  void* handle = NULL;
  std::string name;

public:
  ModuleInfo();
  ~ModuleInfo();

  void release();

  ModuleInfo(const ModuleInfo&) = default;
  ModuleInfo(ModuleInfo&&) = default;
  ModuleInfo& operator =(const ModuleInfo&) = default;
  ModuleInfo& operator =(ModuleInfo&&) = default;

};

class Application {
private:
  Application();
  ~Application();

  Application(const Application&) = delete;
  Application(Application&&) = delete;
  Application& operator =(const Application&) = default;
  Application& operator =(Application&&) = default;

  friend class idgs::util::singleton<Application>;
public:
  ResultCode init(const std::string & config);
  bool registerSessionActor(idgs::actor::Actor* actor);
  bool registerServiceActor(idgs::actor::Actor* actor);

  ResultCode start();
  ResultCode stop();
  void release();

  void shutdown();

  bool isRunning() {
    return running;
  }

  cluster::ClusterFramework* getClusterFramework();
  cluster::MemberManagerActor* getMemberManager();
  cluster::PartitionManagerActor* getPartitionManager();
  idgs::actor::ActorManager* getActorframework();

  idgs::actor::RpcFramework* getRpcFramework();
  idgs::actor::ActorDescriptorMgr* getActorDescriptorMgr();
  idgs::actor::ActorMessageQueue* getActorMessageQueue();

  ///
  /// Multicast actor message
  /// @param actor_msg_ptr sent actor message
  ///
  idgs::ResultCode multicastMessage(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  ResultCode loadModule(const std::string& name, const std::string& exec, const std::string& configPath);
  ResultCode undeployModule(const std::string& name);

private:
  bool running;
  idgs::cluster::ClusterFramework* cluster;
  idgs::actor::RpcFramework* rpc;
  idgs::actor::ActorDescriptorMgr* actorDescriptorMgr;
  idgs::actor::ActorMessageQueue* messageQueue;
  std::vector<std::shared_ptr<ModuleInfo> > modules;

};

Application* idgs_application();

} // namespace idgs
