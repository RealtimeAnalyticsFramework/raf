
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include "idgs/admin/admin_module.h"
#include "idgs/application.h"

#include "idgs/admin/actor/admin_service_actor.h"
#include "idgs/admin/admin_server.h"
#include "idgs/httpserver/http_server.h"

using namespace idgs::actor;
using namespace idgs::admin::actor;
using namespace idgs::admin;
using namespace google::protobuf;


namespace idgs {
namespace admin {
static const std::string MODULE_DESCRIPTOR = "ADMIN Service";
static const std::string ADMIN_SERVICE_ACTOR_ID = "admin_service";

AdminModule::~AdminModule() {

}

int AdminModule::init(const char* config_path, idgs::Application* theApp){
  LOG(INFO) << "init module admin";
  app = theApp;

  ::idgs::util::singleton<idgs::admin::AdminServer>::getInstance().init();

  idgs::admin::actor::AdminServiceActor* actor = new idgs::admin::actor::AdminServiceActor();
  actor->init();
  app->getActorframework()->Register(actor->getActorId(), actor);

  shared_ptr<ModuleDescriptorWrapper> module_descriptor(new ModuleDescriptorWrapper);
  module_descriptor->setName("admin");
  module_descriptor->setDescription(MODULE_DESCRIPTOR);
  module_descriptor->addActorDescriptor(AdminServiceActor::generateActorDescriptor());
  ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().registerModuleDescriptor(module_descriptor->getName(), module_descriptor);

  idgs::http::server::HttpServer* httpServer =
      idgs::util::singleton<idgs::actor::RpcFramework>::getInstance().getNetwork()->getHttpServer();
  idgs::http::server::HttpAsyncServletGenFunc f = [](){
    idgs::http::server::HttpAsyncServletPtr admin_ptr(new AdminAsyncServlet);
    return admin_ptr;
  };
  httpServer->registerHttpAsyncServlet(
      ADMIN_SERVICE_ACTOR_ID, f
  );

  return RC_OK;
}

int AdminModule::start(void){
  LOG(INFO) << "start module ADMIN";

  return RC_OK;
}

int AdminModule::stop(void){
  LOG(INFO) << "stop module ADMIN";

  // remove actor descriptor
  ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().unRegisterModuleDescriptor(MODULE_DESCRIPTOR);

  // unregister actor
  app->getActorframework()->unRegisterStatelessActor(ADMIN_SERVICE_ACTOR_ID);

  return RC_OK;
}


static AdminModule* module = NULL;

/// entry point of this module
AdminModule*  idgs_admin_module(void) {
  if(!module) {
    module = new AdminModule();
  }
  return module;
}

void  release_admin_module(AdminModule* mod) {
  if(!mod) {
    if(mod == module) {
      module = NULL;
    }
    delete mod;
  }
}

} // namespace rdd
} // namespace idgs

/// entry point of this module
idgs::Module*  get_idgs_module(void) {
  return idgs_admin_module();
}
void  release_idgs_module(idgs::Module* mod) {
  release_admin_module(dynamic_cast<idgs::admin::AdminModule*>(mod));
}




