
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "data_store_actor.h"

#include "idgs/store/store_module.h"
#include "idgs/store/listener/listener_manager.h"
#include "idgs/store/storage/data_store_actor.h"
#include "idgs/store/schema/store_schema_actor.h"

#include "idgs/sync/store_migration_source_actor.h"
#include "idgs/sync/store_sync_source_actor.h"

using namespace idgs::actor;
using namespace idgs::store::pb;
using namespace google::protobuf;

namespace idgs {
namespace store {

StoreServiceActor::StoreServiceActor(const string& actorId) {
  setActorId(actorId);
  descriptor = StoreServiceActor::generateActorDescriptor();
}

StoreServiceActor::~StoreServiceActor() {
}

const idgs::actor::ActorMessageHandlerMap& StoreServiceActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {OP_INSERT,              static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleInsertStore)},
      {OP_INTERNAL_INSERT,     static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleLocalInsert)},
      {OP_GET,                 static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleGetStore)},
      {OP_INTERNAL_GET,        static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleLocalGet)},
      {OP_UPDATE,              static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleUpdateStore)},
      {OP_INTERNAL_UPDATE,     static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleLocalUpdate)},
      {OP_DELETE,              static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleDeleteStore)},
      {OP_INTERNAL_DELETE,     static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleLocalDelete)},
      {OP_COUNT,               static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleCountStore)},
      {OP_INTERNAL_COUNT,      static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleLocalCount)},
      {OP_TRUNCATE,            static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleTruncateStore)},
      {OP_INTERNAL_TRUNCATE,   static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleLocalTruncate)}
  };

  return handlerMap;
}

ActorDescriptorPtr StoreServiceActor::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = std::make_shared<ActorDescriptorWrapper>();

  descriptor->setName(ACTORID_STORE_SERVCIE);
  descriptor->setDescription("Data store CRUD operation");
  descriptor->setType(idgs::pb::AT_STATELESS);

  // operation OP_INSERT
  ActorOperationDescriporWrapper dataInsert;
  dataInsert.setName(OP_INSERT);
  dataInsert.setDescription("Receive insert data request from client, find the member which the data is on.");
  dataInsert.setPayloadType("idgs.store.pb.InsertRequest");
  descriptor->setInOperation(dataInsert.getName(), dataInsert);
  descriptor->setOutOperation(dataInsert.getName(), dataInsert);

  // operation OP_INTERNAL_INSERT
  ActorOperationDescriporWrapper dataLocalInsert;
  dataLocalInsert.setName(OP_INTERNAL_INSERT);
  dataLocalInsert.setDescription("Handle insert data operation on local member.");
  dataLocalInsert.setPayloadType("idgs.store.pb.InsertRequest");
  descriptor->setInOperation(dataLocalInsert.getName(), dataLocalInsert);

  // operation OP_INSERT_RESPONSE
  ActorOperationDescriporWrapper dataInsertResponse;
  dataInsertResponse.setName(OP_INSERT_RESPONSE);
  dataInsertResponse.setDescription("Send insert response data to client");
  dataInsertResponse.setPayloadType("idgs.store.pb.InsertResponse");
  descriptor->setOutOperation(dataInsertResponse.getName(), dataInsertResponse);

  // operation OP_GET
  ActorOperationDescriporWrapper dataGet;
  dataGet.setName(OP_GET);
  dataGet.setDescription("Receive get data request from client");
  dataGet.setPayloadType("idgs.store.pb.GetRequest");
  descriptor->setInOperation(dataGet.getName(), dataGet);
  descriptor->setOutOperation(dataGet.getName(), dataGet);

  // operation OP_INTERNAL_GET
  ActorOperationDescriporWrapper dataLocalGet;
  dataLocalGet.setName(OP_INTERNAL_GET);
  dataLocalGet.setDescription("Handle insert operation on local member.");
  dataLocalGet.setPayloadType("idgs.store.pb.GetRequest");
  descriptor->setInOperation(dataLocalGet.getName(), dataLocalGet);

  // operation OP_GET_RESPONSE
  ActorOperationDescriporWrapper dataGetResponse;
  dataGetResponse.setName(OP_GET_RESPONSE);
  dataGetResponse.setDescription("Send get data response to client");
  dataGetResponse.setPayloadType("idgs.store.pb.GetResponse");
  descriptor->setOutOperation(dataGetResponse.getName(), dataGetResponse);

  // operation OP_UPDATE
  ActorOperationDescriporWrapper dataUpdate;
  dataUpdate.setName(OP_UPDATE);
  dataUpdate.setDescription("Receive update data request from client, find the member which the data is on.");
  dataUpdate.setPayloadType("idgs.store.pb.UpdateRequest");
  descriptor->setInOperation(dataUpdate.getName(), dataUpdate);
  descriptor->setOutOperation(dataUpdate.getName(), dataUpdate);

  // operation OP_INTERNAL_UPDATE
  ActorOperationDescriporWrapper dataLocalUpdate;
  dataLocalUpdate.setName(OP_INTERNAL_UPDATE);
  dataLocalUpdate.setDescription("Handle update data operation on local member.");
  dataLocalUpdate.setPayloadType("idgs.store.pb.UpdateRequest");
  descriptor->setInOperation(dataLocalUpdate.getName(), dataLocalUpdate);

  // operation OP_UPDATE_RESPONSE
  ActorOperationDescriporWrapper dataUpdateResponse;
  dataUpdateResponse.setName(OP_UPDATE_RESPONSE);
  dataUpdateResponse.setDescription("Send update data response to client");
  dataUpdateResponse.setPayloadType("idgs.store.pb.UpdateResponse");
  descriptor->setInOperation(dataUpdateResponse.getName(), dataUpdateResponse);

  // operation OP_DELETE
  ActorOperationDescriporWrapper dataDelete;
  dataDelete.setName(OP_DELETE);
  dataDelete.setDescription("Receive delete data request from client, find the member which the data is on.");
  dataDelete.setPayloadType("idgs.store.pb.DeleteRequest");
  descriptor->setInOperation(dataDelete.getName(), dataDelete);
  descriptor->setOutOperation(dataDelete.getName(), dataDelete);

  // operation OP_INTERNAL_DELETE
  ActorOperationDescriporWrapper dataLocalDelete;
  dataLocalDelete.setName(OP_INTERNAL_DELETE);
  dataLocalDelete.setDescription("Handle delete data operation on local member.");
  dataLocalDelete.setPayloadType("idgs.store.pb.DeleteRequest");
  descriptor->setInOperation(dataLocalDelete.getName(), dataLocalDelete);

  // operation OP_DELETE_RESPONSE
  ActorOperationDescriporWrapper dataDeleteResponse;
  dataDeleteResponse.setName(OP_DELETE_RESPONSE);
  dataDeleteResponse.setDescription("Send delete data response to client");
  dataDeleteResponse.setPayloadType("idgs.store.pb.DeleteResponse");
  descriptor->setOutOperation(dataDeleteResponse.getName(), dataDeleteResponse);

  // operation OP_COUNT
  ActorOperationDescriporWrapper dataCount;
  dataCount.setName(OP_COUNT);
  dataCount.setDescription("Receive count request from client, handle insert operation.");
  dataCount.setPayloadType("idgs.store.pb.SizeRequest");
  descriptor->setInOperation(dataCount.getName(), dataCount);
  descriptor->setOutOperation(dataCount.getName(), dataCount);

  // operation OP_INTERNAL_COUNT
  ActorOperationDescriporWrapper dataLocalCount;
  dataLocalCount.setName(OP_INTERNAL_COUNT);
  dataLocalCount.setDescription("Handle count operation on local member.");
  dataLocalCount.setPayloadType("idgs.store.pb.SizeRequest");
  descriptor->setInOperation(dataLocalCount.getName(), dataLocalCount);

  // operation OP_COUNT_RESPONSE
  ActorOperationDescriporWrapper dataSizeResponse;
  dataSizeResponse.setName(OP_COUNT_RESPONSE);
  dataSizeResponse.setDescription("Send size data response to client");
  dataSizeResponse.setPayloadType("idgs.store.pb.SizeResponse");
  descriptor->setOutOperation(dataSizeResponse.getName(), dataSizeResponse);

  // operation OP_TRUNCATE
  ActorOperationDescriporWrapper storeTruncate;
  storeTruncate.setName(OP_TRUNCATE);
  storeTruncate.setDescription("Handle truncate store.");
  storeTruncate.setPayloadType("idgs.store.pb.TruncateRequest");
  descriptor->setInOperation(storeTruncate.getName(), storeTruncate);
  descriptor->setOutOperation(storeTruncate.getName(), storeTruncate);

  // operation OP_INTERNAL_TRUNCATE
  ActorOperationDescriporWrapper localStoreTruncate;
  localStoreTruncate.setName(OP_INTERNAL_TRUNCATE);
  localStoreTruncate.setDescription("Handle truncate store on local member.");
  localStoreTruncate.setPayloadType("idgs.store.pb.TruncateRequest");
  descriptor->setInOperation(localStoreTruncate.getName(), localStoreTruncate);

  // operator OP_TRUNCATE_RESPONSE
  ActorOperationDescriporWrapper storeTruncateResponse;
  storeTruncateResponse.setName(OP_TRUNCATE_RESPONSE);
  storeTruncateResponse.setDescription("Send truncate result to client");
  storeTruncateResponse.setPayloadType("idgs.store.pb.TruncateResponse");
  descriptor->setOutOperation(storeTruncateResponse.getName(), storeTruncateResponse);

  return descriptor;
}

