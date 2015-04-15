/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "rdd_internal_service_actor.h"


#include "idgs/rdd/pair_rdd_actor.h"
#include "idgs/rdd/rdd_module.h"
#include "idgs/rdd/rdd_store_listener.h"
#include "idgs/rdd/pair_store_delegate_rdd_actor.h"
#include "idgs/rdd/pair_store_delegate_rdd_partition.h"

#include "idgs/rdd/pb/rdd_internal.pb.h"

#include "idgs/store/store_module.h"

#include "idgs/util/utillity.h"

#include "protobuf/type_composer.h"

using namespace std;
using namespace idgs::actor;
using namespace idgs::pb;
using namespace idgs::rdd::op;
using namespace idgs::rdd::pb;
using namespace protobuf;

namespace idgs {
namespace rdd {

RddInternalServiceActor::RddInternalServiceActor() {
  this->actorId = RDD_INTERNAL_SERVICE_ACTOR;
  descriptor = RddInternalServiceActor::generateActorDescriptor();
}

RddInternalServiceActor::~RddInternalServiceActor() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  rddLocalMap.clear();
}

const ActorMessageHandlerMap& RddInternalServiceActor::getMessageHandlerMap() const {
  static ActorMessageHandlerMap handlerMap = {
      {CREATE_STORE_DELEGATE_RDD,      static_cast<ActorMessageHandler>(&RddInternalServiceActor::handleCreateStoreDelegate)},
      {CREATE_DELEGATE_PARTITION,      static_cast<ActorMessageHandler>(&RddInternalServiceActor::handleCreateDelegatePartition)},
      {CREATE_RDD,                     static_cast<ActorMessageHandler>(&RddInternalServiceActor::handleCreateRdd)},
      {CREATE_RDD_PARTITION,           static_cast<ActorMessageHandler>(&RddInternalServiceActor::handleCreateRddPartition)},
      {PARTITION_CREATED,              static_cast<ActorMessageHandler>(&RddInternalServiceActor::handlePartitionCreated)},
      {RDD_ACTION_REQUEST,             static_cast<ActorMessageHandler>(&RddInternalServiceActor::handleRddActionRequest)},
      {RDD_DESTROY,                    static_cast<ActorMessageHandler>(&RddInternalServiceActor::handleRddDestroy)},
      {REMOVE_RDD_LOCAL,               static_cast<ActorMessageHandler>(&RddInternalServiceActor::handleRemoveRddLocal)},
      {RDD_STATE_SYNC,                 static_cast<ActorMessageHandler>(&RddInternalServiceActor::handleRddStateSync)},
      {PERSIST_TYPE_SYNC,              static_cast<ActorMessageHandler>(&RddInternalServiceActor::handlePersistTypeSync)},
      {MEMBER_CHANGE_EVENT,            static_cast<ActorMessageHandler>(&RddInternalServiceActor::handleMemberEvent)},

      {OID_LIST_RDD,                   static_cast<ActorMessageHandler>(&RddInternalServiceActor::handleListRdd)},
  };
  return handlerMap;
}

ActorDescriptorPtr RddInternalServiceActor::generateActorDescriptor() {
  static ActorDescriptorPtr descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = std::make_shared<ActorDescriptorWrapper>();

  descriptor->setName(RDD_INTERNAL_SERVICE_ACTOR);
  descriptor->setDescription("RDD Internal Service");
  descriptor->setType(AT_STATELESS);

  // in operation
  ActorOperationDescriporWrapper createDelegate;
  createDelegate.setName(CREATE_STORE_DELEGATE_RDD);
  createDelegate.setDescription("create store delegate RDD");
  createDelegate.setPayloadType("idgs.rdd.pb.CreateDelegateRddRequest");
  descriptor->setInOperation(createDelegate.getName(), createDelegate);

  ActorOperationDescriporWrapper createDelegatePartition;
  createDelegatePartition.setName(CREATE_DELEGATE_PARTITION);
  createDelegatePartition.setDescription("create RDD partition of store delegate");
  createDelegatePartition.setPayloadType("idgs.rdd.pb.CreateDelegateRddRequest");
  descriptor->setInOperation(createDelegatePartition.getName(), createDelegatePartition);

  ActorOperationDescriporWrapper createRDD;
  createRDD.setName(CREATE_RDD);
  createRDD.setDescription("create transform RDD");
  createRDD.setPayloadType("idgs.rdd.pb.CreateRddRequest");
  descriptor->setInOperation(createRDD.getName(), createRDD);

  ActorOperationDescriporWrapper createRddPartition;
  createRddPartition.setName(CREATE_RDD_PARTITION);
  createRddPartition.setDescription("create RDD partition");
  createRddPartition.setPayloadType("idgs.rdd.pb.CreateRddRequest");
  descriptor->setInOperation(createRddPartition.getName(), createRddPartition);

  ActorOperationDescriporWrapper partitionCreated;
  partitionCreated.setName(PARTITION_CREATED);
  partitionCreated.setDescription("receive RDD and partition info and parse transformer request");
  partitionCreated.setPayloadType("idgs.rdd.pb.RddActorInfo");
  descriptor->setInOperation(partitionCreated.getName(), partitionCreated);

  ActorOperationDescriporWrapper actionRequest;
  actionRequest.setName(RDD_ACTION_REQUEST);
  actionRequest.setDescription("handle action for RDD");
  actionRequest.setPayloadType("idgs.rdd.pb.ActionRequest");
  descriptor->setInOperation(actionRequest.getName(), actionRequest);

  ActorOperationDescriporWrapper rddDestroy;
  rddDestroy.setName(RDD_DESTROY);
  rddDestroy.setDescription("destroy named RDD");
  rddDestroy.setPayloadType("idgs.rdd.pb.DestroyRddRequest");
  descriptor->setInOperation(rddDestroy.getName(), rddDestroy);

  ActorOperationDescriporWrapper removeRddLocal;
  removeRddLocal.setName(REMOVE_RDD_LOCAL);
  removeRddLocal.setDescription("remove rddlocal");
  removeRddLocal.setPayloadType("idgs.rdd.pb.DestroyRddRequest");
  descriptor->setInOperation(removeRddLocal.getName(), removeRddLocal);

  ActorOperationDescriporWrapper rddStateSync;
  rddStateSync.setName(RDD_STATE_SYNC);
  rddStateSync.setDescription("change state of RDD");
  rddStateSync.setPayloadType("idgs.rdd.pb.RddStateTracing");
  descriptor->setInOperation(rddStateSync.getName(), rddStateSync);

  ActorOperationDescriporWrapper persistTypeSync;
  persistTypeSync.setName(PERSIST_TYPE_SYNC);
  persistTypeSync.setDescription("change persist type of RDD");
  persistTypeSync.setPayloadType("idgs.rdd.pb.PersistInfo");
  descriptor->setInOperation(persistTypeSync.getName(), persistTypeSync);

  // out operation
  // out operation for CREATE_STORE_DELEGATE_RDD and CREATE_RDD
  ActorOperationDescriporWrapper createRddResponse;
  createRddResponse.setName(CREATE_RDD_RESPONSE);
  createRddResponse.setDescription("the response of create RDD.");
  createRddResponse.setPayloadType("idgs.rdd.pb.CreateDelegateRddResponse");
  descriptor->setOutOperation(createRddResponse.getName(), createRddResponse);

  // out operation for CREATE_DELEGATE_PARTITION and CREATE_RDD_PARTITION
  ActorOperationDescriporWrapper createPartitionResponse;
  createPartitionResponse.setName(CREATE_RDD_PARTITION_RESPONSE);
  createPartitionResponse.setDescription("response of create RDD partition");
  createPartitionResponse.setPayloadType("idgs.rdd.pb.CreateRddPartitionResponse");
  descriptor->setOutOperation(createPartitionResponse.getName(), createPartitionResponse);

  // out operation for MULTICAST_RDD_INFO
  ActorOperationDescriporWrapper rddPrepared;
  rddPrepared.setName(RDD_PARTITION_PREPARED);
  rddPrepared.setDescription("response of multicast delegate RDD or common RDD");
  rddPrepared.setPayloadType("idgs.rdd.pb.RddResponse");
  descriptor->setOutOperation(rddPrepared.getName(), rddPrepared);

  ActorOperationDescriporWrapper rddTransformPrepared;
  rddTransformPrepared.setName(RDD_TRANSFORM_PREPARED);
  rddTransformPrepared.setDescription("action prepared of each member");
  rddTransformPrepared.setPayloadType("idgs.rdd.pb.RddResponse");
  descriptor->setOutOperation(rddTransformPrepared.getName(), rddTransformPrepared);

  // out operation for RDD_ACTION_REQUEST
  ActorOperationDescriporWrapper actionResponse;
  actionResponse.setName(RDD_ACTION_RESPONSE);
  actionResponse.setDescription("response of RDD action");
  actionResponse.setPayloadType("idgs.rdd.pb.ActionResponse");
  descriptor->setOutOperation(actionResponse.getName(), actionResponse);
  descriptor->setOutOperation(actionRequest.getName(), actionRequest);

  // out operation for RDD_DESTROY
  ActorOperationDescriporWrapper rddOpDestroy;
  rddOpDestroy.setName(OP_DESTROY);
  rddOpDestroy.setDescription("destroy named RDD");
  rddOpDestroy.setPayloadType("idgs.rdd.pb.DestroyRddRequest");
  descriptor->setOutOperation(rddOpDestroy.getName(), rddOpDestroy);

  ActorOperationDescriporWrapper destroyRddResponse;
  destroyRddResponse.setName(RDD_DESTROY_RESPONSE);
  destroyRddResponse.setDescription("response of destroy named RDD");
  destroyRddResponse.setPayloadType("idgs.rdd.pb.RddResponse");
  descriptor->setOutOperation(destroyRddResponse.getName(), destroyRddResponse);

  // no out operation for REMOVE_RDD_LOCAL and RDD_STATE_CHANGED

  return descriptor;
}

const RddLocal* RddInternalServiceActor::getRddLocal(const std::string& rddName) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  auto it = rddLocalMap.find(rddName);
  if (it == rddLocalMap.end()) {
    return NULL;
  } else {
    return &it->second;
  }
}

