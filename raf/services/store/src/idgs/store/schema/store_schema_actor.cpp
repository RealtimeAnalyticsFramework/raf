/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "store_schema_actor.h"

#include "idgs/store/store_module.h"
#include "idgs/store/data_store.h"

namespace idgs {
namespace store {

StoreSchemaActor::StoreSchemaActor(const std::string& actorId) {
  setActorId(actorId);

  descriptor = StoreSchemaActor::generateActorDescriptor();
}

StoreSchemaActor::~StoreSchemaActor() {
}

const idgs::actor::ActorMessageHandlerMap& StoreSchemaActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {OP_SHOWSTORES,              static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaActor::handleShowStores)},
      {OP_CREATE_SCHEMA,           static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaActor::handleGlobalRequest)},
      {OP_LOCAL_CREATE_SCHEMA,     static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaActor::handleLocalCreateSchema)},
      {OP_DROP_SCHEMA,             static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaActor::handleGlobalRequest)},
      {OP_LOCAL_DROP_SCHEMA,       static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaActor::handleLocalDropSchema)},
      {OP_CREATE_STORE,            static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaActor::handleGlobalRequest)},
      {OP_LOCAL_CREATE_STORE,      static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaActor::handleLocalCreateStore)},
      {OP_DROP_STORE,              static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaActor::handleGlobalRequest)},
      {OP_LOCAL_DROP_STORE,        static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaActor::handleLocalDropStore)}
  };

  return handlerMap;
}

