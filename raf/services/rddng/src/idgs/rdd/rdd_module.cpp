
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
#include "idgs/rdd/pair_store_delegate_rdd_actor.h"
#include "idgs/rdd/pair_store_delegate_rdd_partition.h"
#include "idgs/rdd/pair_rdd_actor.h"

#include "idgs/rdd/op/reduce_operator.h"
#include "idgs/rdd/op/join_operator.h"

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
using namespace idgs::rdd::op;
using namespace idgs::rdd::transform;
using namespace idgs::rdd::action;
using namespace google::protobuf;

namespace idgs {
namespace rdd {

RddModule::RddModule() : app(NULL),
    actionManager(NULL), transformManager(NULL), operatorManager(NULL),
    serviceActor(NULL), internalServiceActor(NULL), memberEventListener(NULL), messageHelper(NULL) {
  actionManager = new ActionMgr;
  transformManager = new TransformerMgr;
  operatorManager = new RddOperatorMgr;
}

RddModule::~RddModule() {
  if(actionManager) {
    delete actionManager;
    actionManager = NULL;
  }

  if(transformManager) {
    delete transformManager;
    transformManager = NULL;
  }

  if (operatorManager) {
    delete operatorManager;
    operatorManager = NULL;
  }

  if (messageHelper) {
    delete messageHelper;
    messageHelper = NULL;
  }

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
  std::shared_ptr<T> t = std::make_shared<T>();
  mgr.put(t->getName(), t);
}
template<typename T>
inline void registerAction(ActionMgr& mgr, const std::string& name) {
  std::shared_ptr<T> t = std::make_shared<T>();
  mgr.put(name, t);
}
}

int RddModule::init(const char* config_path, idgs::Application* theApp) {
  LOG(INFO)<< "init module rdd";
  app = theApp;

  /// stateless actor
  serviceActor = new RddServiceActor();
  app->getActorframework()->registerServiceActor(serviceActor->getActorId(), serviceActor);
  internalServiceActor = new RddInternalServiceActor();
  app->getActorframework()->registerServiceActor(internalServiceActor->getActorId(), internalServiceActor);

  shared_ptr<ModuleDescriptorWrapper> module_descriptor = std::make_shared<ModuleDescriptorWrapper>();
  module_descriptor->setName(RDD_MODULE_DESCRIPTOR_NAME);
  module_descriptor->setDescription(RDD_MODULE_DESCRIPTOR_DESCRIPTION);
  module_descriptor->addActorDescriptor(RddServiceActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(RddInternalServiceActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(PairStoreDelegateRddActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(PairStoreDelegateRddPartition::generateActorDescriptor());
  module_descriptor->addActorDescriptor(PairRddActor::generateActorDescriptor());
  module_descriptor->addActorDescriptor(PairRddPartition::generateActorDescriptor());

  app->getActorDescriptorMgr()->registerModuleDescriptor(module_descriptor->getName(), module_descriptor);

  /// register operator
  RddOperatorMgr& rddOperatorMgr = *getRddOperatorManager();
  RddOperatorPtr mapOp = make_shared<ExprMapOperator>();
  RddOperatorPtr reduceOp = make_shared<ReduceOperator>();
  RddOperatorPtr joinOp = make_shared<JoinOperator>();

  rddOperatorMgr.put(DEFAULT_OPERATOR, mapOp);
  rddOperatorMgr.put(FILTER_TRANSFORMER, mapOp);
  rddOperatorMgr.put(GROUP_TRANSFORMER, mapOp);
  rddOperatorMgr.put(UNION_TRANSFORMER, mapOp);
  rddOperatorMgr.put(REDUCE_BY_KEY_TRANSFORMER, reduceOp);
  rddOperatorMgr.put(REDUCE_TRANSFORMER, reduceOp);
  rddOperatorMgr.put(HASH_JOIN_TRANSFORMER, joinOp);

  ///
  /// register transformers
  ///
  TransformerMgr& transformerMgr = *getTransformManager();

  registerTransformer<FilterTransformer>(transformerMgr);
  registerTransformer<UnionTransformer>(transformerMgr);
  registerTransformer<GroupTransformer>(transformerMgr);
  registerTransformer<ReduceByKeyTransformer>(transformerMgr);
  registerTransformer<HashJoinTransformer>(transformerMgr);
  registerTransformer<ReduceTransformer>(transformerMgr);

  ///
  /// register actions
  ///
  ActionMgr& actionMgr = *getActionManager();
  registerAction<CountAction>(actionMgr, COUNT_ACTION);
  registerAction<SumAction>(actionMgr, SUM_ACTION);
  registerAction<LookupAction>(actionMgr, LOOKUP_ACTION);
  registerAction<CollectAction>(actionMgr, COLLECT_ACTION);
  registerAction<ExportAction>(actionMgr, EXPORT_ACTION);
  registerAction<TopNAction>(actionMgr, TOP_N_ACTION);

  resetMessageHelper();

  memberEventListener = new RddMemberEventListener;
  app->getMemberManager()->addListener(memberEventListener);

  return RC_OK;
}

int RddModule::start(void) {
  LOG(INFO)<< "start module rdd";

  return RC_OK;
}

int RddModule::stop(void) {
  LOG(INFO)<<"stopping module RDD";

  if (memberEventListener) {
    app->getMemberManager()->removeListener(memberEventListener);
    delete memberEventListener;
    memberEventListener = NULL;
  }

  // remove actor descriptor
  app->getActorDescriptorMgr()->unRegisterModuleDescriptor(RDD_MODULE_DESCRIPTOR_NAME);

  // unregister actor
  if (serviceActor) {
    serviceActor->terminate();
  }

  if (internalServiceActor) {
    internalServiceActor->terminate();
  }

  RddOperatorMgr& rddOperatorMgr = *getRddOperatorManager();
  rddOperatorMgr.remove(FILTER_TRANSFORMER);
  rddOperatorMgr.remove(GROUP_TRANSFORMER);
  rddOperatorMgr.remove(UNION_TRANSFORMER);
  rddOperatorMgr.remove(REDUCE_BY_KEY_TRANSFORMER);
  rddOperatorMgr.remove(REDUCE_TRANSFORMER);
  rddOperatorMgr.remove(HASH_JOIN_TRANSFORMER);

  TransformerMgr& transformerMgr = *getTransformManager();
  transformerMgr.remove(FILTER_TRANSFORMER);
  transformerMgr.remove(UNION_TRANSFORMER);
  transformerMgr.remove(GROUP_TRANSFORMER);
  transformerMgr.remove(HASH_JOIN_TRANSFORMER);
  transformerMgr.remove(REDUCE_BY_KEY_TRANSFORMER);
  transformerMgr.remove(PARALLEL_REDUCEVALUE_TRANSFORMER);

  ActionMgr& actionMgr = *getActionManager();
  actionMgr.remove(COUNT_ACTION);
  actionMgr.remove(SUM_ACTION);
  actionMgr.remove(LOOKUP_ACTION);
  actionMgr.remove(COLLECT_ACTION);
  actionMgr.remove(EXPORT_ACTION);
  actionMgr.remove(TOP_N_ACTION);

  string cmd = "rm -rf " + RDD_DYNAMIC_PROTO_PATH + "*.*";
  int ret = system(cmd.c_str());
  DVLOG(5) << "RDD module stopped " << ret;

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