void RddInternalServiceActor::handleCreateStoreDelegate(const ActorMessagePtr& msg) {
  CreateDelegateRddRequest* request = dynamic_cast<CreateDelegateRddRequest*>(msg->getPayload().get());
  const string& rddName = request->rdd_name();
  const string& storeName = request->store_name();
  const string& schemaName = request->schema_name();

  DVLOG(3) << "Start to create store delegate RDD " << rddName << ".";

  shared_ptr<CreateDelegateRddResponse> response = make_shared<CreateDelegateRddResponse>();

  idgs::store::StorePtr store;
  auto datastore = idgs::store::idgs_store_module()->getDataStore();
  if (request->has_schema_name()) {
    store = datastore->getStore(schemaName, storeName);
  } else {
    store = datastore->getStore(storeName);
  }

  if (!store) {
    LOG(ERROR) << "store '" << schemaName << "." << storeName << "' was not found.";
    response->set_result_code(RRC_STORE_NOT_FOUND);
    sendResponse(msg, CREATE_RDD_RESPONSE, response);
    return;
  }
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);

  auto& configWrapper = store->getStoreConfigWrapper();
  auto it = rddLocalMap.find(rddName);
  if (it != rddLocalMap.end()) {
    auto listener = it->second.getRddStoreListener();
    if (listener) {
      auto& listeners = const_cast<vector<idgs::store::StoreListener*>&>(configWrapper->getStoreListener());
      listeners.erase(remove(listeners.begin(),listeners.end(), listener));
    }

    rddLocalMap.erase(rddName);
  }

  VLOG(1) << "Store Delegate RDD " << rddName << " created, the state is INIT.";
  PairStoreDelegateRddActor* rdd = new PairStoreDelegateRddActor(rddName);

  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
  response->mutable_rdd_id()->set_actor_id(rdd->getActorId());
  response->mutable_rdd_id()->set_member_id(localMemberId);
  idgs_application()->getRpcFramework()->getActorManager()->registerSessionActor(rdd->getActorId(), rdd);

  rddLocalMap.insert(pair<string, RddLocal>(rddName, RddLocal()));
  RddLocal& rddLocal = rddLocalMap[rddName];
  rddLocal.setRddInfo(rddName, localMemberId, rdd->getActorId(), INIT);
  rddLocal.setTransformerMsg(msg);
  rddLocal.setDelegateRdd(true);
  rdd->setRddLocal(&rddLocal);

  rdd->process(const_cast<const ActorMessagePtr&>(msg));

  DVLOG(3) << "handle create store delegate RDD done, send response to client";
  response->set_result_code(RRC_SUCCESS);
  sendResponse(msg, CREATE_RDD_RESPONSE, response);
}