template<typename REQUEST, typename RESPONSE>
ResultCode parseRequest(
    const idgs::actor::ActorMessagePtr& msg,
    bool hasValue,
    StorePtr& store,
    std::shared_ptr<google::protobuf::Message>& key,
    std::shared_ptr<google::protobuf::Message>& value,
    std::shared_ptr<RESPONSE>& response) {

  REQUEST* request = dynamic_cast<REQUEST*>(msg->getPayload().get());

  auto& schemaName = request->schema_name();
  auto& storeName = request->store_name();

  auto datastore = idgs_store_module()->getDataStore();
  if (request->has_schema_name()) {
    store = datastore->getStore(schemaName, storeName);
  } else {
    store = datastore->getStore(storeName);
  }

  if (!store) {
    LOG(ERROR)<< "Store not found: " << schemaName << "." << storeName;
    response = std::make_shared<RESPONSE>();
    response->set_result_code(SRC_STORE_NOT_FOUND);
    return idgs::RC_STORE_NOT_FOUND;
  }

  auto& storeConfig = store->getStoreConfig();

  key = storeConfig->newKey();
  if (!msg->parseAttachment(STORE_ATTACH_KEY, key.get())) {
    LOG(ERROR)<< "Invalid key, store: " << schemaName << "." << storeName;
    response = make_shared<pb::InsertResponse>();
    response->set_result_code(SRC_INVALID_KEY);
    return idgs::RC_INVALID_KEY;
  }

  if ( hasValue) {
    value = storeConfig->newValue();
    if (!msg->parseAttachment(STORE_ATTACH_VALUE, value.get())) {
      LOG(ERROR)<< "Invalid value, store: " << schemaName << "." << storeName;
      response = make_shared<pb::InsertResponse>();
      response->set_result_code(SRC_INVALID_VALUE);
      return idgs::RC_INVALID_VALUE;
    }
  }

  return idgs::RC_OK;
}