idgs::actor::ActorDescriptorPtr StoreSchemaActor::generateActorDescriptor() {
  static std::shared_ptr<idgs::actor::ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = std::make_shared<idgs::actor::ActorDescriptorWrapper>();

  descriptor->setName(ACTORID_SCHEMA_SERVCIE);
  descriptor->setDescription("Data definition actor, the operation of schema and store.");
  descriptor->setType(idgs::pb::AT_STATELESS);

  // in operation
  idgs::actor::ActorOperationDescriporWrapper showStoresRequest;
  showStoresRequest.setName(OP_SHOWSTORES);
  showStoresRequest.setDescription("Retrieve all store names of schema");
  showStoresRequest.setPayloadType("idgs.store.pb.ShowStoresRequest");
  descriptor->setInOperation(showStoresRequest.getName(), showStoresRequest);

  idgs::actor::ActorOperationDescriporWrapper createSchemaRequest;
  createSchemaRequest.setName(OP_CREATE_SCHEMA);
  createSchemaRequest.setDescription("Request of create schema.");
  createSchemaRequest.setPayloadType("idgs.store.pb.CreateSchemaRequest");
  descriptor->setInOperation(createSchemaRequest.getName(), createSchemaRequest);

  idgs::actor::ActorOperationDescriporWrapper localCreateSchemaRequest;
  localCreateSchemaRequest.setName(OP_LOCAL_CREATE_SCHEMA);
  localCreateSchemaRequest.setDescription("Request of create schema.");
  localCreateSchemaRequest.setPayloadType("idgs.store.pb.CreateSchemaRequest");
  descriptor->setInOperation(localCreateSchemaRequest.getName(), localCreateSchemaRequest);

  idgs::actor::ActorOperationDescriporWrapper dropSchemaRequest;
  dropSchemaRequest.setName(OP_DROP_SCHEMA);
  dropSchemaRequest.setDescription("Request of drop schema.");
  dropSchemaRequest.setPayloadType("idgs.store.pb.DropSchemaRequest");
  descriptor->setInOperation(dropSchemaRequest.getName(), dropSchemaRequest);

  idgs::actor::ActorOperationDescriporWrapper localDropSchemaRequest;
  localDropSchemaRequest.setName(OP_LOCAL_DROP_SCHEMA);
  localDropSchemaRequest.setDescription("Request of drop schema.");
  localDropSchemaRequest.setPayloadType("idgs.store.pb.DropSchemaRequest");
  descriptor->setInOperation(localDropSchemaRequest.getName(), localDropSchemaRequest);

  idgs::actor::ActorOperationDescriporWrapper createStoreRequest;
  createStoreRequest.setName(OP_CREATE_STORE);
  createStoreRequest.setDescription("Request of create store.");
  createStoreRequest.setPayloadType("idgs.store.pb.CreateStoreRequest");
  descriptor->setInOperation(createStoreRequest.getName(), createStoreRequest);

  idgs::actor::ActorOperationDescriporWrapper localCreateStoreRequest;
  localCreateStoreRequest.setName(OP_LOCAL_CREATE_STORE);
  localCreateStoreRequest.setDescription("Request of create store.");
  localCreateStoreRequest.setPayloadType("idgs.store.pb.CreateStoreRequest");
  descriptor->setInOperation(localCreateStoreRequest.getName(), localCreateStoreRequest);

  idgs::actor::ActorOperationDescriporWrapper dropStoreRequest;
  dropStoreRequest.setName(OP_DROP_STORE);
  dropStoreRequest.setDescription("Request of drop store.");
  dropStoreRequest.setPayloadType("idgs.store.pb.DropStoreRequest");
  descriptor->setInOperation(dropStoreRequest.getName(), dropStoreRequest);

  idgs::actor::ActorOperationDescriporWrapper localDropStoreRequest;
  localDropStoreRequest.setName(OP_LOCAL_DROP_STORE);
  localDropStoreRequest.setDescription("Request of drop store.");
  localDropStoreRequest.setPayloadType("idgs.store.pb.DropStoreRequest");
  descriptor->setInOperation(localDropStoreRequest.getName(), localDropStoreRequest);

  // out operation
  // OP_SHOWSTORES
  idgs::actor::ActorOperationDescriporWrapper showStoresResponse;
  showStoresResponse.setName(OP_SHOWSTORES_RESPONSE);
  showStoresResponse.setDescription("Response of show stores");
  showStoresResponse.setPayloadType("idgs.store.pb.ShowStoresResponse");
  descriptor->setOutOperation(showStoresResponse.getName(), showStoresResponse);

  // OP_CREATE_SCHEMA
  descriptor->setOutOperation(createSchemaRequest.getName(), createSchemaRequest);

  // OP_LOCAL_CREATE_SCHEMA
  idgs::actor::ActorOperationDescriporWrapper createSchemaResponse;
  createSchemaResponse.setName(OP_CREATE_SCHEMA_RESPONSE);
  createSchemaResponse.setDescription("Response of create schema on each member");
  createSchemaResponse.setPayloadType("idgs.store.pb.CreateSchemaResponse");
  descriptor->setOutOperation(createSchemaResponse.getName(), createSchemaResponse);

  // OP_DROP_SCHEMA
  descriptor->setOutOperation(dropSchemaRequest.getName(), dropSchemaRequest);

  // OP_LOCAL_DROP_SCHEMA
  idgs::actor::ActorOperationDescriporWrapper dropSchemaResponse;
  dropSchemaResponse.setName(OP_DROP_SCHEMA_RESPONSE);
  dropSchemaResponse.setDescription("Response of drop schema on each member");
  dropSchemaResponse.setPayloadType("idgs.store.pb.DropSchemaResponse");
  descriptor->setOutOperation(dropSchemaResponse.getName(), dropSchemaResponse);

  // OP_CREATE_STORE
  descriptor->setOutOperation(createStoreRequest.getName(), createStoreRequest);

  // OP_LOCAL_CREATE_STORE
  idgs::actor::ActorOperationDescriporWrapper createStoreResponse;
  createStoreResponse.setName(OP_CREATE_STORE_RESPONSE);
  createStoreResponse.setDescription("Response of create store on each member");
  createStoreResponse.setPayloadType("idgs.store.pb.CreateStoreResponse");
  descriptor->setOutOperation(createStoreResponse.getName(), createStoreResponse);

  // OP_DROP_STORE
  descriptor->setOutOperation(dropStoreRequest.getName(), dropStoreRequest);

  // OP_LOCAL_DROP_STORE
  idgs::actor::ActorOperationDescriporWrapper dropStoreResponse;
  dropStoreResponse.setName(OP_DROP_STORE_RESPONSE);
  dropStoreResponse.setDescription("Response of drop store on each member");
  dropStoreResponse.setPayloadType("idgs.store.pb.DropStoreResponse");
  descriptor->setOutOperation(dropStoreResponse.getName(), dropStoreResponse);

  return descriptor;
}

