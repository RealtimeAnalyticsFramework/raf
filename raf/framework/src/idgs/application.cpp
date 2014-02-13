
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include <dlfcn.h>

#include "idgs/application.h"
#include "idgs/actor/actor_message_queue.h"
#include "idgs/expr/expression_factory.h"


using namespace idgs::cluster;
using namespace idgs::actor;
namespace idgs {

  ModuleInfo::ModuleInfo():handle(NULL), module(NULL) {

  }

  ModuleInfo::~ModuleInfo() {
    if(handle) {
      // release the module.
      fn_release_idgs_module fn_release_module = (fn_release_idgs_module)dlsym(handle, "release_idgs_module");
      (*fn_release_module)(module);

      // @fixme enable the below line.
//      dlclose(handle);
      handle = NULL;
    }
  }

  Application::Application() : running(true) {
    google::InstallFailureSignalHandler();
  }

  Application::~Application(){
    function_footprint();
  }

  cluster::MembershipTableMgr* Application::getMemberManager() {
    ClusterFramework& cluster = ::idgs::util::singleton<ClusterFramework>::getInstance();
    return cluster.getMemberManager();
  }

  cluster::PartitionTableMgr* Application::getPartitionManager() {
    ClusterFramework& cluster = ::idgs::util::singleton<ClusterFramework>::getInstance();
    return cluster.getPartitionManager();
  }

  ActorFramework* Application::getActorframework() {
    RpcFramework& rpc = ::idgs::util::singleton<RpcFramework>::getInstance();
    return rpc.getActorFramework();
  }


  ResultCode Application::init(const std::string & config) {
    function_footprint();
    idgs::expr::ExpressionFactory::init();
    ResultCode rc;
    ClusterFramework& cluster = ::idgs::util::singleton<ClusterFramework>::getInstance();
    RpcFramework& rpc = ::idgs::util::singleton<RpcFramework>::getInstance();

/// load cluster config file.
    rc = cluster.loadCfgFile(config.c_str());
    CHECK_RC(rc);
    idgs::pb::ClusterConfig* cfg = cluster.getClusterConfig();
    if (cfg == NULL) {
      return RC_CLUSTER_ERR_PARSE_CONFIG_FILE;
    }

/// initialize network model
    rc = (ResultCode)rpc.getNetwork()->init(cfg);
    CHECK_RC(rc);

/// initialize all modules
    for (int i = 0; i < cfg->modules_size(); ++i ) {
      const ::idgs::pb::ModuleConfig& module = cfg->modules(i);
      LOG(INFO) << "Module: " << module.name() << ", so:" <<  module.module_path();
    }
    for (int i = 0; i < cfg->modules_size(); ++i ) {
      const ::idgs::pb::ModuleConfig& module = cfg->modules(i);
      if (module.enable()) {
        rc = loadModule(module.name(), module.module_path(), module.config_path());
        CHECK_RC(rc);
      }
    }
/// initialize thread model
    rpc.getThreadModel()->initialize(cluster.getClusterConfig()->thread_count());
    rc = rpc.initialize();
    CHECK_RC(rc);

/// initialize cluster service
    rc = cluster.init();
    CHECK_RC(rc);

    return RC_SUCCESS;
  }

  ResultCode Application::start() {
    function_footprint();
    ClusterFramework& cluster = ::idgs::util::singleton<ClusterFramework>::getInstance();
    RpcFramework& rpc = ::idgs::util::singleton<RpcFramework>::getInstance();
    ResultCode rc;

/// start all modules
    idgs::pb::ClusterConfig* cfg = cluster.getClusterConfig();
    if (cfg == NULL) {
      return RC_CLUSTER_ERR_PARSE_CONFIG_FILE;
    }
    for (auto& mod : modules) {
      rc = (idgs::ResultCode)(mod->module->start());
      CHECK_RC(rc);
    }

/// start scheduler service
    rc = rpc.getScheduler().start();
    CHECK_RC(rc);

/// start thread service
    ::idgs::util::singleton<idgs::actor::ActorMessageQueue>::getInstance().setMaxIdleCount(cfg->max_idle_thread());
    rc = (ResultCode)rpc.getThreadModel()->start();
    CHECK_RC(rc);

/// start network service
    rc = (ResultCode)rpc.getNetwork()->start();
    CHECK_RC(rc);

/// start cluster service
    rc = cluster.start();
    CHECK_RC(rc);

    running = true;
    return RC_SUCCESS;
  }

