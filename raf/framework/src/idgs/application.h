
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#include "idgs/idgs_module.h"
#include "idgs/cluster/cluster_framework.h"


namespace idgs {

  struct ModuleInfo {
    Module* module = NULL;
    void* handle = NULL;

    ModuleInfo();
    ~ModuleInfo();
    ModuleInfo(const ModuleInfo&) = default;
    ModuleInfo(ModuleInfo&&) = default;
    ModuleInfo& operator =(const ModuleInfo&) = default;
    ModuleInfo& operator =(ModuleInfo&&) = default;

  };

  class Application {
  public:
    Application();
    ~Application();

    ResultCode init(const std::string & config);
    bool regsiterActor(idgs::actor::StatefulActor* actor);
    bool regsiterActor(idgs::actor::StatelessActor* actor);
    ResultCode start();
    ResultCode stop();

    void shutdown() {
      running = false;
    }
    bool isRunning() {
      return running;
    }

    // cluster::ClusterFramework& getClusterFramework();
    cluster::MembershipTableMgr* getMemberManager();
    cluster::PartitionTableMgr* getPartitionManager();
    idgs::actor::ActorFramework* getActorframework();

    ResultCode loadModule(const std::string& name, const std::string& exec, const std::string& configPath);
    ResultCode undeployModule(const std::string& name);

  private:
    bool running;
    std::vector<std::shared_ptr<ModuleInfo> > modules;

  };

} // namespace idgs