void StoreSchemaActor::handleGlobalRequest(const idgs::actor::ActorMessagePtr& msg) {
  StoreSchemaAggrActor* actor = new StoreSchemaAggrActor;
  idgs_application()->registerSessionActor(actor);
  actor->process(msg);
}

void StoreSchemaActor::handleShowStores(const idgs::actor::ActorMessagePtr& msg) {
  auto memberMgr = idgs_application()->getMemberManager();
  auto local = memberMgr->getLocalMember();
  if (local->isLocalStore() && (local->getState() == idgs::pb::MS_ACTIVE || local->getState() == idgs::pb::MS_PREPARED)) {
    auto request = dynamic_cast<pb::ShowStoresRequest*>(msg->getPayload().get());
    auto datastore = idgs_store_module()->getDataStore();
    std::vector<StorePtr> stores;
    auto code = datastore->getStores(request->schema_name(), stores);

    std::shared_ptr<pb::ShowStoresResponse> response = std::make_shared<pb::ShowStoresResponse>();
    response->set_result_code(getResultCode(code));
    if (code == RC_SUCCESS) {
      auto it = stores.begin();
      for (; it != stores.end(); ++ it) {
        response->add_store_name((* it)->getStoreConfig()->getStoreConfig().name());
      }
    } else {
      LOG(ERROR) << "Show stores error, caused by " << getErrorDescription(code);
    }

    auto respMsg = msg->createResponse();
    respMsg->setOperationName(OP_SHOWSTORES_RESPONSE);
    respMsg->setPayload(response);
    idgs::actor::sendMessage(respMsg);
  } else {
    auto& members = memberMgr->getMemberTable();
    auto it = members.begin();
    for (; it != members.end(); ++ it) {
      if (it->isLocalStore() && (it->getState() == idgs::pb::MS_ACTIVE || it->getState() == idgs::pb::MS_PREPARED)) {
        auto routeMsg = msg->createRouteMessage(it->getId(), this->getActorId());
        idgs::actor::sendMessage(routeMsg);
        return;
      }
    }
  }
}

void StoreSchemaActor::handleLocalCreateSchema(const idgs::actor::ActorMessagePtr& msg) {
  DVLOG(2) << "Start to create schema on member " << std::to_string(msg->getDestMemberId());
  auto request = dynamic_cast<pb::CreateSchemaRequest*>(msg->getPayload().get());
  ResultCode code = RC_SUCCESS;
  auto datastore = idgs_store_module()->getDataStore();
  if (request->has_config()) {
    code = datastore->createDataStore(request->config(), true);
  }

  if (code == RC_SUCCESS && request->has_config_content()) {
    code = datastore->createDataStore(request->config_content(), true);
  }

  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Create schema error, caused by " << getErrorDescription(code);
  }

  std::shared_ptr<pb::CreateSchemaResponse> response = std::make_shared<pb::CreateSchemaResponse>();
  response->set_result_code(getResultCode(code));

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(OP_CREATE_SCHEMA_RESPONSE);
  respMsg->setPayload(response);
  idgs::actor::sendMessage(respMsg);
}

void StoreSchemaActor::handleLocalDropSchema(const idgs::actor::ActorMessagePtr& msg) {
  auto request = dynamic_cast<pb::DropSchemaRequest*>(msg->getPayload().get());
  DVLOG(2) << "Start to drop schema " << request->schema_name() << " on member " << std::to_string(msg->getDestMemberId());

  auto datastore = idgs_store_module()->getDataStore();

  auto code = datastore->dropSchema(request->schema_name());

  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Drop schema error, caused by " << getErrorDescription(code);
  }

  std::shared_ptr<pb::DropSchemaResponse> response = std::make_shared<pb::DropSchemaResponse>();
  response->set_result_code(getResultCode(code));

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(OP_DROP_SCHEMA_RESPONSE);
  respMsg->setPayload(response);
  idgs::actor::sendMessage(respMsg);
}