void StoreServiceActor::handleInsertStore(const ActorMessagePtr& msg) {
  pb::InsertRequest* request = dynamic_cast<pb::InsertRequest*>(msg->getPayload().get());

  StorePtr store;
  std::shared_ptr<google::protobuf::Message> key;
  std::shared_ptr<google::protobuf::Message> value;
  std::shared_ptr<pb::InsertResponse> response;

  auto rc = parseRequest<pb::InsertRequest, pb::InsertResponse> (msg, true, store, key, value, response);
  if ( rc != idgs::RC_OK) {
    sendResponse(msg, OP_INSERT_RESPONSE, response);
    return;
  }

  // calculate partition id and whether need to route message to right member
  calcStorePartition<pb::InsertRequest>(request, key, store->getStoreConfig().get());
  auto cluster = idgs_application()->getClusterFramework();
  auto partition = cluster->getPartitionManager()->getPartition(request->partition_id());
  int32_t destMemberId = partition->getPrimaryMemberId();
  if (destMemberId < 0) {
    LOG(ERROR)<< "Insert data to store " << request->schema_name() << "." << request->store_name()  << " failed, caused by partition not ready";
    shared_ptr<pb::InsertResponse> response = make_shared<pb::InsertResponse>();
    response->set_result_code(SRC_PARTITION_NOT_READY);
    sendResponse(msg, OP_INSERT_RESPONSE, response);
    return;
  }

  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
  if (destMemberId != localMemberId) {
    // need route message
    ActorMessagePtr message = msg->createRouteMessage(destMemberId, msg->getDestActorId(), true);
    DVLOG_FIRST_N(2, 20) << "partition " << request->partition_id() << ", route message from member " << localMemberId
        << " to member " << destMemberId << ", message: " << message->toString();
    idgs::actor::sendMessage(message);
    return;
  }

  // store is partition table
  if (store->getStoreConfig()->getStoreConfig().partition_type() == pb::PARTITION_TABLE) {
    // insert data from local member
    ActorMessagePtr message = const_cast<const ActorMessagePtr&>(msg);
    message->setOperationName(OP_INTERNAL_INSERT);
    realInsert(msg, store, key, value);

    auto state = partition->getMemberState(localMemberId);
    if (state == idgs::pb::PS_SOURCE) {
      addToRedoLog(store, request->partition_id(), OP_INTERNAL_INSERT, key, value);
    }
  // store is replicated
  } else if (store->getStoreConfig()->getStoreConfig().partition_type() == pb::REPLICATED) {
    // create DataAggregatorActor to multicast all members.
    ActorMessagePtr message = const_cast<const ActorMessagePtr&>(msg);
    message->setOperationName(OP_INTERNAL_INSERT);
    processReplicatedStore(message);
  }
}

void StoreServiceActor::handleLocalInsert(const ActorMessagePtr& msg) {
  auto cluster = idgs_application()->getClusterFramework();
  if (!cluster->getLocalMember()->isLocalStore()) {
    std::shared_ptr<InsertResponse> response = std::make_shared<InsertResponse>();
    response->set_result_code(pb::SRC_NOT_LOCAL_STORE);
    sendResponse(msg, OP_INSERT_RESPONSE, response);
    return;
  }

  // handle insert operation on local member
  InsertRequest* request = dynamic_cast<InsertRequest*>(msg->getPayload().get());

  StorePtr store;
  std::shared_ptr<google::protobuf::Message> key;
  std::shared_ptr<google::protobuf::Message> value;
  std::shared_ptr<pb::InsertResponse> response;

  auto rc = parseRequest<pb::InsertRequest, pb::InsertResponse> (msg, true, store, key, value, response);
  if ( rc != idgs::RC_OK) {
    sendResponse(msg, OP_INSERT_RESPONSE, response);
    return;
  }


  calcStorePartition<pb::InsertRequest>(request, key, store->getStoreConfig().get());

  realInsert(msg, store, key, value);
}

void StoreServiceActor::realInsert(const idgs::actor::ActorMessagePtr& msg, StorePtr& store,
    std::shared_ptr<google::protobuf::Message>& key, std::shared_ptr<google::protobuf::Message>& value) {
  InsertRequest* request = dynamic_cast<InsertRequest*>(msg->getPayload().get());

  // handle insert
  idgs::store::StoreOption ps;
  ps.partitionId = request->partition_id();
  StoreValue<Message> storeValue(value);
  ResultCode status = store->put(key, storeValue, &ps);
  if (status != RC_SUCCESS) {
    LOG(ERROR)<< "Insert data to store " << request->schema_name() << "." << request->store_name() << " failed, " << getErrorDescription(status);
    shared_ptr<InsertResponse> response = make_shared<InsertResponse>();
    response->set_result_code(getResultCode(status));
    sendResponse(msg, OP_INSERT_RESPONSE, response);
    return;
  }

  if (store->getStoreConfig()->getStoreConfig().partition_type() == pb::REPLICATED) {
    auto cluster = idgs_application()->getClusterFramework();
    int32_t primaryMemberId = cluster->getPartitionManager()->getPartition(request->partition_id())->getPrimaryMemberId();
    int32_t localMemberId = cluster->getMemberManager()->getLocalMemberId();
    if (primaryMemberId != localMemberId) {
      shared_ptr<InsertResponse> response = make_shared<InsertResponse>();
      response->set_result_code(pb::SRC_SUCCESS);
      sendResponse(msg, OP_INSERT_RESPONSE, response);
      return;
    }
  }

  sendStoreListener(msg);
}


