
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/rdd/rdd_module.h"
#include "idgs/application.h"

#include "idgs/rdd/rdd_service_actor.h"
#include "idgs/rdd/rdd_internal_service_actor.h"
#include "idgs/rdd/store_delegate_rdd_actor.h"
#include "idgs/rdd/store_delegate_rdd_partition.h"
#include "idgs/rdd/rdd_actor.h"

#include "idgs/rdd/transform/filter_transformer.h"
#include "idgs/rdd/transform/union_transformer.h"
#include "idgs/rdd/transform/group_transformer.h"
#include "idgs/rdd/transform/reducebykey_transformer.h"
#include "idgs/rdd/transform/reduce_transformer.h"
#include "idgs/rdd/transform/hash_join_transformer.h"

#include "idgs/rdd/action/count_action.h"
#include "idgs/rdd/action/sum_action.h"
#include "idgs/rdd/action/lookup_action.h"
#include "idgs/rdd/action/collect_action.h"
#include "idgs/rdd/action/export_action.h"
#include "idgs/rdd/action/top_n_action.h"

using namespace idgs::actor;
using namespace idgs::rdd;
using namespace idgs::rdd::transform;
using namespace idgs::rdd::action;
using namespace google::protobuf;

namespace idgs {
namespace rdd {

RddModule::~RddModule() {
  if (serviceActor) {
    delete serviceActor;
    serviceActor = NULL;
  }
  if (internalServiceActor) {
    delete internalServiceActor;
    internalServiceActor = NULL;
  }

}

namespace {
template<typename T>
inline void registerTransformer(TransformerMgr& mgr) {
  std::shared_ptr<T> t(std::make_shared<T>());
  mgr.put(t->getName(), t);
}
}

int RddModule::init(const char* config_path, idgs::Application* theApp) {
  LOG(INFO)<< "init module rdd";
  app = theApp;

  /// stateless actor
  serviceActor = new RddServiceActor();
  app->getActorframework()->Register(serviceActor->getActorId(), serviceActor);
  internalServiceActor = new RddInternalServiceActor();
  app->getActorframework()->Register(internalServiceActor->getActorId(), internalServiceActor);

  shared_ptr<ModuleDescriptorWrapper> module_descriptor(new ModuleDescriptorWrapper);
  module_descriptor->setName(RDD_MODULE_DESCRIPTOR_NAME);
  module_descriptor->setDescription(RDD_MODULE_DESCRIPTOR_DESCRIPTION);
  module_descriptor->addActorDescriptor(RddServiceActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(RddInternalServiceActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(StoreDelegateRddActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(StoreDelegateRddPartition::generateActorDescriptor());
  module_descriptor->addActorDescriptor(RddActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(RddPartition::generateActorDescriptor());

  ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().registerModuleDescriptor(module_descriptor->getName(), module_descriptor);

  ///
  /// register transformers
  ///
  TransformerMgr& transformerMgr = getTransformManager();

  registerTransformer<FilterTransformer>(transformerMgr);
  registerTransformer<UnionTransformer>(transformerMgr);
  registerTransformer<GroupTransformer>(transformerMgr);
  registerTransformer<ReduceByKeyTransformer>(transformerMgr);
//  registerTransformer<ParallelReduceByKeyTransformer>(transformerMgr);
  registerTransformer<HashJoinTransformer>(transformerMgr);
  registerTransformer<ReduceTransformer>(transformerMgr);

  ///
  /// register actions
  ///
  ActionMgr& actionMgr = getActionManager();
  ActionPtr countAction(new CountAction);
  actionMgr.put(COUNT_ACTION, countAction);

  ActionPtr sumAction(new SumAction);
  actionMgr.put(SUM_ACTION, sumAction);

  /// register lookup action
  ActionPtr lookupAction(new LookupAction);
  actionMgr.put(LOOKUP_ACTION, lookupAction);

  /// register collect action
  ActionPtr collectAction(new CollectAction);
  actionMgr.put(COLLECT_ACTION, collectAction);

  ActionPtr exportAction(new ExportAction);
  actionMgr.put(EXPORT_ACTION, exportAction);

  ActionPtr topNAction(new TopNAction);
  actionMgr.put(TOP_N_ACTION, topNAction);

  return RC_OK;
}

int RddModule::start(void) {
  LOG(INFO)<< "start module rdd";

  return RC_OK;
}

int RddModule::stop(void) {
  LOG(INFO)<<"stop module rdd";

  // remove actor descriptor
  ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().unRegisterModuleDescriptor(RDD_MODULE_DESCRIPTOR_NAME);

  // unregister actor
  app->getActorframework()->unRegisterStatelessActor(RDD_SERVICE_ACTOR);
  app->getActorframework()->unRegisterStatelessActor(RDD_INTERNAL_SERVICE_ACTOR);

  TransformerMgr& transformerMgr = getTransformManager();
  transformerMgr.remove(FILTER_TRANSFORMER);
  transformerMgr.remove(UNION_TRANSFORMER);
  transformerMgr.remove(GROUP_TRANSFORMER);
  transformerMgr.remove(HASH_JOIN_TRANSFORMER);
  transformerMgr.remove(REDUCEVALUE_TRANSFORMER);
  transformerMgr.remove(PARALLEL_REDUCEVALUE_TRANSFORMER);

  ActionMgr& actionMgr = getActionManager();
  actionMgr.remove(COUNT_ACTION);
  actionMgr.remove(SUM_ACTION);
  actionMgr.remove(LOOKUP_ACTION);
  actionMgr.remove(COLLECT_ACTION);
  actionMgr.remove(EXPORT_ACTION);
  actionMgr.remove(TOP_N_ACTION);

  string cmd = "rm -rf " + RDD_DYNAMIC_PROTO_PATH + "*.*";
  int ret = system(cmd.c_str());
  DVLOG(5) << "rdd module stop " << ret;

  return RC_OK;
}

static RddModule* module = NULL;
/// entry point of this module
RddModule* idgs_rdd_module(void) {
  if(!module) {
    module = new RddModule();
  }
  return module;
}
void  release_rdd_module(RddModule* mod) {
  if(!mod) {
    if(mod == module) {
      module = NULL;
    }
    delete mod;
  }
}


} // namespace rdd
}// namespace idgs

/// entry point of this module
idgs::Module* get_idgs_module(void) {
  return idgs_rdd_module();
}

void release_idgs_module(idgs::Module* mod) {
  release_rdd_module(dynamic_cast<idgs::rdd::RddModule*>(mod));
}