void StoreSchemaActor::handleLocalCreateStore(const idgs::actor::ActorMessagePtr& msg) {
  auto request = dynamic_cast<pb::CreateStoreRequest*>(msg->getPayload().get());
  DVLOG(2) << "Start to create store of schema " << request->schema().schema_name() << " on member " << std::to_string(msg->getDestMemberId());
  pb::DataStoreConfig config;
  if (request->has_schema()) {
    config.add_schemas()->CopyFrom(request->schema());
  }

  if (request->has_schema_content()) {
    config.add_schema_contents(request->schema_content());
  }

  auto datastore = idgs_store_module()->getDataStore();
  auto code = datastore->createDataStore(config, true);

  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Create store error, caused by " << getErrorDescription(code);
  }

  std::shared_ptr<pb::CreateStoreResponse> response = std::make_shared<pb::CreateStoreResponse>();
  response->set_result_code(getResultCode(code));

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(OP_CREATE_STORE_RESPONSE);
  respMsg->setPayload(response);
  idgs::actor::sendMessage(respMsg);
}

void StoreSchemaActor::handleLocalDropStore(const idgs::actor::ActorMessagePtr& msg) {
  auto request = dynamic_cast<pb::DropStoreRequest*>(msg->getPayload().get());
  DVLOG(2) << "Start to drop store " << request->store_name() << " of schema " << request->schema_name()
          << " on member " << std::to_string(msg->getDestMemberId());
  auto datastore = idgs_store_module()->getDataStore();

  auto code = datastore->dropStore(request->schema_name(), request->store_name());

  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Drop store error, caused by " << getErrorDescription(code);
  }

  std::shared_ptr<pb::DropStoreResponse> response = std::make_shared<pb::DropStoreResponse>();
  response->set_result_code(getResultCode(code));

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(OP_DROP_STORE_RESPONSE);
  respMsg->setPayload(response);
  idgs::actor::sendMessage(respMsg);
}

pb::StoreResultCode StoreSchemaActor::getResultCode(const ResultCode& status) {
  static std::map<ResultCode, pb::StoreResultCode> codeMapping = {
      {RC_SUCCESS,                       pb::SRC_SUCCESS},
      {RC_SCHEMA_NOT_FOUND,              pb::SRC_SCHEMA_NOT_FOUND},
      {RC_STORE_NOT_FOUND,               pb::SRC_STORE_NOT_FOUND},
      {RC_PARSE_CONFIG_ERROR,            pb::SRC_PARSE_CONFIG_ERROR},
      {RC_LOAD_PROTO_ERROR,              pb::SRC_PARSE_CONFIG_ERROR},
      {RC_INVALID_KEY,                   pb::SRC_INVALID_KEY},
      {RC_INVALID_VALUE,                 pb::SRC_INVALID_VALUE},
      {RC_KEY_TYPE_NOT_REGISTERED,       pb::SRC_INVALID_KEY},
      {RC_VALUE_TYPE_NOT_REGISTERED,     pb::SRC_INVALID_VALUE},
      {RC_STORE_EXISTED,                 pb::SRC_STORE_EXISTED},
      {RC_NOT_SUPPORT,                   pb::SRC_NOT_SUPPORT}
  };
  auto it = codeMapping.find(status);
  if (it == codeMapping.end()) {
    return pb::SRC_UNKNOWN_ERROR;
  }

  return it->second;
}

idgs::actor::ActorDescriptorPtr StoreSchemaAggrActor::descriptor;

StoreSchemaAggrActor::StoreSchemaAggrActor() : resultCode(pb::SRC_SUCCESS) {
  requestCount = 0;
  responseCount = 0;
}

StoreSchemaAggrActor::~StoreSchemaAggrActor() {
}

const idgs::actor::ActorMessageHandlerMap& StoreSchemaAggrActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {OP_CREATE_SCHEMA,              static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaAggrActor::handleCreateSchema)},
      {OP_CREATE_SCHEMA_RESPONSE,     static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaAggrActor::handleCreateSchemaResponse)},
      {OP_DROP_SCHEMA,                static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaAggrActor::handleDropSchema)},
      {OP_DROP_SCHEMA_RESPONSE,       static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaAggrActor::handleDropSchemaResponse)},
      {OP_CREATE_STORE,               static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaAggrActor::handleCreateStore)},
      {OP_CREATE_STORE_RESPONSE,      static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaAggrActor::handleCreateStoreResponse)},
      {OP_DROP_STORE,                 static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaAggrActor::handleDropStore)},
      {OP_DROP_STORE_RESPONSE,        static_cast<idgs::actor::ActorMessageHandler>(&StoreSchemaAggrActor::handleDropStoreResponse)}
  };

  return handlerMap;
}