void StoreServiceActor::handleGetStore(const ActorMessagePtr& msg) {
  // globe get operation
  GetRequest* request = dynamic_cast<GetRequest*>(msg->getPayload().get());

  auto& schemaName = request->schema_name();
  auto& storeName = request->store_name();

  DVLOG_FIRST_N(2, 10) << "handle globe get store " << schemaName << "." << storeName;

  StorePtr store;
  auto datastore = idgs_store_module()->getDataStore();
  if (request->has_schema_name()) {
    store = datastore->getStore(schemaName, storeName);
  } else {
    store = datastore->getStore(storeName);
  }

  if (!store) {
    LOG(ERROR)<< "Get data to store " << schemaName << "." << storeName << " failed, caused by store not found.";
    shared_ptr<GetResponse> response = make_shared<GetResponse>();
    response->set_result_code(SRC_STORE_NOT_FOUND);
    sendResponse(msg, OP_GET_RESPONSE, response);
    return;
  }

  auto& storeConfig = store->getStoreConfig();

  shared_ptr<Message> key = storeConfig->newKey();
  if (!msg->parseAttachment(STORE_ATTACH_KEY, key.get())) {
    LOG(ERROR)<< "Get data to store " << schemaName << "." << storeName << " failed, caused by invalid key";
    shared_ptr<GetResponse> response = make_shared<GetResponse>();
    response->set_result_code(SRC_INVALID_KEY);
    sendResponse(msg, OP_GET_RESPONSE, response);
    return;
  }

  // calculate partition id and whether need to route message to right member
  calcStorePartition<pb::GetRequest>(request, key, storeConfig.get());
  auto cluster = idgs_application()->getClusterFramework();
  auto partition = cluster->getPartitionManager()->getPartition(request->partition_id());
  int32_t destMemberId = partition->getPrimaryMemberId();
  if (destMemberId == -1) {
    LOG(ERROR)<< "Get data to store " << schemaName << "." << storeName << " failed, caused by partition not ready";
    shared_ptr<GetResponse> response = make_shared<GetResponse>();
    response->set_result_code(SRC_PARTITION_NOT_READY);
    sendResponse(msg, OP_GET_RESPONSE, response);
    return;
  }

  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
  if (destMemberId != localMemberId) {
    // need route message
    DVLOG_FIRST_N(2, 20) << "partition " << request->partition_id() << ", route message from member " << localMemberId << " to member " << destMemberId;
    ActorMessagePtr message = msg->createRouteMessage(destMemberId, msg->getDestActorId(), true);
    idgs::actor::sendMessage(message);
    return;
  }

  ActorMessagePtr message = const_cast<const ActorMessagePtr&>(msg);
  message->setOperationName(OP_INTERNAL_GET);
  handleLocalGet(message);
}

void StoreServiceActor::handleLocalGet(const idgs::actor::ActorMessagePtr& msg) {
  // handle get operation on local member
  GetRequest* request = dynamic_cast<GetRequest*>(msg->getPayload().get());

  auto& schemaName = request->schema_name();
  auto& storeName = request->store_name();

  DVLOG_FIRST_N(2, 20) << "Handle local get data from store " << schemaName << "." << storeName << " partition:" << request->partition_id();

  auto cluster = idgs_application()->getClusterFramework();
  if (!cluster->getLocalMember()->isLocalStore()) {
    shared_ptr<GetResponse> response = make_shared<GetResponse>();
    response->set_result_code(pb::SRC_NOT_LOCAL_STORE);
    sendResponse(msg, OP_GET_RESPONSE, response);
    return;
  }

  StorePtr store;
  auto datastore = idgs_store_module()->getDataStore();
  if (request->has_schema_name()) {
    store = datastore->getStore(schemaName, storeName);
  } else {
    store = datastore->getStore(storeName);
  }

  if (!store) {
    LOG(ERROR)<< "Get data to store " << schemaName << "." << storeName << " failed, caused by store not found.";
    shared_ptr<GetResponse> response = make_shared<GetResponse>();
    response->set_result_code(SRC_STORE_NOT_FOUND);
    sendResponse(msg, OP_GET_RESPONSE, response);
    return;
  }

  auto& storeConfig = store->getStoreConfig();

  shared_ptr<Message> key = storeConfig->newKey();
  if (!msg->parseAttachment(STORE_ATTACH_KEY, key.get())) {
    LOG(ERROR)<< "Get data to store " << schemaName << "." << storeName << " failed, caused by invalid key";
    shared_ptr<GetResponse> response = make_shared<GetResponse>();
    response->set_result_code(SRC_INVALID_KEY);
    sendResponse(msg, OP_GET_RESPONSE, response);
    return;
  }

  calcStorePartition<pb::GetRequest>(request, key, storeConfig.get());

  idgs::store::StoreOption ps;
  ps.partitionId = request->partition_id();
  shared_ptr<Message> value;
  StoreValue<Message> storeValue(value);
  ResultCode status = store->get(key, storeValue, &ps);
  if (status != RC_SUCCESS && status != RC_DATA_NOT_FOUND) {
    LOG(ERROR)<< "Get data from store " << schemaName << "." << storeName << " failed, " << getErrorDescription(status);
    shared_ptr<GetResponse> response = make_shared<GetResponse>();
    response->set_result_code(getResultCode(status));
    sendResponse(msg, OP_GET_RESPONSE, response);
    return;
  }

  if (storeValue.get()) {
    msg->setAttachment(STORE_ATTACH_VALUE, storeValue.get());
  }

  // call store listener
  sendStoreListener(msg, getResultCode(status));
}