void RddInternalServiceActor::handleCreateDelegatePartition(const ActorMessagePtr& msg) {
  CreateDelegateRddRequest* request = dynamic_cast<CreateDelegateRddRequest*>(msg->getPayload().get());
  shared_ptr<CreateRddPartitionResponse> response = make_shared<CreateRddPartitionResponse>();

  const string& rddName = request->rdd_name();
  const string& storeName = request->store_name();
  const string& schemaName = request->schema_name();

  idgs::store::StorePtr store;
  auto datastore = idgs::store::idgs_store_module()->getDataStore();
  if (request->has_schema_name()) {
    store = datastore->getStore(schemaName, storeName);
  } else {
    store = datastore->getStore(storeName);
  }

  if (!store) {
    LOG(ERROR) << "store '" << schemaName << "." << storeName << "' was not found.";
    response->set_result_code(RRC_STORE_NOT_FOUND);
    sendResponse(msg, CREATE_RDD_PARTITION_RESPONSE, response);
    return;
  }

  auto& configWrapper = store->getStoreConfigWrapper();

  auto config = configWrapper->getStoreConfig();
  auto storeKeyTemplate = configWrapper->getKeyTemplate();
  auto storeValueTemplate = configWrapper->getValueTemplate();

  // copy store key-value template to RDD module
  auto helper = idgs_rdd_module()->getMessageHelper();
  if (!helper->isMessageRegistered(storeKeyTemplate->GetDescriptor()->full_name())) {
    helper->addPbPrototype(storeKeyTemplate);
  }

  if (!helper->isMessageRegistered(storeValueTemplate->GetDescriptor()->full_name())) {
    helper->addPbPrototype(storeValueTemplate);
  }

  DVLOG(3) << "create store delegate RDD partition for " << rddName << " on member " << to_string(msg->getDestMemberId());

  auto rddMemberId = msg->getSourceMemberId();
  auto& rddActorId = msg->getSourceActorId();
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);

  auto itRddLocal = rddLocalMap.find(rddName);
  if (itRddLocal != rddLocalMap.end()) {
    auto rddId = itRddLocal->second.getRddId();
    if (rddId.member_id() != rddMemberId || rddId.actor_id() != rddActorId) {
      auto listener = itRddLocal->second.getRddStoreListener();
      if (listener) {
        auto& listeners = const_cast<vector<idgs::store::StoreListener*>&>(configWrapper->getStoreListener());
        listeners.erase(remove(listeners.begin(),listeners.end(), listener));
      }

      rddLocalMap.erase(rddName);
      rddLocalMap.insert(pair<string, RddLocal>(rddName, RddLocal()));
    }
  } else {
    rddLocalMap.insert(pair<string, RddLocal>(rddName, RddLocal()));
  }

  RddLocal& rddLocal = rddLocalMap[rddName];
  rddLocal.setRddInfo(rddName, rddMemberId, rddActorId, CREATED);
  rddLocal.setPersistType(ORDERED);
  rddLocal.setTransformerMsg(msg);
  rddLocal.setDelegateRdd(true);

  auto keyTemplate = configWrapper->newKey();
  auto valueTemplate = configWrapper->newValue();
  rddLocal.setKeyValueTemplate(keyTemplate, valueTemplate);
  rddLocal.setReplicatedRdd(config.partition_type() == idgs::store::pb::REPLICATED);

  auto& partitions = rddLocal.getLocalPartitions();
  auto it = partitions.begin();
  auto actorFramework = idgs_application()->getRpcFramework()->getActorManager();
  for (; it != partitions.end(); ++ it) {
    PairStoreDelegateRddPartition* rddPartition = new PairStoreDelegateRddPartition(rddName, (* it));
    actorFramework->registerSessionActor(rddPartition->getActorId(), rddPartition);

    auto rddPart = response->add_rdd_partition();
    rddPart->set_partition((* it));
    rddPart->mutable_actor_id()->set_actor_id(rddPartition->getActorId());
    rddPart->mutable_actor_id()->set_member_id(msg->getDestMemberId());

    rddLocal.addLocalPartition((* it), rddPartition);

    rddPartition->setRddLocal(&rddLocal);
    rddPartition->initPartitionStore(store);
  }

  RddStoreListener* listener = new RddStoreListener;
  rddLocal.setRddStoreListener(listener);
  listener->setRddLocal(&rddLocal);
  configWrapper->addStoreListener(listener);

  response->set_result_code(RRC_SUCCESS);
  sendResponse(msg, CREATE_RDD_PARTITION_RESPONSE, response);
}