const idgs::actor::ActorDescriptorPtr& StoreSchemaAggrActor::getDescriptor() const {
  return StoreSchemaAggrActor::descriptor;
}

idgs::actor::ActorDescriptorPtr StoreSchemaAggrActor::generateActorDescriptor() {
  if (descriptor) {
    return descriptor;
  }

  descriptor = std::make_shared<idgs::actor::ActorDescriptorWrapper>();

  descriptor->setName(DATA_STORE_SCHEMA_AGGR_ACTOR);
  descriptor->setDescription("Data definition aggregate actor, the operation of schema and store.");
  descriptor->setType(idgs::pb::AT_STATEFUL);

  // in operation
  idgs::actor::ActorOperationDescriporWrapper createSchemaRequest;
  createSchemaRequest.setName(OP_CREATE_SCHEMA);
  createSchemaRequest.setDescription("Route create schema message to all local store members");
  createSchemaRequest.setPayloadType("idgs.store.pb.CreateSchemaRequest");
  descriptor->setInOperation(createSchemaRequest.getName(), createSchemaRequest);

  idgs::actor::ActorOperationDescriporWrapper createSchemaResponse;
  createSchemaResponse.setName(OP_CREATE_SCHEMA_RESPONSE);
  createSchemaResponse.setDescription("Receive create schema response from local store members.");
  createSchemaResponse.setPayloadType("idgs.store.pb.CreateSchemaResponse");
  descriptor->setInOperation(createSchemaResponse.getName(), createSchemaResponse);

  idgs::actor::ActorOperationDescriporWrapper dropSchemaRequest;
  dropSchemaRequest.setName(OP_DROP_SCHEMA);
  dropSchemaRequest.setDescription("Route drop schema message to all local store members");
  dropSchemaRequest.setPayloadType("idgs.store.pb.DropSchemaRequest");
  descriptor->setInOperation(dropSchemaRequest.getName(), dropSchemaRequest);

  idgs::actor::ActorOperationDescriporWrapper dropSchemaResponse;
  dropSchemaResponse.setName(OP_DROP_SCHEMA_RESPONSE);
  dropSchemaResponse.setDescription("Receive drop schema response from local store members.");
  dropSchemaResponse.setPayloadType("idgs.store.pb.DropSchemaResponse");
  descriptor->setInOperation(dropSchemaResponse.getName(), dropSchemaResponse);

  idgs::actor::ActorOperationDescriporWrapper createStoreRequest;
  createStoreRequest.setName(OP_CREATE_STORE);
  createStoreRequest.setDescription("Route create store message to all local store members");
  createStoreRequest.setPayloadType("idgs.store.pb.CreateStoreRequest");
  descriptor->setInOperation(createStoreRequest.getName(), createStoreRequest);

  idgs::actor::ActorOperationDescriporWrapper createStoreResponse;
  createStoreResponse.setName(OP_CREATE_STORE_RESPONSE);
  createStoreResponse.setDescription("Receive create store response from local store members.");
  createStoreResponse.setPayloadType("idgs.store.pb.CreateStoreResponse");
  descriptor->setInOperation(createStoreResponse.getName(), createStoreResponse);

  idgs::actor::ActorOperationDescriporWrapper dropStoreRequest;
  dropStoreRequest.setName(OP_DROP_STORE);
  dropStoreRequest.setDescription("Route drop store message to all local store members");
  dropStoreRequest.setPayloadType("idgs.store.pb.DropStoreRequest");
  descriptor->setInOperation(dropStoreRequest.getName(), dropStoreRequest);

  idgs::actor::ActorOperationDescriporWrapper dropStoreResponse;
  dropStoreResponse.setName(OP_DROP_STORE_RESPONSE);
  dropStoreResponse.setDescription("Receive drop store response from local store members.");
  dropStoreResponse.setPayloadType("idgs.store.pb.DropStoreResponse");
  descriptor->setInOperation(dropStoreResponse.getName(), dropStoreResponse);

  // out operation
  // OP_CREATE_SCHEMA
  idgs::actor::ActorOperationDescriporWrapper createLocalSchemaRequest;
  createLocalSchemaRequest.setName(OP_LOCAL_CREATE_SCHEMA);
  createLocalSchemaRequest.setDescription("Route create schema message to all local store members.");
  createLocalSchemaRequest.setPayloadType("idgs.store.pb.CreateSchemaRequest");
  descriptor->setOutOperation(createLocalSchemaRequest.getName(), createLocalSchemaRequest);

  // OP_CREATE_SCHEMA_RESPONSE
  descriptor->setOutOperation(createSchemaResponse.getName(), createSchemaResponse);

  // OP_DROP_SCHEMA
  idgs::actor::ActorOperationDescriporWrapper dropLocalSchemaRequest;
  dropLocalSchemaRequest.setName(OP_LOCAL_DROP_SCHEMA);
  dropLocalSchemaRequest.setDescription("Route drop schema message to all local store members.");
  dropLocalSchemaRequest.setPayloadType("idgs.store.pb.DropSchemaRequest");
  descriptor->setOutOperation(dropLocalSchemaRequest.getName(), dropLocalSchemaRequest);

  // OP_DROP_SCHEMA_RESPONSE
  descriptor->setOutOperation(dropSchemaResponse.getName(), dropSchemaResponse);

  // OP_CREATE_STORE
  idgs::actor::ActorOperationDescriporWrapper createLocalStoreRequest;
  createLocalStoreRequest.setName(OP_LOCAL_CREATE_STORE);
  createLocalStoreRequest.setDescription("Route create store message to all local store members.");
  createLocalStoreRequest.setPayloadType("idgs.store.pb.CreateSchemaRequest");
  descriptor->setOutOperation(createLocalStoreRequest.getName(), createLocalStoreRequest);

  // OP_CREATE_STORE_RESPONSE
  descriptor->setOutOperation(createStoreResponse.getName(), createStoreResponse);

  // OP_CREATE_STORE
  idgs::actor::ActorOperationDescriporWrapper dropLocalStoreRequest;
  dropLocalStoreRequest.setName(OP_LOCAL_DROP_STORE);
  dropLocalStoreRequest.setDescription("Route drop store message to all local store members.");
  dropLocalStoreRequest.setPayloadType("idgs.store.pb.DropSchemaRequest");
  descriptor->setOutOperation(dropLocalStoreRequest.getName(), dropLocalStoreRequest);

  // OP_CREATE_STORE_RESPONSE
  descriptor->setOutOperation(dropStoreResponse.getName(), dropStoreResponse);

  StoreSchemaAggrActor::descriptor = descriptor;

  return descriptor;
}