void StoreServiceActor::handleUpdateStore(const ActorMessagePtr& msg) {
  // globe update operation
  UpdateRequest* request = dynamic_cast<UpdateRequest*>(msg->getPayload().get());

  auto& schemaName = request->schema_name();
  auto& storeName = request->store_name();

  DVLOG_FIRST_N(2, 10) << "handle globe update store " << schemaName << "." << storeName;

  StorePtr store;
  auto datastore = idgs_store_module()->getDataStore();
  if (request->has_schema_name()) {
    store = datastore->getStore(schemaName, storeName);
  } else {
    store = datastore->getStore(storeName);
  }

  if (!store) {
    LOG(ERROR)<< "Update data to store " << schemaName << "." << storeName << " failed, caused by store not found.";
    shared_ptr<UpdateResponse> response = make_shared<UpdateResponse>();
    response->set_result_code(SRC_STORE_NOT_FOUND);
    sendResponse(msg, OP_UPDATE_RESPONSE, response);
    return;
  }

  auto& storeConfig = store->getStoreConfig();

  shared_ptr<Message> key = storeConfig->newKey();
  if (!msg->parseAttachment(STORE_ATTACH_KEY, key.get())) {
    LOG(ERROR)<< "Update data to store " << schemaName << "." << storeName << " failed, caused by invalid key";
    shared_ptr<UpdateResponse> response = make_shared<UpdateResponse>();
    response->set_result_code(SRC_INVALID_KEY);
    sendResponse(msg, OP_UPDATE_RESPONSE, response);
    return;
  }

  shared_ptr<Message> value = storeConfig->newValue();
  if (!msg->parseAttachment(STORE_ATTACH_VALUE, value.get())) {
    LOG(ERROR)<< "Update data to store " << schemaName << "." << storeName << " failed, caused by invalid value";
    shared_ptr<UpdateResponse> response = make_shared<UpdateResponse>();
    response->set_result_code(SRC_INVALID_VALUE);
    sendResponse(msg, OP_UPDATE_RESPONSE, response);
    return;
  }

  // calculate partition id and whether need to route message to right member
  calcStorePartition<pb::UpdateRequest>(request, key, storeConfig.get());
  auto cluster = idgs_application()->getClusterFramework();
  auto partition = cluster->getPartitionManager()->getPartition(request->partition_id());
  int32_t destMemberId = partition->getPrimaryMemberId();
  if (destMemberId == -1) {
    LOG(ERROR)<< "Update data to store " << schemaName << "." << storeName << " failed, caused by partition not ready";
    shared_ptr<UpdateResponse> response = make_shared<UpdateResponse>();
    response->set_result_code(SRC_PARTITION_NOT_READY);
    sendResponse(msg, OP_UPDATE_RESPONSE, response);
    return;
  }

  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
  if (destMemberId != localMemberId) {
    // need route message
    DVLOG_FIRST_N(2, 20) << "partition " << request->partition_id() << ", route message from member " << localMemberId << " to member " << destMemberId;
    ActorMessagePtr message = msg->createRouteMessage(destMemberId, msg->getDestActorId(), true);
    idgs::actor::sendMessage(message);
    return;
  }

  // store is partition table
  if (storeConfig->getStoreConfig().partition_type() == pb::PARTITION_TABLE) {
    // update data from local member
    ActorMessagePtr message = const_cast<const ActorMessagePtr&>(msg);
    message->setOperationName(OP_INTERNAL_UPDATE);
    handleLocalUpdate(message);
    auto state = partition->getMemberState(localMemberId);
    if (state == idgs::pb::PS_SOURCE) {
      addToRedoLog(store, request->partition_id(), OP_INTERNAL_UPDATE, key, value);
    }
  // store is replicated
  } else if (storeConfig->getStoreConfig().partition_type() == pb::REPLICATED) {
    // create DataAggregatorActor to multicast all members.
    ActorMessagePtr message = const_cast<const ActorMessagePtr&>(msg);
    message->setOperationName(OP_INTERNAL_UPDATE);
    processReplicatedStore(message);
  }
}

void StoreServiceActor::handleLocalUpdate(const ActorMessagePtr& msg) {
  // handle insert operation on local member
  UpdateRequest* request = dynamic_cast<UpdateRequest*>(msg->getPayload().get());

  auto& schemaName = request->schema_name();
  auto& storeName = request->store_name();

  DVLOG_FIRST_N(2, 20) << "starting handle update data to store " << schemaName << "." << storeName << " from local member.";

  auto cluster = idgs_application()->getClusterFramework();
  if (!cluster->getLocalMember()->isLocalStore()) {
    shared_ptr<UpdateResponse> response = make_shared<UpdateResponse>();
    response->set_result_code(pb::SRC_NOT_LOCAL_STORE);
    sendResponse(msg, OP_UPDATE_RESPONSE, response);
    return;
  }

  StorePtr store;
  auto datastore = idgs_store_module()->getDataStore();
  if (request->has_schema_name()) {
    store = datastore->getStore(schemaName, storeName);
  } else {
    store = datastore->getStore(storeName);
  }

  if (!store) {
    LOG(ERROR)<< "Update data to store " << schemaName << "." << storeName << " failed, caused by store not found.";
    shared_ptr<UpdateResponse> response = make_shared<UpdateResponse>();
    response->set_result_code(SRC_STORE_NOT_FOUND);
    sendResponse(msg, OP_UPDATE_RESPONSE, response);
    return;
  }

  auto& storeConfig = store->getStoreConfig();

  shared_ptr<Message> key = storeConfig->newKey();
  if (!msg->parseAttachment(STORE_ATTACH_KEY, key.get())) {
    LOG(ERROR)<< "Update data to store " << schemaName << "." << storeName << " failed, caused by invalid key";
    shared_ptr<UpdateResponse> response = make_shared<UpdateResponse>();
    response->set_result_code(SRC_INVALID_KEY);
    sendResponse(msg, OP_UPDATE_RESPONSE, response);
    return;
  }

  shared_ptr<Message> value = storeConfig->newValue();
  if (!msg->parseAttachment(STORE_ATTACH_VALUE, value.get())) {
    LOG(ERROR)<< "Update data to store " << schemaName << "." << storeName << " failed, caused by invalid value";
    shared_ptr<UpdateResponse> response = make_shared<UpdateResponse>();
    response->set_result_code(SRC_INVALID_VALUE);
    sendResponse(msg, OP_UPDATE_RESPONSE, response);
    return;
  }

  // handle update

  // calculate partition id and whether need to route message to right member
  calcStorePartition<pb::UpdateRequest>(request, key, storeConfig.get());

  idgs::store::StoreOption ps;
  ps.partitionId = request->partition_id();
  StoreValue<Message> storeValue(value);
  ResultCode status = store->put(key, storeValue, &ps);
  if (status != RC_SUCCESS) {
    LOG(ERROR)<< "Update data to store " << schemaName << "." << storeName << " failed, " << getErrorDescription(status);
    shared_ptr<UpdateResponse> response = make_shared<UpdateResponse>();
    response->set_result_code(getResultCode(status));
    sendResponse(msg, OP_UPDATE_RESPONSE, response);
    return;
  }

  // call store listener
  if (request->options() & RETRIEVE_PREVIOUS) {
    if (!storeValue.get()) {
      msg->setAttachment(STORE_ATTACH_LAST_VALUE, storeValue.get());
    }
  }

  if (storeConfig->getStoreConfig().partition_type() == pb::REPLICATED) {
    auto primaryMemberId = cluster->getPartitionManager()->getPartition(request->partition_id())->getPrimaryMemberId();
    auto localMemberId = cluster->getMemberManager()->getLocalMemberId();
    if (primaryMemberId != localMemberId) {
      shared_ptr<UpdateResponse> response = make_shared<UpdateResponse>();
      response->set_result_code(pb::SRC_SUCCESS);
      sendResponse(msg, OP_UPDATE_RESPONSE, response);
      return;
    }
  }

  sendStoreListener(msg);
}