  bool Application::regsiterActor(StatefulActor* actor) {
    RpcFramework& rpc = ::idgs::util::singleton<RpcFramework>::getInstance();
    return rpc.getActorFramework()->Register(actor->getActorId(), actor);
  }

  bool Application::regsiterActor(StatelessActor* actor) {
    RpcFramework& rpc = ::idgs::util::singleton<RpcFramework>::getInstance();
    return rpc.getActorFramework()->Register(actor->getActorId(), actor);
  }

  ResultCode Application::stop() {
    function_footprint();
    // get references to singleton
    ClusterFramework& cluster = ::idgs::util::singleton<ClusterFramework>::getInstance();
    RpcFramework& rpc = ::idgs::util::singleton<RpcFramework>::getInstance();
    ResultCode rc;

    ::idgs::util::singleton<ActorMessageQueue>::getInstance().markShutdown();

    // stop network
    rpc.getNetwork()->shutdown();

    // stop actor thread pool
    (ResultCode)rpc.getThreadModel()->shutdown();

    // stop cluster thread
    cluster.getClusterAdapter()->stop();
    // stop scheduler thread
    ::idgs::util::singleton<RpcFramework>::getInstance().getScheduler().stop();


    // stop all services
    // start all modules
    idgs::pb::ClusterConfig*  clusterConfig = cluster.getClusterConfig();
    if (clusterConfig == NULL) {
      return RC_CLUSTER_ERR_PARSE_CONFIG_FILE;
    }

    for(auto it = modules.rbegin(); it != modules.rend(); ++it) {
      rc = (idgs::ResultCode)((*it)->module->stop());
      CHECK_RC(rc);

    }
    modules.clear();

    // destroy cluster
    cluster.destory();

    rpc.getActorFramework()->destroy();

    ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().clear();


    return RC_SUCCESS;
  }

  ResultCode Application::loadModule(const std::string& name, const std::string& exec, const std::string& configPath) {
    LOG(INFO)<< "Load module: " << name << ", path:" << exec << ", config:" << configPath;
    ResultCode rc;
    std::shared_ptr<ModuleInfo> info = std::make_shared<ModuleInfo>();

    info->handle  = dlopen(exec.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if(info->handle == NULL) {
      LOG(ERROR) << "Failed to open " << exec << ", error: " << dlerror();
      return RC_ERROR;
    }

    fn_get_idgs_module fn_get_module = (fn_get_idgs_module)dlsym(info->handle, "get_idgs_module");
    if (fn_get_module == NULL) {
      LOG(ERROR) << "Invalid module, no entry function " << "get_idgs_module" << ": " << exec;
      return RC_ERROR;
    }

    fn_release_idgs_module fn_release_module = (fn_release_idgs_module)dlsym(info->handle, "release_idgs_module");
    if (fn_release_module == NULL) {
      LOG(ERROR) << "Invalid module, no entry function " << "release_idgs_module" << ": " << exec;
      return RC_ERROR;
    }

    info->module = (*fn_get_module)();
    if (info->module == NULL) {
      LOG(ERROR) << "Invalid module, no callbacks " << exec;
      return RC_ERROR;
    }


    rc = static_cast<ResultCode>(info->module->init(configPath.c_str(), this));
    CHECK_RC(rc);

    modules.push_back(info);

    return RC_OK;
  }

  ResultCode Application::undeployModule(const std::string& name) {
    return RC_OK;
  }

} // namespace idgs