void StoreSchemaAggrActor::handleCreateSchema(const idgs::actor::ActorMessagePtr& msg) {
  clientMsg = msg;
  multicast(msg, ACTORID_SCHEMA_SERVCIE, OP_LOCAL_CREATE_SCHEMA);
}

void StoreSchemaAggrActor::handleCreateSchemaResponse(const idgs::actor::ActorMessagePtr& msg) {
  handleMemberResponse<pb::CreateSchemaResponse>(msg);
}

void StoreSchemaAggrActor::handleDropSchema(const idgs::actor::ActorMessagePtr& msg) {
  clientMsg = msg;
  multicast(msg, ACTORID_SCHEMA_SERVCIE, OP_LOCAL_DROP_SCHEMA);
}

void StoreSchemaAggrActor::handleDropSchemaResponse(const idgs::actor::ActorMessagePtr& msg) {
  handleMemberResponse<pb::DropSchemaResponse>(msg);
}

void StoreSchemaAggrActor::handleCreateStore(const idgs::actor::ActorMessagePtr& msg) {
  clientMsg = msg;
  multicast(msg, ACTORID_SCHEMA_SERVCIE, OP_LOCAL_CREATE_STORE);
}

void StoreSchemaAggrActor::handleCreateStoreResponse(const idgs::actor::ActorMessagePtr& msg) {
  handleMemberResponse<pb::CreateStoreResponse>(msg);
}

void StoreSchemaAggrActor::handleDropStore(const idgs::actor::ActorMessagePtr& msg) {
  clientMsg = msg;
  multicast(msg, ACTORID_SCHEMA_SERVCIE, OP_LOCAL_DROP_STORE);
}

void StoreSchemaAggrActor::handleDropStoreResponse(const idgs::actor::ActorMessagePtr& msg) {
  handleMemberResponse<pb::DropStoreResponse>(msg);
}

} /* namespace store */
} /* namespace idgs */