void StoreServiceActor::handleDeleteStore(const ActorMessagePtr& msg) {
  // globe delete operation
  DeleteRequest* request = dynamic_cast<DeleteRequest*>(msg->getPayload().get());

  auto& schemaName = request->schema_name();
  auto& storeName = request->store_name();

  DVLOG_FIRST_N(2, 10) << "handle globe delete data from store " << schemaName << "." << storeName;

  StorePtr store;
  auto datastore = idgs_store_module()->getDataStore();
  if (request->has_schema_name()) {
    store = datastore->getStore(schemaName, storeName);
  } else {
    store = datastore->getStore(storeName);
  }

  if (!store) {
    LOG(ERROR)<< "Delete data from store " << schemaName << "." << storeName << " failed, caused by store not found.";
    shared_ptr<DeleteResponse> response = make_shared<DeleteResponse>();
    response->set_result_code(SRC_STORE_NOT_FOUND);
    sendResponse(msg, OP_DELETE_RESPONSE, response);
    return;
  }

  auto& storeConfig = store->getStoreConfig();

  shared_ptr<Message> key = storeConfig->newKey();
  if (!msg->parseAttachment(STORE_ATTACH_KEY, key.get())) {
    LOG(ERROR)<< "Delete data from store " << schemaName << "." << storeName << " failed, caused by invalid key";
    shared_ptr<DeleteResponse> response = make_shared<DeleteResponse>();
    response->set_result_code(SRC_INVALID_KEY);
    sendResponse(msg, OP_DELETE_RESPONSE, response);
    return;
  }

  // calculate partition id and whether need to route message to right member
  calcStorePartition<pb::DeleteRequest>(request, key, storeConfig.get());
  auto cluster = idgs_application()->getClusterFramework();
  auto partition = cluster->getPartitionManager()->getPartition(request->partition_id());
  int32_t destMemberId = partition->getPrimaryMemberId();
  if (destMemberId == -1) {
    LOG(ERROR)<< "Delete data from store " << schemaName << "." << storeName << " failed, caused by partition not ready";
    shared_ptr<DeleteResponse> response = make_shared<DeleteResponse>();
    response->set_result_code(SRC_PARTITION_NOT_READY);
    sendResponse(msg, OP_DELETE_RESPONSE, response);
    return;
  }

  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
  if (destMemberId != localMemberId) {
    // need route message
    DVLOG_FIRST_N(2, 20) << "partition " << request->partition_id() << ", route message from member " << localMemberId << " to member " << destMemberId;
    ActorMessagePtr message = msg->createRouteMessage(destMemberId, msg->getDestActorId(), true);
    idgs::actor::sendMessage(message);
    return;
  }

  // store is partition table
  if (storeConfig->getStoreConfig().partition_type() == pb::PARTITION_TABLE) {
    // delete data from local member
    ActorMessagePtr message = const_cast<const ActorMessagePtr&>(msg);
    message->setOperationName(OP_INTERNAL_DELETE);
    handleLocalDelete(message);
    auto state = partition->getMemberState(localMemberId);
    if (state == idgs::pb::PS_SOURCE) {
      PbMessagePtr value;
      addToRedoLog(store, request->partition_id(), OP_INTERNAL_DELETE, key, value);
    }
  // store is replicated
  } else if (storeConfig->getStoreConfig().partition_type() == pb::REPLICATED) {
    // create DataAggregatorActor to multicast all members.
    ActorMessagePtr message = const_cast<const ActorMessagePtr&>(msg);
    message->setOperationName(OP_INTERNAL_DELETE);
    processReplicatedStore(message);
  }
}

void StoreServiceActor::handleLocalDelete(const ActorMessagePtr& msg) {
  // handle insert operation on local member
  DeleteRequest* request = dynamic_cast<DeleteRequest*>(msg->getPayload().get());

  auto& schemaName = request->schema_name();
  auto& storeName = request->store_name();

  DVLOG_FIRST_N(2, 20) << "starting handle delete data to store " << schemaName << "." << storeName << " from local member.";

  auto cluster = idgs_application()->getClusterFramework();
  if (!cluster->getLocalMember()->isLocalStore()) {
    shared_ptr<DeleteResponse> response = make_shared<DeleteResponse>();
    response->set_result_code(pb::SRC_NOT_LOCAL_STORE);
    sendResponse(msg, OP_DELETE_RESPONSE, response);
    return;
  }

  StorePtr store;
  auto datastore = idgs_store_module()->getDataStore();
  if (request->has_schema_name()) {
    store = datastore->getStore(schemaName, storeName);
  } else {
    store = datastore->getStore(storeName);
  }

  if (!store) {
    LOG(ERROR)<< "Delete data from store " << schemaName << "." << storeName << " failed, caused by store not found.";
    shared_ptr<DeleteResponse> response = make_shared<DeleteResponse>();
    response->set_result_code(SRC_STORE_NOT_FOUND);
    sendResponse(msg, OP_DELETE_RESPONSE, response);
    return;
  }

  auto& storeConfig = store->getStoreConfig();

  shared_ptr<Message> key = storeConfig->newKey();
  if (!msg->parseAttachment(STORE_ATTACH_KEY, key.get())) {
    LOG(ERROR)<< "Delete data from store " << schemaName << "." << storeName << " failed, caused by invalid key";
    shared_ptr<DeleteResponse> response = make_shared<DeleteResponse>();
    response->set_result_code(SRC_INVALID_KEY);
    sendResponse(msg, OP_DELETE_RESPONSE, response);
    return;
  }

  // calculate partition id and whether need to route message to right member
  calcStorePartition<pb::DeleteRequest>(request, key, storeConfig.get());

  // handle update
  idgs::store::StoreOption ps;
  ps.partitionId = request->partition_id();
  shared_ptr<Message> value;
  StoreValue<Message> storeValue(value);
  ResultCode status = store->remove(key, storeValue, &ps);
  if (status != RC_SUCCESS) {
    LOG(ERROR)<< "Delete data from store " << schemaName << "." << storeName << " by key " << key->DebugString() << " failed, " << getErrorDescription(status);
    shared_ptr<DeleteResponse> response = make_shared<DeleteResponse>();
    response->set_result_code(getResultCode(status));
    sendResponse(msg, OP_DELETE_RESPONSE, response);
    return;
  }

  if (request->options() & RETRIEVE_PREVIOUS) {
    if (!storeValue.get()) {
      msg->setAttachment(STORE_ATTACH_LAST_VALUE, storeValue.get());
    }
  }

  if (storeConfig->getStoreConfig().partition_type() == pb::REPLICATED) {
    auto primaryMemberId = cluster->getPartitionManager()->getPartition(request->partition_id())->getPrimaryMemberId();
    auto localMemberId = cluster->getMemberManager()->getLocalMemberId();
    if (primaryMemberId != localMemberId) {
      shared_ptr<DeleteResponse> response = make_shared<DeleteResponse>();
      response->set_result_code(pb::SRC_SUCCESS);
      sendResponse(msg, OP_DELETE_RESPONSE, response);
      return;
    }
  }

  // call store listener
  sendStoreListener(msg);
}

