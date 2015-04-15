
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/store/store_module.h"


namespace idgs {
namespace store {

StoreModule::StoreModule() : app(NULL), datastore(NULL),
    partChangeListener(NULL), memberJoinedListener(NULL), backupStoreListener(NULL),
    storeActor(NULL), listenerManager(NULL), schemaActor(NULL),
    migrationTargetActor(NULL), migrationSourceActor(NULL),
    syncTargetActor(NULL), syncSourceActor(NULL) {
  datastore = new DataStore;
}

StoreModule::~StoreModule() {
  if (datastore) {
    datastore->stop();
    delete datastore;
    datastore = NULL;
  }

  if (storeActor) {
    delete storeActor;
    storeActor = NULL;
  }

  if (listenerManager) {
    delete listenerManager;
    listenerManager = NULL;
  }

  if (schemaActor) {
    delete schemaActor;
    schemaActor = NULL;
  }

  if (migrationTargetActor) {
    delete migrationTargetActor;
    migrationTargetActor = NULL;
  }

  if (migrationSourceActor) {
    delete migrationSourceActor;
    migrationSourceActor = NULL;
  }

  if (syncTargetActor) {
    delete syncTargetActor;
    syncTargetActor = NULL;
  }

  if (syncSourceActor) {
    delete syncSourceActor;
    syncSourceActor = NULL;
  }

  if (backupStoreListener) {
    delete backupStoreListener;
    backupStoreListener = NULL;
  }
}

int StoreModule::init(const char* config_path, idgs::Application* theApp) {
  function_footprint();
  app = theApp;

  // parse configuration file
  ResultCode rc;
  rc = datastore->initialize(config_path);
  CHECK_RC(rc);

  partChangeListener = new DataMigraionListener;
  theApp->getPartitionManager()->addListener(partChangeListener);
  memberJoinedListener = new DataSyncListener;
  theApp->getMemberManager()->addListener(memberJoinedListener);
  backupStoreListener = new BackupStoreListener;
  datastore->registerStoreListener(backupStoreListener);

  auto actorFramework = app->getActorframework();

  // register stateless actor and its descriptor
  storeActor = new StoreServiceActor(ACTORID_STORE_SERVCIE);
  actorFramework->registerServiceActor(ACTORID_STORE_SERVCIE, storeActor);

  listenerManager = new ListenerManager(LISTENER_MANAGER);
  listenerManager->setDataStore(datastore);
  actorFramework->registerServiceActor(LISTENER_MANAGER, listenerManager);

  schemaActor = new StoreSchemaActor(ACTORID_SCHEMA_SERVCIE);
  actorFramework->registerServiceActor(ACTORID_SCHEMA_SERVCIE, schemaActor);

  migrationTargetActor = new MigrationTargetActor;
  actorFramework->registerServiceActor(MIGRATION_TARGET_ACTOR, migrationTargetActor);

  migrationSourceActor = new MigrationSourceActor;
  actorFramework->registerServiceActor(MIGRATION_SOURCE_ACTOR, migrationSourceActor);

  syncTargetActor = new SyncTargetActor;
  actorFramework->registerServiceActor(SYNC_TARGET_ACTOR, syncTargetActor);

  syncSourceActor = new SyncSourceActor;
  actorFramework->registerServiceActor(SYNC_SOURCE_ACTOR, syncSourceActor);

  // register actor descriptor
  auto module_descriptor = std::make_shared<idgs::actor::ModuleDescriptorWrapper>();
  module_descriptor->setName(STORE_MODULE_DESCRIPTOR_NAME);
  module_descriptor->setDescription(STORE_MODULE_DESCRIPTOR_DESCRIPTION);
  module_descriptor->addActorDescriptor(DataAggregatorActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(StoreServiceActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(ListenerManager::generateActorDescriptor());
  module_descriptor->addActorDescriptor(StoreSchemaActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(StoreSchemaAggrActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(MigrationTargetActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(MigrationSourceActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(StoreMigrationTargetActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(StoreMigrationSourceActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(SyncTargetActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(SyncSourceActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(StoreSyncTargetActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(StoreSyncSourceActor::generateActorDescriptor());
  app->getActorDescriptorMgr()->registerModuleDescriptor(module_descriptor->getName(), module_descriptor);

  return RC_OK;
}

int StoreModule::start(void) {
  function_footprint();
  LOG(INFO)<< "start module store";

  ResultCode rc;

  rc = datastore->start();
  CHECK_RC(rc);

  return RC_OK;
}

int StoreModule::stop(void) {
  function_footprint();
  LOG(INFO)<< "stopping module store";

  if (app == NULL) {
    LOG(WARNING)<< "Not found application, module clear not done.";
    return RC_OK;
  }

  // unregister listener
  if (partChangeListener) {
    app->getPartitionManager()->removeListener(partChangeListener);
    delete partChangeListener;
    partChangeListener = NULL;
  }
  if (memberJoinedListener) {
    app->getMemberManager()->removeListener(memberJoinedListener);
    delete memberJoinedListener;
    memberJoinedListener = NULL;
  }

  if (storeActor) {
    storeActor->terminate();
  }

  if (listenerManager) {
    listenerManager->terminate();
  }

  if (schemaActor) {
    schemaActor->terminate();
  }

  if (migrationTargetActor) {
    migrationTargetActor->terminate();
  }

  if (migrationSourceActor) {
    migrationSourceActor->terminate();
  }

  if (syncTargetActor) {
    syncTargetActor->terminate();
  }

  if (syncSourceActor) {
    syncSourceActor->terminate();
  }

  if (backupStoreListener) {
    datastore->unregisterStoreListener(backupStoreListener);
  }

  app->getActorDescriptorMgr()->unRegisterModuleDescriptor(STORE_MODULE_DESCRIPTOR_NAME);

  app = NULL;

  LOG(INFO)<< "module store stopped";

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