void RddInternalServiceActor::handleCreateRdd(const ActorMessagePtr& msg) {
  CreateRddRequest* request = dynamic_cast<CreateRddRequest*>(msg->getPayload().get());
  const string& rddName = request->out_rdd().rdd_name();

  DVLOG(3) << "Start to create RDD " << rddName << ".";

  shared_ptr<CreateRddResponse> response = make_shared<CreateRddResponse>();
  if (request->in_rdd_size() == 0) {
    response->set_result_code(RRC_INVALID_RDD_INPUT);
    sendResponse(msg, CREATE_RDD_RESPONSE, response);
    return;
  }

  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);

  auto it = rddLocalMap.find(rddName);
  if (it != rddLocalMap.end()) {
    rddLocalMap.erase(it);
  }

  VLOG(1) << "RDD " << rddName << " created, the state is INIT.";
  PairRddActor* rdd = new PairRddActor(rddName);

  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
  response->mutable_rdd_id()->set_actor_id(rdd->getActorId());
  response->mutable_rdd_id()->set_member_id(localMemberId);
  idgs_application()->getRpcFramework()->getActorManager()->registerSessionActor(rdd->getActorId(), rdd);

  rddLocalMap.insert(pair<string, RddLocal>(rddName, RddLocal()));
  RddLocal& rddLocal = rddLocalMap[rddName];
  rddLocal.setRddInfo(rddName, localMemberId, rdd->getActorId(), INIT);

  rdd->setRddLocal(&rddLocal);

  rdd->process(const_cast<const ActorMessagePtr&>(msg));

  response->set_result_code(RRC_SUCCESS);
  sendResponse(msg, CREATE_RDD_RESPONSE, response);
}