void StoreServiceActor::processReplicatedStore(const idgs::actor::ActorMessagePtr& msg) {
  DataAggregatorActor* actor = new DataAggregatorActor();
  DVLOG_FIRST_N(2, 20) << "register insert aggregator actor with id " << actor->getActorId();
  idgs_application()->getRpcFramework()->getActorManager()->registerSessionActor(actor->getActorId(), actor);
  ActorMessagePtr routeMsg = const_cast<const ActorMessagePtr&>(msg);
  actor->process(routeMsg);
}

void StoreServiceActor::handleCountStore(const idgs::actor::ActorMessagePtr& msg) {
  SizeRequest* request = dynamic_cast<SizeRequest*>(msg->getPayload().get());

  auto& schemaName = request->schema_name();
  auto& storeName = request->store_name();

  StorePtr store;
  auto datastore = idgs_store_module()->getDataStore();
  if (request->has_schema_name()) {
    store = datastore->getStore(schemaName, storeName);
  } else {
    store = datastore->getStore(storeName);
  }

  if (!store) {
    LOG(ERROR)<< "Count data from store " << schemaName << "." << storeName << " failed, caused by store not found.";
    shared_ptr<SizeResponse> response = make_shared<SizeResponse>();
    response->set_result_code(SRC_STORE_NOT_FOUND);
    sendResponse(msg, OP_COUNT_RESPONSE, response);
    return;
  }

  auto& storeConfigWrapper = store->getStoreConfig();

  // partition table store.
  if (storeConfigWrapper->getStoreConfig().partition_type() == pb::PARTITION_TABLE) {
    DataAggregatorActor* actor = new DataAggregatorActor;
    idgs_application()->getRpcFramework()->getActorManager()->registerSessionActor(actor->getActorId(), actor);
    ActorMessagePtr message = const_cast<const ActorMessagePtr&>(msg);
    message->setOperationName(OP_INTERNAL_COUNT);
    actor->process(message);
    // replicated store.
  } else if (storeConfigWrapper->getStoreConfig().partition_type() == pb::REPLICATED) {
    ActorMessagePtr message = const_cast<const ActorMessagePtr&>(msg);
    message->setOperationName(OP_INTERNAL_COUNT);
    handleLocalCount(message);
  }
}

void StoreServiceActor::handleLocalCount(const ActorMessagePtr& msg) {
  DVLOG_FIRST_N(2, 20) << "starting handle count data to store.";
  SizeRequest* request = dynamic_cast<SizeRequest*>(msg->getPayload().get());

  auto& schemaName = request->schema_name();
  auto& storeName = request->store_name();

  shared_ptr<SizeResponse> response = make_shared<SizeResponse>();

  auto cluster = idgs_application()->getClusterFramework();
  if (!cluster->getLocalMember()->isLocalStore()) {
    response->set_result_code(pb::SRC_NOT_LOCAL_STORE);
    sendResponse(msg, OP_COUNT_RESPONSE, response);
    return;
  }

  StorePtr store;
  auto datastore = idgs_store_module()->getDataStore();
  if (request->has_schema_name()) {
    store = datastore->getStore(schemaName, storeName);
  } else {
    store = datastore->getStore(storeName);
  }

  if (!store) {
    LOG(ERROR)<< "Count data from store " << schemaName << "." << storeName << " failed, caused by store not found.";
    response->set_result_code(SRC_STORE_NOT_FOUND);
    sendResponse(msg, OP_COUNT_RESPONSE, response);
    return;
  }

  size_t size = store->size();

  response->set_partition(request->partition());
  response->set_result_code(SRC_SUCCESS);
  response->set_size(size);

  // response result
  sendResponse(msg, OP_COUNT_RESPONSE, response);
}

void StoreServiceActor::sendResponse(const ActorMessagePtr& msg, const string& opName,
    const shared_ptr<Message>& payload, const shared_ptr<Message>& returnValue) {
  // create response message and send back to client
  DVLOG_FIRST_N(2, 20) << "send response to client";
  std::shared_ptr<ActorMessage> am = msg->createResponse();
  am->setOperationName(opName);
  am->setPayload(payload);
  if (am->getChannel() == idgs::pb::TC_MULTICAST) {
    am->setChannel(idgs::pb::TC_AUTO);
  }

  if (returnValue.get() != NULL) {
    DVLOG_FIRST_N(2, 20) << "add return value : " << returnValue->DebugString();
    am->setAttachment(STORE_ATTACH_VALUE, returnValue);
  }

  idgs::actor::sendMessage(am);
}

