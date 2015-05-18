
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/application.h"

#include "idgs/rdd/rdd_module.h"

#include "idgs/tpc/actor/line_crud_actor.h"
#include "idgs/tpc/actor/migration_verify_actor.h"
#include "idgs/tpc/actor/sync_verify_actor.h"
#include "idgs/tpc/transform/ssb_Q1_transformer.h"
#include "idgs/tpc/transform/tpch_Q6_transformer.h"
#include "idgs/tpc/action/ssb_Q1_action.h"
#include "idgs/tpc/action/tpch_Q6_action.h"
#include "idgs/tpc/action/partition_count_action.h"

using namespace idgs;
using namespace idgs::rdd;

using namespace idgs::actor;
using namespace idgs::tpc;
using namespace idgs::tpc::transform;
using namespace idgs::tpc::action;
using namespace idgs::tpc::actor;

namespace idgs {
namespace tpc {
struct TpcModule : public idgs::Module {
  virtual ~TpcModule();

  virtual int init(const char* config_path, idgs::Application* theApp);
  virtual int start();
  virtual int stop();
  idgs::Application* app;

private:
  LineCrudActor* lineCrudActor;
  MigrationVerifyActor* migrationVerifyActor;
  SyncVerifyActor* syncVerifyActor;
};

TpcModule::~TpcModule() {
  if (lineCrudActor) {
    delete lineCrudActor;
    lineCrudActor = NULL;
  }

  if (migrationVerifyActor) {
    migrationVerifyActor = NULL;
  }

  if (syncVerifyActor) {
    syncVerifyActor = NULL;
  }
}

static const char TPC_MODULE_DESCRIPTOR_NAME[] = "tpc";

int TpcModule::init(const char* config_path, idgs::Application* theApp){
  function_footprint();
  LOG(INFO) << "initialize module tpc_svc";
  app = theApp;

  /// Create LineCrud Actor
  lineCrudActor = new LineCrudActor;
  lineCrudActor->init();
  app->registerServiceActor(lineCrudActor); /// Register Actor

  migrationVerifyActor = new MigrationVerifyActor;
  app->registerServiceActor(migrationVerifyActor);

  syncVerifyActor = new SyncVerifyActor;
  app->registerServiceActor(syncVerifyActor);

  /// Register module descriptor
  std::shared_ptr<ModuleDescriptorWrapper> module_descriptor = std::make_shared<ModuleDescriptorWrapper>();
  module_descriptor->setName(TPC_MODULE_DESCRIPTOR_NAME);
  module_descriptor->setDescription("tpc module descriptor");

  module_descriptor->addActorDescriptor(lineCrudActor->getDescriptor());
  module_descriptor->addActorDescriptor(MigrationVerifyActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(SyncVerifyActor::generateActorDescriptor());
  idgs_application()->getActorDescriptorMgr()->registerModuleDescriptor(module_descriptor->getName(), module_descriptor);

  TransformerMgr& transformerMgr = *idgs_rdd_module()->getTransformManager();

  TransformerPtr tpchQ6Transformer = std::make_shared<TpchQ6Transformer>();
  transformerMgr.put(TPCH_Q6_TRANSFORMER, tpchQ6Transformer);

  TransformerPtr ssbQ1Transformer = std::make_shared<SsbQ1_1Transformer>();
  transformerMgr.put(SSB_Q1_1_TRANSFORMER, ssbQ1Transformer);

  ActionMgr& actionMgr = *idgs_rdd_module()->getActionManager();

  ActionPtr tpchQ6Action = std::make_shared<TpchQ6Action>();
  actionMgr.put(TPCH_Q6_ACTION, tpchQ6Action);

  ActionPtr ssbQ1Action = std::make_shared<SsbQ1_1Action>();
  actionMgr.put(SSB_Q1_1_ACTION, ssbQ1Action);

  ActionPtr partition_count_action = std::make_shared<PartitionCountAction>();
  actionMgr.put(PARTITION_COUNT_ACTION, partition_count_action);

  return RC_OK;
}

int TpcModule::start(void) {
  function_footprint();
  return RC_OK;
}

int TpcModule::stop(void){
  function_footprint();
  LOG(INFO) << "stop module tpc_svc";

  TransformerMgr& transformerMgr = *idgs_rdd_module()->getTransformManager();
  transformerMgr.remove(TPCH_Q6_TRANSFORMER);
  transformerMgr.remove(SSB_Q1_1_TRANSFORMER);

  ActionMgr& actionMgr = *idgs_rdd_module()->getActionManager();
  actionMgr.remove(TPCH_Q6_ACTION);
  actionMgr.remove(SSB_Q1_1_ACTION);

  DVLOG(3) << "begin terminate actor";
  if (lineCrudActor) {
    lineCrudActor->terminate();
  }

  if (migrationVerifyActor) {
    migrationVerifyActor->terminate();
  }

  if (syncVerifyActor) {
    syncVerifyActor->terminate();
  }

  DVLOG(3) << "end terminate actor";

  DVLOG(3) << "begin unregister module descriptor";
  idgs_application()->getActorDescriptorMgr()->unRegisterModuleDescriptor(TPC_MODULE_DESCRIPTOR_NAME);/// unregister module descriptor
  DVLOG(3) << "end unregister module actor";

  return RC_OK;
}
} // namespace tpc
} // namespace idgs

static TpcModule* module = NULL;
/// entry point of this module
Module*  get_idgs_module(void) {
  if(!module) {
    module = new TpcModule();
  }
  return module;
}

void release_idgs_module(idgs::Module* mod) {
  if(mod) {
    if (module == mod) {
      module = NULL;
    }
    delete mod;
  }
}