void RddInternalServiceActor::handleCreateRddPartition(const ActorMessagePtr& msg) {
  CreateRddRequest* request = dynamic_cast<CreateRddRequest*>(msg->getPayload().get());
  shared_ptr<CreateRddPartitionResponse> response = make_shared<CreateRddPartitionResponse>();

  const string& rddName = request->out_rdd().rdd_name();

  DVLOG(3) << "create RDD partition for " << rddName << " on member " << to_string(msg->getDestMemberId());

  bool isRddMember = false;
  auto rddMemberId = msg->getSourceMemberId();
  auto& rddActorId = msg->getSourceActorId();
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);

  auto itRddLocal = rddLocalMap.find(rddName);
  if (itRddLocal != rddLocalMap.end()) {
    auto rddId = itRddLocal->second.getRddId();
    isRddMember = (rddId.member_id() == rddMemberId) && (rddId.actor_id() == rddActorId);
    if (!isRddMember) {
      rddLocalMap.erase(rddName);
      rddLocalMap.insert(pair<string, RddLocal>(rddName, RddLocal()));
    }
  } else {
    rddLocalMap.insert(pair<string, RddLocal>(rddName, RddLocal()));
  }

  RddLocal& rddLocal = rddLocalMap[rddName];
  rddLocal.setTransformerMsg(msg);
  if (isRddMember) {
    rddLocal.setRddState(CREATED);
  } else {
    rddLocal.setRddInfo(rddName, rddMemberId, rddActorId, CREATED);
  }

  auto& partitions = rddLocal.getLocalPartitions();
  auto it = partitions.begin();
  auto actorFramework = idgs_application()->getRpcFramework()->getActorManager();
  for (; it != partitions.end(); ++ it) {
    PairRddPartition* rddPartition = new PairRddPartition(rddName, (* it));
    actorFramework->registerSessionActor(rddPartition->getActorId(), rddPartition);

    rddLocal.addLocalPartition((* it), rddPartition);
    rddPartition->setRddLocal(&rddLocal);

    auto rddPart = response->add_rdd_partition();
    rddPart->set_partition((* it));
    rddPart->mutable_actor_id()->set_actor_id(rddPartition->getActorId());
    rddPart->mutable_actor_id()->set_member_id(msg->getDestMemberId());
  }

  response->set_result_code(RRC_SUCCESS);
  sendResponse(msg, CREATE_RDD_PARTITION_RESPONSE, response);
}

void RddInternalServiceActor::handlePartitionCreated(const idgs::actor::ActorMessagePtr& msg) {
  RddActorInfo* rddActorInfo = dynamic_cast<RddActorInfo*>(msg->getPayload().get());
  const string& rddName = rddActorInfo->rdd_name();

  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  RddLocal& rddLocal = rddLocalMap[rddName];

  shared_ptr<RddResponse> response = make_shared<RddResponse>();
  if (!rddLocal.isDelegateRdd()) {
    CreateRddRequest* request = dynamic_cast<CreateRddRequest*>(rddLocal.getTransformerMsg()->getPayload().get());
    auto inRddSize = request->in_rdd_size();

    for (int32_t i = 0; i < inRddSize; ++ i) {
      auto& cldRddName = request->in_rdd(i).rdd_name();
      auto pit = rddLocalMap.find(cldRddName);
      if (pit == rddLocalMap.end()) {
        idgs::actor::postMessage(const_cast<idgs::actor::ActorMessagePtr&>(msg));
        return;
      }

      auto state = pit->second.getRddState();
      if (state == ERROR) {
        LOG(ERROR) << "RDD named " << cldRddName << " is error.";
        response->set_result_code(RRC_RDD_ERROR);
        sendResponse(msg, RDD_PARTITION_PREPARED, response);
        return;
      } else if (state == INIT || state == CREATED) {
        idgs::actor::postMessage(const_cast<idgs::actor::ActorMessagePtr&>(msg));
        return;
      }
    }

    auto& transformerName = request->transformer_op_name();
    auto& transformerMgr = *idgs_rdd_module()->getTransformManager();
    auto& transformer = transformerMgr.get(transformerName);
    if (!transformer) {
      LOG(ERROR) << "transformer named " << transformerName << " is not found.";
      response->set_result_code(RRC_TRANSFORMER_NOT_FOUND);
      sendResponse(msg, RDD_PARTITION_PREPARED, response);
      return;
    }

    auto& outputRddInfo = request->out_rdd();
    if (outputRddInfo.has_persist_type()) {
      auto persistType = request->out_rdd().persist_type();
      rddLocal.setPersistType(persistType);
    } else {
      static char* t = getenv("DEFAULT_PERSIST_TYPE");
      if (t && string(t) == "NONE") {
        rddLocal.setPersistType(NONE);
      } else {
        rddLocal.setPersistType(ORDERED);
      }
    }

    if (outputRddInfo.has_input_sync()) {
      rddLocal.setUpstreamSync(outputRddInfo.input_sync());
    }

    rddLocal.setRepartition(request->repartition());
    rddLocal.setTransformer(transformer);

    RddResultCode code = registerPbMessage(outputRddInfo, to_string(msg->getDestMemberId()));
    if (code != RRC_SUCCESS) {
      LOG(ERROR) << "RDD " << rddName << " failed to register pb message, caused by " << RddResultCode_Name(code);
      response->set_result_code(code);
      sendResponse(msg, RDD_PARTITION_PREPARED, response);
      return;
    }

    auto helper = idgs_rdd_module()->getMessageHelper();
    auto keyTemplate = helper->createMessage(outputRddInfo.key_type_name());
    auto valueTemplate = helper->createMessage(outputRddInfo.value_type_name());
    rddLocal.setKeyValueTemplate(keyTemplate, valueTemplate);

    bool isReplicatedRdd = true;
    auto opMgr = idgs_rdd_module()->getRddOperatorManager();
    string opName = transformerName;
    RddOperator* fstOp = NULL;
    for (int32_t i = 0; i < inRddSize; ++ i) {
      auto& cldRddName = request->in_rdd(i).rdd_name();
      auto& rddlocal = rddLocalMap[cldRddName];

      rddLocal.addUpstreamRddLocal(&rddlocal);
      rddlocal.addDownstreamRddLocal(&rddLocal);

      RddOperator* rddOp = NULL;
      if (!rddLocal.isUpstreamSync() && i > 0) {
        opName = DEFAULT_OPERATOR;
        rddlocal.setPersistType(ORDERED);
      }

      auto& op = opMgr->get(opName);
      rddOp = (op) ? op->clone() : opMgr->get(DEFAULT_OPERATOR)->clone();
      if (!rddOp->parse(request->in_rdd(i), outputRddInfo, &rddlocal, &rddLocal)) {
        response->set_result_code(RRC_INVALID_RDD_INPUT);
        sendResponse(msg, RDD_PARTITION_PREPARED, response);
        return;
      }

      if (i == 0) {
        fstOp = rddOp;
      } else {
        fstOp->paramOperators.push_back(rddOp);
      }

      rddlocal.addRddOperator(msg, rddOp);
      isReplicatedRdd = isReplicatedRdd & rddlocal.isReplicatedRdd();
    }
    rddLocal.setReplicatedRdd(isReplicatedRdd);
    rddLocal.setMainRddOperator(fstOp);

    VLOG(1) << "RDD " << rddName << " transformer : " << transformerName
                                 << ", persist type : " << PersistType_Name(rddLocal.getPersistType())
                                 << ", repartition : " << (rddLocal.isRepartition() ? "true" : "false")
                                 << ", replicated : " << (rddLocal.isReplicatedRdd() ? "true" : "false")
                                 << ", synchronize : " << (rddLocal.isUpstreamSync() ? "true" : "false");
  }

  if (msg->getSourceMemberId() != msg->getDestMemberId()) {
    rddLocal.setRddState(rddActorInfo->state());
    auto itPartition = rddActorInfo->rdd_partition().begin();
    for (; itPartition != rddActorInfo->rdd_partition().end(); ++ itPartition) {
      rddLocal.addPartitionActor(itPartition->partition(), itPartition->actor_id());
      rddLocal.setPartitionState(itPartition->partition(), pb::PREPARED);
    }
  }

  DVLOG(3) << "update RDD info on member " << to_string(msg->getDestMemberId()) << " and send response.";
  response->set_result_code(RRC_SUCCESS);
  sendResponse(msg, RDD_PARTITION_PREPARED, response);
}