StoreResultCode StoreServiceActor::getResultCode(const ResultCode& status) {
  DVLOG_FIRST_N(2, 20) << "operate done , code : " << getErrorDescription(status);
  static std::map<ResultCode, StoreResultCode> codeMapping = {
      {RC_SUCCESS,                    SRC_SUCCESS},
      {RC_INVALID_KEY,                SRC_INVALID_KEY},
      {RC_INVALID_VALUE,              SRC_INVALID_VALUE},
      {RC_STORE_NOT_FOUND,            SRC_STORE_NOT_FOUND},
      {RC_PARTITION_NOT_FOUND,        SRC_PARTITION_NOT_FOUND},
      {RC_DATA_NOT_FOUND,             SRC_DATA_NOT_FOUND},
      {RC_PARTITION_NOT_READY,        SRC_PARTITION_NOT_READY},
      {RC_NOT_LOCAL_STORE,            SRC_NOT_LOCAL_STORE}
  };
  auto it = codeMapping.find(status);
  if (it == codeMapping.end()) {
    return SRC_UNKNOWN_ERROR;
  }

  return it->second;
}

void StoreServiceActor::handleTruncateStore(const idgs::actor::ActorMessagePtr& msg) {
  TruncateRequest* request = dynamic_cast<TruncateRequest*>(msg->getPayload().get());

  auto& schemaName = request->schema_name();
  auto& storeName = request->store_name();

  DVLOG_FIRST_N(2, 20) << "data store handle truncate store " << schemaName << "." << storeName;

  StorePtr store;
  auto datastore = idgs_store_module()->getDataStore();
  if (request->has_schema_name()) {
    store = datastore->getStore(schemaName, storeName);
  } else {
    store = datastore->getStore(storeName);
  }

  if (!store) {
    LOG(ERROR) << "This member has no local store";
    shared_ptr<TruncateResponse> response = make_shared<TruncateResponse>();
    response->set_result_code(pb::SRC_STORE_NOT_FOUND);
    sendResponse(msg, OP_TRUNCATE_RESPONSE, response);
    return;
  }

  DataAggregatorActor* actor = new DataAggregatorActor();
  DVLOG_FIRST_N(2, 20) << "register aggregator actor with id " << actor->getActorId();
  idgs_application()->getRpcFramework()->getActorManager()->registerSessionActor(actor->getActorId(), actor);
  ActorMessagePtr message = const_cast<const ActorMessagePtr&>(msg);
  message->setOperationName(OP_INTERNAL_TRUNCATE);
  actor->process(message);
}

void StoreServiceActor::handleLocalTruncate(const idgs::actor::ActorMessagePtr& msg) {
  TruncateRequest* request = dynamic_cast<TruncateRequest*>(msg->getPayload().get());

  auto& schemaName = request->schema_name();
  auto& storeName = request->store_name();

  DVLOG_FIRST_N(2, 20) << "data store actor handle local truncate store:" << schemaName << "." << storeName;

  shared_ptr<TruncateResponse> response = make_shared<TruncateResponse>();
  auto cluster = idgs_application()->getClusterFramework();
  if (!cluster->getLocalMember()->isLocalStore()) {
    LOG(ERROR) << "This member has no local store";
    response->set_result_code(pb::SRC_NOT_LOCAL_STORE);
    sendResponse(msg, OP_TRUNCATE_RESPONSE, response);
    return;
  }

  StorePtr store;
  auto datastore = idgs_store_module()->getDataStore();
  if (request->has_schema_name()) {
    store = datastore->getStore(schemaName, storeName);
  } else {
    store = datastore->getStore(storeName);
  }

  if (!store) {
    LOG(ERROR) << "This member has no local store";
    shared_ptr<TruncateResponse> response = make_shared<TruncateResponse>();
    response->set_result_code(pb::SRC_STORE_NOT_FOUND);
    sendResponse(msg, OP_TRUNCATE_RESPONSE, response);
    return;
  }

  store->removeAll();

  response->set_result_code(SRC_SUCCESS);
  sendResponse(msg, OP_TRUNCATE_RESPONSE, response);
}

void StoreServiceActor::addToRedoLog(StorePtr& store, const int32_t& partition, const string& opName, const PbMessagePtr& key, const PbMessagePtr& value) {
  auto type = store->getStoreConfig()->getStoreConfig().partition_type();
  if (type == PARTITION_TABLE) {
    auto pstore = dynamic_cast<PartitionedStore*>(store.get());
    auto actors = pstore->getMigrationActors();
    auto it = actors.begin() ;
    for (; it != actors.end(); ++ it) {
      auto actor = dynamic_cast<StoreMigrationSourceActor*>(* it);
      if (actor->getPartitionid() == partition) {
        actor->addToRedoLog(opName, key, value);
      }
    }
  } else if (type == REPLICATED){
    auto rstore = dynamic_cast<ReplicatedStore*>(store.get());
    auto actors = rstore->getSyncActors();
    auto it = actors.begin() ;
    for (; it != actors.end(); ++ it) {
      auto actor = dynamic_cast<StoreSyncSourceActor*>(* it);
      actor->addToRedoLog(opName, key, value);
    }
  }
}

void StoreServiceActor::sendStoreListener(const ActorMessagePtr& msg, pb::StoreResultCode code) {
  auto cluster = idgs_application()->getClusterFramework();
  auto localMemberId = cluster->getMemberManager()->getLocalMemberId();

  shared_ptr<pb::StoreListenerInfo> listener = make_shared<pb::StoreListenerInfo>();
  listener->set_result_code(code);
  listener->set_listener_index(0);
  listener->set_primary_member_id(localMemberId);

  ActorMessagePtr listenerMsg = msg->createRouteMessage(localMemberId, LISTENER_MANAGER, true);
  listenerMsg->setAttachment(STORE_ATTACH_LISTENER, listener);
  if (listenerMsg->getChannel() == idgs::pb::TC_MULTICAST) {
    listenerMsg->setChannel(idgs::pb::TC_AUTO);
  }

  DVLOG_FIRST_N(2, 20) << "sending message to handle store listener";
  idgs::actor::sendMessage(listenerMsg);
}

} //namespace store
} // namespace idgs
