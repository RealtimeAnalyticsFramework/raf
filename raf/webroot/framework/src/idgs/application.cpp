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
#include "idgs/expr/expression_factory.h"

using namespace idgs::cluster;
using namespace idgs::actor;

namespace idgs {

ModuleInfo::ModuleInfo() :
    module(NULL), handle(NULL) {

}

ModuleInfo::~ModuleInfo() {
  if (handle) {
    release();

    LOG(INFO) << "unload dynamic module: " << name;
    dlclose(handle);
    handle = NULL;
  }
}

void ModuleInfo::release() {
  if (module) {
    // release the module.
    LOG(INFO) << "Release module: " << name;
    fn_release_idgs_module fn_release_module = (fn_release_idgs_module) dlsym(handle, "release_idgs_module");
    (*fn_release_module)(module);
    module = NULL;
  }
}


Application::Application() :
    running(true), cluster(NULL), rpc(NULL), actorDescriptorMgr(NULL), messageQueue(NULL) {
  google::InstallFailureSignalHandler();
}

Application::~Application() {
  function_footprint();
  release();
}

void Application::release() {
  function_footprint();
  if (cluster) {
    delete cluster;
    cluster = NULL;
  }

  if (rpc) {
    delete rpc;
    rpc = NULL;
  }

  if (actorDescriptorMgr) {
    delete actorDescriptorMgr;
    actorDescriptorMgr = NULL;
  }

  if (messageQueue) {
    delete messageQueue;
    messageQueue = NULL;
  }
}

void Application::shutdown() {
  running = false;
}


ClusterFramework* Application::getClusterFramework() {
  if (!cluster) {
    cluster = new ClusterFramework;
  }
  return cluster;
}

cluster::MemberManagerActor* Application::getMemberManager() {
  return getClusterFramework()->getMemberManager();
}

cluster::PartitionManagerActor* Application::getPartitionManager() {
  return getClusterFramework()->getPartitionManager();
}

ActorManager* Application::getActorframework() {
  return getRpcFramework()->getActorManager();
}

idgs::actor::RpcFramework* Application::getRpcFramework() {
  if (!rpc) {
    rpc = new RpcFramework;
  }
  return rpc;
}

idgs::actor::ActorDescriptorMgr* Application::getActorDescriptorMgr() {
  if (!actorDescriptorMgr) {
    actorDescriptorMgr = new ActorDescriptorMgr;
  }
  return actorDescriptorMgr;
}

idgs::actor::ActorMessageQueue* Application::getActorMessageQueue() {
  if (!messageQueue) {
    messageQueue = new ActorMessageQueue;
  }
  return messageQueue;
}

///
/// Multicast actor message
/// @param actor_msg_ptr sent actor message
///
idgs::ResultCode Application::multicastMessage(const std::shared_ptr<idgs::actor::ActorMessage>& msg) {
  return cluster->getClusterAdapter()->multicastMessage(msg);
}

ResultCode Application::init(const std::string & config) {
  function_footprint();

  idgs::expr::ExpressionFactory::init();
  ResultCode rc;

  auto cluster = getClusterFramework();
  auto rpc = getRpcFramework();

  /// load cluster config file.
  rc = cluster->loadCfgFile(config.c_str());
  CHECK_RC(rc);
  idgs::pb::ClusterConfig* cfg = cluster->getClusterConfig();
  if (cfg == NULL) {
    return RC_CLUSTER_ERR_PARSE_CONFIG_FILE;
  }

  /// initialize network model
  rc = (ResultCode) rpc->getNetwork()->init(cfg);
  CHECK_RC(rc);

  getActorMessageQueue()->setMaxIdleCount(cfg->max_idle_thread());

  /// initialize all modules
  for (int i = 0; i < cfg->modules_size(); ++i) {
    const ::idgs::pb::ModuleConfig& module = cfg->modules(i);
    LOG(INFO)<< "Module: " << module.name() << ", so:" << module.module_path();
  }

  for (int i = 0; i < cfg->modules_size(); ++i) {
    const ::idgs::pb::ModuleConfig& module = cfg->modules(i);
    if (module.enable()) {
      rc = loadModule(module.name(), module.module_path(), module.config_path());
      CHECK_RC(rc);
    }
  }

  /// initialize thread model
  rpc->getThreadModel()->initialize(cluster->getClusterConfig()->thread_count());
  rc = rpc->initialize();
  CHECK_RC(rc);

  /// initialize cluster service
  rc = cluster->init();
  CHECK_RC(rc);

  /// register module descriptor
  std::shared_ptr<idgs::actor::ModuleDescriptorWrapper> module_descriptor = std::make_shared<idgs::actor::ModuleDescriptorWrapper>();
  module_descriptor->setName(CLUSTER_MODULE_DESCRIPTOR_NAME);
  module_descriptor->setDescription(CLUSTER_MODULE_DESCRIPTOR_DESCRIPTION);
  module_descriptor->addActorDescriptor(cluster->getMemberManager()->getDescriptor());
  module_descriptor->addActorDescriptor(cluster->getPartitionManager()->getDescriptor());
  /// register module descriptor
  getActorDescriptorMgr()->registerModuleDescriptor(module_descriptor->getName(), module_descriptor);

  return RC_SUCCESS;
}

ResultCode Application::start() {
  function_footprint();
  ResultCode rc;

  /// start all modules
  idgs::pb::ClusterConfig* cfg = cluster->getClusterConfig();
  if (cfg == NULL) {
    return RC_CLUSTER_ERR_PARSE_CONFIG_FILE;
  }

  for (auto& mod : modules) {
    rc = (idgs::ResultCode) (mod->module->start());
    CHECK_RC(rc);
  }

  /// start scheduler service
  rc = rpc->getScheduler().start();
  CHECK_RC(rc);

  /// start thread service
  rc = (ResultCode) rpc->getThreadModel()->start();
  CHECK_RC(rc);

  /// start network service
  rc = (ResultCode) rpc->getNetwork()->start();
  CHECK_RC(rc);

  /// start cluster service
  rc = cluster->start();
  CHECK_RC(rc);

  LOG(INFO) << "Member started: " << std::endl << cfg->DebugString();


  running = true;
  return RC_SUCCESS;
}

bool Application::registerSessionActor(Actor* actor) {
  return getActorframework()->registerSessionActor(actor->getActorId(), actor);
}

bool Application::registerServiceActor(Actor* actor) {
  return getActorframework()->registerServiceActor(actor->getActorId(), actor);
}

ResultCode Application::stop() {
  function_footprint();

  DVLOG(2) << rpc->getActorManager()->toString();

  // get references to singleton
  ResultCode rc;

  // stop all modules
  for (auto it = modules.rbegin(); it != modules.rend(); ++it) {
    rc = (idgs::ResultCode) ((*it)->module->stop());
    CHECK_RC(rc);
  }


  // stop network
  rpc->getNetwork()->shutdown();
  // stop cluster thread
  cluster->getClusterAdapter()->stop();

  // stop scheduler thread
  rpc->getScheduler().stop();

  // destroy cluster
  cluster->terminate();

  // stop actor thread pool
  messageQueue->close();
  // waiting for message queus is empty
  int pendingMessages;
  while ((pendingMessages = messageQueue->getPendingMessages()) > 0) {
    LOG(INFO) << "Pending messages: " << pendingMessages;
    std::chrono::milliseconds dura( 100 );
    std::this_thread::sleep_for( dura );
  }
  (ResultCode) rpc->getThreadModel()->shutdown();

  // clear actor and module
  rpc->getActorManager()->destroy();

  actorDescriptorMgr->unRegisterModuleDescriptor(CLUSTER_MODULE_DESCRIPTOR_NAME);
  actorDescriptorMgr->clear();

  for(auto& m: modules) {
    m->release();
  }

  release();
  google::protobuf::ShutdownProtobufLibrary();
  modules.clear();

  return RC_SUCCESS;
}

ResultCode Application::loadModule(const std::string& name, const std::string& exec, const std::string& configPath) {
  LOG(INFO)<< "Load module: " << name << ", path:" << exec << ", config:" << configPath;
  ResultCode rc;
  std::shared_ptr<ModuleInfo> info = std::make_shared<ModuleInfo>();

  info->name = name;
  info->handle = dlopen(exec.c_str(), RTLD_NOW | RTLD_GLOBAL);
  if(info->handle == NULL) {
    char* temp = getenv("IDGS_HOME");
    std::string idgsHome;
    if (temp) {
      idgsHome = temp;
    } else {
      idgsHome = "..";
    }
    std::string path = idgsHome + "/" + exec;
    info->handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);

    if(info->handle == NULL) {
      LOG(ERROR) << "Failed to open " << exec << ", error: " << dlerror();
      return RC_ERROR;
    }
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

Application* idgs_application() {
  return &idgs::util::singleton<Application>::getInstance();
}

} // namespace idgs