void RddInternalServiceActor::handleRddActionRequest(const ActorMessagePtr& msg) {
  ActionRequest* request = dynamic_cast<ActionRequest*>(msg->getPayload().get());
  shared_ptr<ActionResponse> response = make_shared<ActionResponse>();
  response->set_action_id(request->action_id());

  if (!request->has_rdd_name()) {
    LOG(ERROR) << "Name of RDD must be set in ActionRequest.";
    response->set_result_code(RRC_RDD_NOT_FOUND);
    sendResponse(msg, RDD_ACTION_RESPONSE, response);
    return;
  }

  const string& rddName = request->rdd_name();
  VLOG(2) << "RDD " << rddName << " received " << request->action_op_name() << " action with id: " << request->action_id();

  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);

  auto it = rddLocalMap.find(rddName);
  if (it == rddLocalMap.end()) {
    LOG(ERROR) << "RDD named " << rddName << " is not found on member " << msg->getSourceMemberId() << ", please retry.";
    response->set_result_code(RRC_RDD_NOT_FOUND);
    sendResponse(msg, RDD_ACTION_RESPONSE, response);
    return;
  }

  const ActorId& rddId = it->second.getRddId();
  auto respMsg = msg->createRouteMessage(rddId.member_id(), rddId.actor_id());
  idgs::actor::sendMessage(respMsg);
}

void RddInternalServiceActor::handleRddDestroy(const ActorMessagePtr& msg) {
  DestroyRddRequest* request = dynamic_cast<DestroyRddRequest*>(msg->getPayload().get());
  shared_ptr<RddResponse> response = make_shared<RddResponse>();

  auto& rddName = request->rdd_name();
  DVLOG(2) << "RDD Service destroy named RDD: " << rddName;
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);

  RddResultCode code = RRC_SUCCESS;
  auto it = rddLocalMap.find(rddName);
  if (it == rddLocalMap.end()) {
    LOG(ERROR) << "RDD named " << rddName << " is not found on member " << msg->getSourceMemberId() << ", please retry.";
    code = RRC_RDD_NOT_FOUND;
  } else {
    auto& rddid = it->second.getRddId();
    ActorMessagePtr message = createActorMessage();
    message->setDestMemberId(rddid.member_id());
    message->setDestActorId(rddid.actor_id());
    message->setOperationName(idgs::actor::OP_DESTROY);
    message->setPayload(make_shared<RddRequest>());
    idgs::actor::postMessage(message);
  }

  response->set_result_code(code);

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(RDD_DESTROY_RESPONSE);
  respMsg->setPayload(response);

  // send response
  idgs::actor::sendMessage(respMsg);
}

