
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/store/store_module.h"

#include "idgs/application.h"
#include "idgs/store/datastore_listener.h"
#include "idgs/store/datastore_const.h"
#include "idgs/store/data_store.h"

using namespace std;


namespace idgs {
namespace store {


StoreModule::~StoreModule() {

}

int StoreModule::init(const char* config_path, idgs::Application* theApp) {
  function_footprint();
  app = theApp;

  // parse configuration file
  DataStore& store = ::idgs::util::singleton<DataStore>::getInstance();
  string configPath(config_path);
  ResultCode rc;
  rc = store.initialize(configPath);
  CHECK_RC(rc);

  partChangeListener = new PartitionChangeListener;
  theApp->getPartitionManager()->addListener(partChangeListener);
  memberJoinedListener = new MemberJoinedListener;
  theApp->getMemberManager()->addListener(memberJoinedListener);

  return RC_OK;
}

int StoreModule::start(void) {
  function_footprint();
  LOG(INFO)<< "start module store";

  ResultCode rc;

  DataStore& store = ::idgs::util::singleton<DataStore>::getInstance();
  rc = store.start();
  CHECK_RC(rc);

  return RC_OK;
}

int StoreModule::stop(void) {
  function_footprint();
  LOG(INFO)<< "stop module store";

  DataStore& store = ::idgs::util::singleton<DataStore>::getInstance();
  store.stop();

  if (app == NULL) {
    LOG(WARNING)<< "Not found application, module clear not done.";
    return RC_OK;
  }


  // @todo unregister module descriptor
  // remove actor descriptor
//  ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().removeActorDescriptor(ACTORID_STORE_SERVCIE);
//  ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().removeActorDescriptor(DATA_STORE_SYNC_ACTOR);
//  ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().removeActorDescriptor(DATA_AGGREGATOR_ACTOR);
//  ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().removeActorDescriptor(DATA_SIZE_AGGREGATOR_ACTOR);
//  ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().removeActorDescriptor(DATA_REPLICATED_STORE_SYNC_ACTOR);

  // unregister stateless actor
  app->getActorframework()->unRegisterStatelessActor(ACTORID_STORE_SERVCIE);
  app->getActorframework()->unRegisterStatelessActor(DATA_STORE_SYNC_ACTOR);

  // unregister listener
  app->getPartitionManager()->removeListener(partChangeListener);
  app->getMemberManager()->removeListener(memberJoinedListener);

  delete partChangeListener;
  delete memberJoinedListener;

  return RC_OK;
}

static idgs::store::StoreModule* module = NULL;
/// entry point of this module
StoreModule* idgs_store_module(void) {
  if(!module) {
    module = new StoreModule();
  }
  return module;
}
void  release_store_module(StoreModule* mod) {
  if(!mod) {
    if(mod == module) {
      module = NULL;
    }
    delete mod;
  }
}

} // namespace store
} // namespace idgs

/// entry point of this module
idgs::Module* get_idgs_module(void) {
  return idgs::store::idgs_store_module();
}

void release_idgs_module(idgs::Module* mod) {
  idgs::store::release_store_module(dynamic_cast<idgs::store::StoreModule*>(mod));
}