void RddInternalServiceActor::handleRemoveRddLocal(const idgs::actor::ActorMessagePtr& msg) {
  DestroyRddRequest* request = dynamic_cast<DestroyRddRequest*>(msg->getPayload().get());
  auto& rddName = request->rdd_name();
  DVLOG(3) << "RDD Service remove rddlocal named " << rddName << " on member " << msg->getDestMemberId();
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);

  auto it = rddLocalMap.find(rddName);
  if (it != rddLocalMap.end()) {
    if (it->second.isDelegateRdd()) {
      CreateDelegateRddRequest* request = dynamic_cast<CreateDelegateRddRequest*>(it->second.getTransformerMsg()->getPayload().get());

      const string& storeName = request->store_name();

      auto store = idgs::store::idgs_store_module()->getDataStore()->getStore(storeName);
      auto& configWrapper = store->getStoreConfigWrapper();

      auto& listeners = const_cast<vector<idgs::store::StoreListener*>&>(configWrapper->getStoreListener());
      listeners.erase(remove(listeners.begin(),listeners.end(), it->second.getRddStoreListener()));
    }
    rddLocalMap.erase(it);
  }
}

void RddInternalServiceActor::handleRddStateSync(const idgs::actor::ActorMessagePtr& msg) {
  RddStateTracing* response = dynamic_cast<RddStateTracing*>(msg->getPayload().get());
  auto& rddName = response->rdd_name();
  auto state = response->state();
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);

  auto it = rddLocalMap.find(rddName);
  if (it == rddLocalMap.end()) {
    LOG(ERROR) << "RDD named " << rddName << " is not found.";
    return;
  }

  it->second.setRddState(state);
}

void RddInternalServiceActor::handlePersistTypeSync(const idgs::actor::ActorMessagePtr& msg) {
  PersistInfo* payload = dynamic_cast<PersistInfo*>(msg->getPayload().get());
  shared_ptr<RddResponse> response = make_shared<RddResponse>();
  auto& rddName = payload->rdd_name();
  RddResultCode code = RRC_SUCCESS;

  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  auto it = rddLocalMap.find(rddName);
  if (it == rddLocalMap.end()) {
    LOG(ERROR) << "RDD named " << rddName << " is not found.";
    code = RRC_RDD_NOT_FOUND;
  } else {
    it->second.setPersistType(payload->persist_type());
  }

  response->set_result_code(code);
  sendResponse(msg, RDD_TRANSFORM_PREPARED, response);
}

void RddInternalServiceActor::handleListRdd(const idgs::actor::ActorMessagePtr& msg) {
  shared_ptr<idgs::rdd::pb::ListRddResponse> response = make_shared<idgs::rdd::pb::ListRddResponse>();
  for ( auto& en : rddLocalMap) {
    const auto& m = en.second.getTransformerMsg();
    const auto& operationName = m->getOperationName();
    if (operationName == CREATE_DELEGATE_PARTITION || operationName == CREATE_STORE_DELEGATE_RDD) {
      *response->add_store_delegate() = *dynamic_cast<idgs::rdd::pb::CreateDelegateRddRequest*>(m->getPayload().get());
    } else if (operationName == CREATE_RDD_PARTITION || operationName == CREATE_RDD) {
      *response->add_rdd() = *dynamic_cast<idgs::rdd::pb::CreateRddRequest*>(m->getPayload().get());
    } else {
      LOG(ERROR) << "Unknown RDD type: " << m->toString();
    }
  }

  auto result = msg->createResponse();
  result->setPayload(response);
  result->setOperationName("list_rdd_response");
  idgs::actor::sendMessage(result);
}


void RddInternalServiceActor::registerPbMessage(const string& protoFile) {
  ResultCode code = idgs_rdd_module()->getMessageHelper()->registerDynamicMessage(protoFile);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "register proto file "<< protoFile <<" failed, caused by " << getErrorDescription(code);
    return;
  }

  remove(protoFile.c_str());
}

RddResultCode RddInternalServiceActor::registerPbMessage(const OutRddInfo& out, const std::string& suffix) {
  string keyType = out.key_type_name();
  string valueType = out.value_type_name();

  auto helper = idgs_rdd_module()->getMessageHelper();
  bool isKeyReg = helper->isMessageRegistered(keyType);
  bool isValueReg = helper->isMessageRegistered(valueType);

  if (isKeyReg && isValueReg) {
    return RRC_SUCCESS;
  }

  const string& rddName = out.rdd_name();
  if (out.has_proto_message()) {
    string fileName = RDD_DYNAMIC_PROTO_PATH + "DM_" + rddName + "_" + suffix + ".proto";
    sys::saveFile(fileName, out.proto_message());
    registerPbMessage(fileName);
  } else {
    auto pos = keyType.find_last_of(".");
    string keyPackage = keyType.substr(0, pos);
    string keyName = keyType.substr(pos + 1, keyType.length());
    pos = valueType.find_last_of(".");
    string valuePackage = valueType.substr(0, pos);
    string valueName = valueType.substr(pos + 1, valueType.length());

    if (keyPackage != valuePackage) {
      if (!isKeyReg) {
        string keyFileName = RDD_DYNAMIC_PROTO_PATH + "DM_" + rddName + "_KEY_" + suffix + ".proto";
        DynamicMessage dmKey;
        dmKey.name = keyName;
        for (int32_t i = 0; i < out.key_fields_size(); ++i) {
          string name = out.key_fields(i).field_name();
          string type = idgs::str::toLower(DataType_Name(out.key_fields(i).field_type()));

          DynamicField field = DynamicField("required", type, name);
          dmKey.addField(field);
        }

        DynamicTypeComposer keyComposer;
        keyComposer.setName("KEY_" + rddName);
        keyComposer.setPackage(keyPackage);
        keyComposer.addMessage(dmKey);
        keyComposer.saveFile(keyFileName);

        registerPbMessage(keyFileName);
      }

      if (!isValueReg) {
        string valueFileName = RDD_DYNAMIC_PROTO_PATH + "DM_" + rddName + "_VALUE_" + suffix + ".proto";
        DynamicMessage dmValue;
        dmValue.name = valueName;
        for (int32_t i = 0; i < out.value_fields_size(); ++i) {
          string name = out.value_fields(i).field_name();
          string type = idgs::str::toLower(DataType_Name(out.value_fields(i).field_type()));

          DynamicField field = DynamicField("optional", type, name);
          dmValue.addField(field);
        }

        DynamicTypeComposer valueComposer;
        valueComposer.setName("VALUE_" + rddName);
        valueComposer.setPackage(valuePackage);
        valueComposer.addMessage(dmValue);
        valueComposer.saveFile(valueFileName);

        registerPbMessage(valueFileName);
      }
    } else {
      DynamicTypeComposer composer;
      composer.setName(rddName);
      if (!isKeyReg) {
        composer.setPackage(keyPackage);
        DynamicMessage dmKey;
        dmKey.name = keyName;
        for (int32_t i = 0; i < out.key_fields_size(); ++i) {
          string name = out.key_fields(i).field_name();
          string type = idgs::str::toLower(DataType_Name(out.key_fields(i).field_type()));

          DynamicField field = DynamicField("required", type, name);
          dmKey.addField(field);
        }
        composer.addMessage(dmKey);
      }

      if (!isValueReg) {
        composer.setPackage(valuePackage);
        DynamicMessage dmValue;
        dmValue.name = valueName;
        for (int32_t i = 0; i < out.value_fields_size(); ++ i) {
          string name = out.value_fields(i).field_name();
          string type = idgs::str::toLower(DataType_Name(out.value_fields(i).field_type()));

          DynamicField field = DynamicField("optional", type, name);
          dmValue.addField(field);
        }
        composer.addMessage(dmValue);
      }

      string fileName = RDD_DYNAMIC_PROTO_PATH + "DM_" + rddName + "_" + suffix + ".proto";
      composer.saveFile(fileName);
      registerPbMessage(fileName);
    }
  }

  isKeyReg = helper->isMessageRegistered(keyType);
  if (!isKeyReg) {
    return RRC_INVALID_KEY_SCHEMA;
  }

  isValueReg = helper->isMessageRegistered(valueType);
  if (!isValueReg) {
    return RRC_INVALID_VALUE_SCHEMA;
  }

  return RRC_SUCCESS;
}

void RddInternalServiceActor::sendResponse(const ActorMessagePtr& msg, const string& opName, const PbMessagePtr& payload) {
  auto respMsg = msg->createResponse();
  respMsg->setOperationName(opName);
  respMsg->setPayload(payload);
  sendMessage(respMsg);
}

void RddInternalServiceActor::handleMemberEvent(const idgs::actor::ActorMessagePtr& msg) {
  destroyAllRddActor();
}

void RddInternalServiceActor::onDestroy() {
  destroyAllRddActor();
  StatelessActor::onDestroy();
}

void RddInternalServiceActor::destroyAllRddActor() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
  auto it = rddLocalMap.begin();
  for (; it != rddLocalMap.end(); ++ it) {
    if (it->second.isDelegateRdd() && it->second.getRddId().member_id() == localMemberId) {
      DVLOG(2) << "Destroy actor " << it->second.getRddId().actor_id();
      terminate(it->second.getRddId().actor_id(), localMemberId);
    }
  }
}

} // namespace rdd
} // namespace idgs

