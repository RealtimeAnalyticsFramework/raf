
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "data_store_actor.h"

#include "data_store.h"
#include "protobuf/message_helper.h"
#include "idgs/cluster/cluster_framework.h"
#include "aggregator_actor.h"

using namespace idgs::cluster;
using namespace idgs::pb;
using namespace idgs::actor;
using namespace google::protobuf;
using namespace protobuf;

namespace idgs {
namespace store {

StoreServiceActor::StoreServiceActor(const string& actorId) {
  setActorId(actorId);

  descriptor = StoreServiceActor::generateActorDescriptor();
}

StoreServiceActor::~StoreServiceActor() {
}

const idgs::actor::ActorMessageHandlerMap& StoreServiceActor::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {OP_INSERT, static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleInsertStore)},
      {DATA_STORE_LOCAL_INSERT, static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleLocalInsert)},
      {OP_GET, static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleGetStore)},
      {DATA_STORE_LOCAL_GET, static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleLocalGet)},
      {OP_UPDATE, static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleUpdateStore)},
      {DATA_STORE_LOCAL_UPDATE, static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleLocalUpdate)},
      {OP_DELETE, static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleRemoveStore)},
      {DATA_STORE_LOCAL_REMOVE, static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleLocalRemove)},
      {OP_COUNT, static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleCountStore)},
      {DATA_STORE_LOCAL_SIZE, static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleLocalCount)},
      {OP_TRUNCATE, static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleDataClear)},
      {LOCAL_DATA_CLEAR, static_cast<idgs::actor::ActorMessageHandler>(&StoreServiceActor::handleLocalDataClear)}
  };

  return handlerMap;
}

ActorDescriptorPtr StoreServiceActor::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor)
    return descriptor;
  descriptor.reset(new ::ActorDescriptorWrapper);

  descriptor->setName(ACTORID_STORE_SERVCIE);
  descriptor->setDescription("Data store CRUD operation");
  descriptor->setType(AT_STATELESS);

  // option DATA_STORE_INSERT
  ActorOperationDescriporWrapper dataInsert;
  dataInsert.setName(OP_INSERT);
  dataInsert.setDescription("Receive insert data request from client, find the member which the data is on.");
  dataInsert.setPayloadType("idgs.store.pb.InsertRequest");
  descriptor->setInOperation(dataInsert.getName(), dataInsert);
  descriptor->setOutOperation(dataInsert.getName(), dataInsert);

  // option DATA_STORE_LOCAL_INSERT
  ActorOperationDescriporWrapper dataLocalInsert;
  dataLocalInsert.setName(DATA_STORE_LOCAL_INSERT);
  dataLocalInsert.setDescription("Handle insert data operation on local member.");
  dataLocalInsert.setPayloadType("idgs.store.pb.InsertRequest");
  descriptor->setInOperation(dataLocalInsert.getName(), dataLocalInsert);

  ActorOperationDescriporWrapper dataInsertResponse;
  dataInsertResponse.setName(DATA_STORE_INSERT_RESPONSE);
  dataInsertResponse.setDescription("Send insert response data to client");
  dataInsertResponse.setPayloadType("idgs.store.pb.InsertResponse");
  descriptor->setOutOperation(dataInsertResponse.getName(), dataInsertResponse);

  // option DATA_STORE_GET
  ActorOperationDescriporWrapper dataGet;
  dataGet.setName(OP_GET);
  dataGet.setDescription("Receive get data request from client");
  dataGet.setPayloadType("idgs.store.pb.GetRequest");
  descriptor->setInOperation(dataGet.getName(), dataGet);
  descriptor->setOutOperation(dataGet.getName(), dataGet);

  // option DATA_STORE_LOCAL_GET
  ActorOperationDescriporWrapper dataLocalGet;
  dataLocalGet.setName(DATA_STORE_LOCAL_GET);
  dataLocalGet.setDescription("Handle insert operation on local member.");
  dataLocalGet.setPayloadType("idgs.store.pb.GetRequest");
  descriptor->setInOperation(dataLocalGet.getName(), dataLocalGet);

  ActorOperationDescriporWrapper dataGetResponse;
  dataGetResponse.setName(DATA_STORE_GET_RESPONSE);
  dataGetResponse.setDescription("Send get data reponse to client");
  dataGetResponse.setPayloadType("idgs.store.pb.GetResponse");
  descriptor->setOutOperation(dataGetResponse.getName(), dataGetResponse);

  // option DATA_STORE_UPDATE
  ActorOperationDescriporWrapper dataUpdate;
  dataUpdate.setName(OP_UPDATE);
  dataUpdate.setDescription("Receive update data request from client, find the member which the data is on.");
  dataUpdate.setPayloadType("idgs.store.pb.UpdateRequest");
  descriptor->setInOperation(dataUpdate.getName(), dataUpdate);
  descriptor->setOutOperation(dataUpdate.getName(), dataUpdate);

  // option DATA_STORE_LOCAL_UPDATE
  ActorOperationDescriporWrapper dataLocalUpdate;
  dataLocalUpdate.setName(DATA_STORE_LOCAL_UPDATE);
  dataLocalUpdate.setDescription("Handle update data operation on local member.");
  dataLocalUpdate.setPayloadType("idgs.store.pb.UpdateRequest");
  descriptor->setInOperation(dataLocalUpdate.getName(), dataLocalUpdate);

  ActorOperationDescriporWrapper dataUpdateResponse;
  dataUpdateResponse.setName(DATA_STORE_UPDATE_RESPONSE);
  dataUpdateResponse.setDescription("Send update data response to client");
  dataUpdateResponse.setPayloadType("idgs.store.pb.UpdateResponse");
  descriptor->setInOperation(dataUpdateResponse.getName(), dataUpdateResponse);

  // option DATA_STORE_REMOVE
  ActorOperationDescriporWrapper dataRemove;
  dataRemove.setName(OP_DELETE);
  dataRemove.setDescription("Receive remove data request from client, find the member which the data is on.");
  dataRemove.setPayloadType("idgs.store.pb.RemoveRequest");
  descriptor->setInOperation(dataRemove.getName(), dataRemove);
  descriptor->setOutOperation(dataRemove.getName(), dataRemove);

  // option DATA_STORE_LOCAL_REMOVE
  ActorOperationDescriporWrapper dataLocalRemove;
  dataLocalRemove.setName(DATA_STORE_LOCAL_REMOVE);
  dataLocalRemove.setDescription("Handle remove data operation on local member.");
  dataLocalRemove.setPayloadType("idgs.store.pb.RemoveRequest");
  descriptor->setInOperation(dataLocalRemove.getName(), dataLocalRemove);

  ActorOperationDescriporWrapper dataRemoveResponse;
  dataRemoveResponse.setName(DATA_STORE_REMOVE_RESPONSE);
  dataRemoveResponse.setDescription("Send remove data response to client");
  dataRemoveResponse.setPayloadType("idgs.store.pb.RemoveResponse");
  descriptor->setOutOperation(dataRemoveResponse.getName(), dataRemoveResponse);

  // option DATA_STORE_SIZE
  ActorOperationDescriporWrapper dataCount;
  dataCount.setName(OP_COUNT);
  dataCount.setDescription("Receive count request from client, handle insert operation.");
  dataCount.setPayloadType("idgs.store.pb.SizeRequest");
  descriptor->setInOperation(dataCount.getName(), dataCount);
  descriptor->setOutOperation(dataCount.getName(), dataCount);

  // option DATA_STORE_LOCAL_SIZE
  ActorOperationDescriporWrapper dataLocalCount;
  dataLocalCount.setName(DATA_STORE_LOCAL_SIZE);
  dataLocalCount.setDescription("Handle count operation on local member.");
  dataLocalCount.setPayloadType("idgs.store.pb.SizeRequest");
  descriptor->setInOperation(dataLocalCount.getName(), dataLocalCount);

  ActorOperationDescriporWrapper dataSizeResponse;
  dataSizeResponse.setName(DATA_STORE_SIZE_RESPONSE);
  dataSizeResponse.setDescription("Send size data response to client");
  dataSizeResponse.setPayloadType("idgs.store.pb.SizeResponse");
  descriptor->setOutOperation(dataSizeResponse.getName(), dataSizeResponse);

  ActorOperationDescriporWrapper dataClear;
  dataClear.setName(OP_TRUNCATE);
  dataClear.setDescription("clear data of store.");
  dataClear.setPayloadType("idgs.store.pb.DataClearRequest");
  descriptor->setInOperation(dataClear.getName(), dataClear);
  descriptor->setOutOperation(dataClear.getName(), dataClear);

  ActorOperationDescriporWrapper localDataClear;
  localDataClear.setName(LOCAL_DATA_CLEAR);
  localDataClear.setDescription("clear data of store.");
  localDataClear.setPayloadType("idgs.store.pb.DataClearRequest");
  descriptor->setInOperation(localDataClear.getName(), localDataClear);

  ActorOperationDescriporWrapper dataClearResponse;
  dataClearResponse.setName(DATA_CLEAR_RESPONSE);
  dataClearResponse.setDescription("Send clear result to client");
  dataClearResponse.setPayloadType("idgs.store.pb.DataClearResponse");
  descriptor->setOutOperation(dataClearResponse.getName(), dataClearResponse);

  return descriptor;
}

ResultCode StoreServiceActor::checkError(const idgs::actor::ActorMessagePtr& msg, const string& storeName,
    shared_ptr<StoreConfigWrapper>& storeConfig, shared_ptr<Message>& key, shared_ptr<Message>& value,
    const bool& isValueValidated) {
  // whether store is configed.
  ResultCode code = ::idgs::util::singleton<DataStore>::getInstance().loadStoreConfig(storeName, storeConfig);
  if (code != RC_SUCCESS) {
    return code;
  }

  // whether key from request is validated
  MessageHelper& helper = idgs::util::singleton<MessageHelper>::getInstance();
  string keyType = storeConfig->getStoreConfig().key_type();
  if (!helper.isMessageRegistered(keyType)) {
    LOG(ERROR)<< "Message with type " << keyType << " is not registered.";
    return RC_KEY_TYPE_NOT_REGISTERED;
  }

  key = helper.createMessage(keyType);
  if (!msg->parseAttachment(STORE_ATTACH_KEY, key.get())) {
    LOG(ERROR)<< "Cannot find or parse the key attachment.";
    return RC_INVALID_KEY;
  }

  // whether value from request is validated
  if (isValueValidated) {
    // whether value is registered.
    string valueType = storeConfig->getStoreConfig().value_type();
    if (!helper.isMessageRegistered(valueType)) {
      LOG(ERROR)<< "Message with type " << valueType << " is not registered.";
      return RC_VALUE_TYPE_NOT_REGISTERED;
    }

    value = helper.createMessage(valueType);
    if (!msg->parseAttachment(STORE_ATTACH_VALUE, value.get())) {
      LOG(ERROR)<< "Cannot find or parse the value attachment.";
      return RC_INVALID_VALUE;
    }
  }

  // none error
  return RC_SUCCESS;
}

void StoreServiceActor::handleInsertStore(const ActorMessagePtr& msg) {
  // globe insert operation
  idgs::store::pb::InsertRequest* request = dynamic_cast<idgs::store::pb::InsertRequest*>(msg->getPayload().get());
  shared_ptr<idgs::store::pb::InsertResponse> response(new idgs::store::pb::InsertResponse);
  DVLOG(2) << "handle globe insert store " << request->store_name()
              << ", local member ID: " << ::idgs::util::singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();

  LOG_IF(ERROR, ::idgs::util::singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId() < 0) << "local member ID: "
      << ::idgs::util::singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();

  shared_ptr<Message> key;
  shared_ptr<Message> value;
  shared_ptr<StoreConfigWrapper> storeConfig;

  // check error
  ResultCode code = checkError(msg, request->store_name(), storeConfig, key, value, true);

  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "Insert store " << request->store_name() << " failed, " << getErrorDescription(code);
    response->set_result_code(getResultCode(code));
    sendResponse(msg, DATA_STORE_INSERT_RESPONSE, response);
    return;
  }

  // store is partition table
  if (storeConfig->getStoreConfig().partition_type() == pb::PARTITION_TABLE) {
    // calucate its partition
    int32_t partition = -1;
    if (request->has_partition_id() && request->partition_id() != -1) {
      partition = request->partition_id();
    } else {
      code = calcStorePartition(key, partition, storeConfig.get());

      // partition is ready
      if (code != RC_SUCCESS) {
        LOG(ERROR)<< "Insert store " << request->store_name() << " failed, " << getErrorDescription(code);
        response->set_result_code(getResultCode(code));
        sendResponse(msg, DATA_STORE_INSERT_RESPONSE, response);
        return;
      }
    }

    // if the data is not on this member, route message to right member.
    request->set_partition_id(partition);
    if (routeMessage(partition, msg)) {
      return;
    }

    handleLocalInsert(msg);
  } else if (storeConfig->getStoreConfig().partition_type() == pb::REPLICATED) {
    // store is replicated

    // create DataAggregatorActor to multicast all members.
    processReplicatedStore(msg);

    // if the data is not on this member, route message to right member.
//        if (routeMessage(partition, msg)) {
//          return;
//        }
//
//        handleLocalInsert(msg);
  }
}

void StoreServiceActor::handleLocalInsert(const ActorMessagePtr& msg) {
  // handle insert operation on local member
  shared_ptr<idgs::store::pb::InsertResponse> response(new idgs::store::pb::InsertResponse);
  if (!::idgs::util::singleton<ClusterFramework>::getInstance().getLocalMember()->isLocalStore()) {
    DVLOG(1) << "This member has no local store";
    return;
  }

  idgs::store::pb::InsertRequest* request = dynamic_cast<idgs::store::pb::InsertRequest*>(msg->getPayload().get());
  DVLOG(2) << "starting handle insert data to store " << request->store_name() << " from local member.";

  shared_ptr<Message> key;
  shared_ptr<Message> value;
  shared_ptr<StoreConfigWrapper> storeConfig;

  // check whether has error
  ResultCode code = checkError(msg, request->store_name(), storeConfig, key, value, true);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "Insert store " << request->store_name() << " failed, " << getErrorDescription(code);
    response->set_result_code(getResultCode(code));
    sendResponse(msg, DATA_STORE_INSERT_RESPONSE, response);
    return;
  }

  // handle insert
  StoreValue<Message> storeValue(value);
  ResultCode status = RC_SUCCESS;
  if (request->has_partition_id() && request->partition_id() != -1) {
    PartitionStatus ps;
    ps.partitionId = request->partition_id();
    status = ::idgs::util::singleton<DataStore>::getInstance().insertData(request->store_name(), key, storeValue, &ps);
  } else {
    status = ::idgs::util::singleton<DataStore>::getInstance().insertData(request->store_name(), key, storeValue);
  }

  response->set_result_code(getResultCode(status));

  // call store listener
  if (status == RC_SUCCESS) {
    sendStoreListener(OP_INSERT, msg);
  }

  // response result
  DVLOG(2) << "send response";
  sendResponse(msg, DATA_STORE_INSERT_RESPONSE, response);

//      if (storeConfig->getStoreConfig().partition_type() == pb::PARTITION_TABLE) {
//        // response result
//        DVLOG(2) << "send response";
//        sendResponse(msg, DATA_STORE_INSERT_RESPONSE, response);
//      } else if (storeConfig->getStoreConfig().partition_type() == pb::REPLICATED) {
//        // calucate its partition
//        int32_t partition;
//        code = calcStorePartition(key, partition, storeConfig.get());
//
//        int32_t destMemberId = ::idgs::util::singleton<ClusterFramework>::getInstance().getPartitionManager()->getPartition(partition)->getPrimaryMemberId();
//        int32_t localMemberId = ::idgs::util::singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();
//
//        if(destMemberId == localMemberId) {
//          DVLOG(2) << "send response";
//          sendResponse(msg, DATA_STORE_INSERT_RESPONSE, response);
//        }
//      }
}

void StoreServiceActor::handleGetStore(const ActorMessagePtr& msg) {
  // globe get operation
  idgs::store::pb::GetRequest* request = dynamic_cast<idgs::store::pb::GetRequest*>(msg->getPayload().get());
  shared_ptr<idgs::store::pb::GetResponse> response(new idgs::store::pb::GetResponse);
  DVLOG(2) << "starting handle get data to store " << request->store_name() << ".";

  shared_ptr<Message> key;
  shared_ptr<Message> value;
  shared_ptr<StoreConfigWrapper> storeConfig;

  // check error
  ResultCode code = checkError(msg, request->store_name(), storeConfig, key, value, false);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "Get store " << request->store_name() << " failed, caused by " << getErrorDescription(code);
    response->set_result_code(getResultCode(code));
    sendResponse(msg, DATA_STORE_GET_RESPONSE, response);
    return;
  }

  // store is partition table
  if (storeConfig->getStoreConfig().partition_type() == pb::PARTITION_TABLE) {
    // calucate its partition
    int32_t partition = -1;
    if (request->has_partition_id() && request->partition_id() != -1) {
      partition = request->partition_id();
    } else {
      code = calcStorePartition(key, partition, storeConfig.get());
      if (code != RC_SUCCESS) {
        LOG(ERROR)<< "Get store " << request->store_name() << " failed, caused by " << getErrorDescription(code);
        response->set_result_code(getResultCode(code));
        sendResponse(msg, DATA_STORE_GET_RESPONSE, response);
        return;
      }
    }

    // route message to right member
    if (routeMessage(partition, msg)) {
      return;
    }
    // store is replicated
  } else if (storeConfig->getStoreConfig().partition_type() == pb::REPLICATED) {
    // to get data from local member
  }

  handleLocalGet(msg);
}

void StoreServiceActor::handleLocalGet(const idgs::actor::ActorMessagePtr& msg) {
  // handle get operation on local member
  shared_ptr<idgs::store::pb::GetResponse> response(new idgs::store::pb::GetResponse);
  if (!::idgs::util::singleton<ClusterFramework>::getInstance().getLocalMember()->isLocalStore()) {
    DVLOG(1) << "This member has no local store";
    return;
  }

  idgs::store::pb::GetRequest* request = dynamic_cast<idgs::store::pb::GetRequest*>(msg->getPayload().get());
  DVLOG(2) << "starting handle get data to store " << request->store_name() << " from local member.";

  shared_ptr<Message> key;
  shared_ptr<Message> value;
  shared_ptr<StoreConfigWrapper> storeConfig;
  ResultCode code = checkError(msg, request->store_name(), storeConfig, key, value, false);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "Get store " << request->store_name() << " failed, " << getErrorDescription(code);
    response->set_result_code(getResultCode(code));
    sendResponse(msg, DATA_STORE_GET_RESPONSE, response);
    return;
  }

  StoreKey<Message> storeKey(key);
  StoreValue<Message> storeValue(value);

  ResultCode status = RC_SUCCESS;
  if (request->has_partition_id() && request->partition_id() != -1) {
    PartitionStatus ps;
    ps.partitionId = request->partition_id();
    status = ::idgs::util::singleton<DataStore>::getInstance().insertData(request->store_name(), key, storeValue, &ps);
  } else {
    status = ::idgs::util::singleton<DataStore>::getInstance().getData(request->store_name(), storeKey, storeValue);
  }

  response->set_result_code(getResultCode(status));

  // call store listener
//  if (status == RC_SUCCESS) {
//    sendStoreListener(OP_GET, msg);
//  }

  // response result
  sendResponse(msg, DATA_STORE_GET_RESPONSE, response, storeValue.get());
}

void StoreServiceActor::handleUpdateStore(const ActorMessagePtr& msg) {
  // globe update operation
  idgs::store::pb::UpdateRequest* request = dynamic_cast<idgs::store::pb::UpdateRequest*>(msg->getPayload().get());
  shared_ptr<idgs::store::pb::UpdateResponse> response(new idgs::store::pb::UpdateResponse);
  DVLOG(2) << "handle globe update store " << request->store_name();

  shared_ptr<Message> key;
  shared_ptr<Message> value;
  shared_ptr<StoreConfigWrapper> storeConfig;

  // check error
  ResultCode code = checkError(msg, request->store_name(), storeConfig, key, value, true);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "Update store " << request->store_name() << " failed, " << getErrorDescription(code);
    response->set_result_code(getResultCode(code));
    sendResponse(msg, DATA_STORE_UPDATE_RESPONSE, response);
    return;
  }

  // store is partition table
  if (storeConfig->getStoreConfig().partition_type() == pb::PARTITION_TABLE) {
    // calucate its partition
    int32_t partition = -1;
    if (request->has_partition_id() && request->partition_id() != -1) {
      partition = request->partition_id();
    } else {
      code = calcStorePartition(key, partition, storeConfig.get());
      if (code != RC_SUCCESS) {
        LOG(ERROR)<< "Update store " << request->store_name() << " failed, " << getErrorDescription(code);
        response->set_result_code(getResultCode(code));
        sendResponse(msg, DATA_STORE_UPDATE_RESPONSE, response);
        return;
      }
    }

    // if the data is not on this member, route message to right member.
    if (routeMessage(partition, msg)) {
      return;
    }

    handleLocalUpdate(msg);
    // store is replicated
  } else if (storeConfig->getStoreConfig().partition_type() == pb::REPLICATED) {
    // create DataAggregatorActor to multicast all members.
    processReplicatedStore(msg);
  }
}

void StoreServiceActor::handleLocalUpdate(const ActorMessagePtr& msg) {
  // handle insert operation on local member
  shared_ptr<idgs::store::pb::UpdateResponse> response(new idgs::store::pb::UpdateResponse);
  if (!::idgs::util::singleton<ClusterFramework>::getInstance().getLocalMember()->isLocalStore()) {
    DVLOG(1) << "This member has no local store";
    return;
  }

  idgs::store::pb::UpdateRequest* request = dynamic_cast<idgs::store::pb::UpdateRequest*>(msg->getPayload().get());
  DVLOG(2) << "starting handle update data to store " << request->store_name() << " from local member.";

  shared_ptr<Message> key;
  shared_ptr<Message> value;
  shared_ptr<StoreConfigWrapper> storeConfig;

  // check whether has error
  ResultCode code = checkError(msg, request->store_name(), storeConfig, key, value, true);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "Insert store " << request->store_name() << " failed, " << getErrorDescription(code);
    response->set_result_code(getResultCode(code));
    sendResponse(msg, DATA_STORE_UPDATE_RESPONSE, response);
    return;
  }

  // handle update
  StoreKey<Message> storeKey(key);
  StoreValue<Message> storeValue(value);

  ResultCode status = RC_SUCCESS;
  if (request->has_partition_id() && request->partition_id() != -1) {
    PartitionStatus ps;
    ps.partitionId = request->partition_id();
    status = ::idgs::util::singleton<DataStore>::getInstance().insertData(request->store_name(), key, storeValue, &ps);
  } else {
    status = ::idgs::util::singleton<DataStore>::getInstance().updateData(request->store_name(), storeKey, storeValue);
  }

  response->set_result_code(getResultCode(status));

  // call store listener
  if (status == RC_SUCCESS) {
    sendStoreListener(OP_UPDATE, msg);
  }

  // response result
  if (request->options() & idgs::store::pb::RETRIEVE_PREVIOUS) {
    sendResponse(msg, DATA_STORE_UPDATE_RESPONSE, response, storeValue.get());
  } else {
    sendResponse(msg, DATA_STORE_UPDATE_RESPONSE, response);
  }
}

void StoreServiceActor::handleRemoveStore(const ActorMessagePtr& msg) {
  // globe remove operation
  idgs::store::pb::RemoveRequest* request = dynamic_cast<idgs::store::pb::RemoveRequest*>(msg->getPayload().get());
  shared_ptr<idgs::store::pb::RemoveResponse> response(new idgs::store::pb::RemoveResponse);
  DVLOG(2) << "handle globe remove store " << request->store_name();

  shared_ptr<Message> key;
  shared_ptr<Message> value;
  shared_ptr<StoreConfigWrapper> storeConfig;

  // check error
  ResultCode code = checkError(msg, request->store_name(), storeConfig, key, value, false);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "Update store " << request->store_name() << " failed, " << getErrorDescription(code);
    response->set_result_code(getResultCode(code));
    sendResponse(msg, DATA_STORE_REMOVE_RESPONSE, response);
    return;
  }

  // store is partition table
  if (storeConfig->getStoreConfig().partition_type() == pb::PARTITION_TABLE) {
    // calucate its partition
    int32_t partition = -1;
    if (request->has_partition_id() && request->partition_id() != -1) {
      partition = request->partition_id();
    } else {
      code = calcStorePartition(key, partition, storeConfig.get());
      if (code != RC_SUCCESS) {
        LOG(ERROR)<< "Update store " << request->store_name() << " failed, " << getErrorDescription(code);
        response->set_result_code(getResultCode(code));
        sendResponse(msg, DATA_STORE_REMOVE_RESPONSE, response);
        return;
      }
    }

    // if the data is not on this member, route message to right member.
    if (routeMessage(partition, msg)) {
      return;
    }

    handleLocalRemove(msg);
    // store is replicated
  } else if (storeConfig->getStoreConfig().partition_type() == pb::REPLICATED) {
    // create DataAggregatorActor to multicast all members.
    processReplicatedStore(msg);
  }
}

void StoreServiceActor::handleLocalRemove(const ActorMessagePtr& msg) {
  // handle insert operation on local member
  shared_ptr<idgs::store::pb::RemoveResponse> response(new idgs::store::pb::RemoveResponse);
  if (!::idgs::util::singleton<ClusterFramework>::getInstance().getLocalMember()->isLocalStore()) {
    DVLOG(1) << "This member has no local store";
    return;
  }

  idgs::store::pb::RemoveRequest* request = dynamic_cast<idgs::store::pb::RemoveRequest*>(msg->getPayload().get());
  DVLOG(2) << "starting handle remove data to store " << request->store_name() << " from local member.";

  shared_ptr<Message> key;
  shared_ptr<Message> value;
  shared_ptr<StoreConfigWrapper> storeConfig;

  // check whether has error
  ResultCode code = checkError(msg, request->store_name(), storeConfig, key, value, false);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "Insert store " << request->store_name() << " failed, " << getErrorDescription(code);
    response->set_result_code(getResultCode(code));
    sendResponse(msg, DATA_STORE_REMOVE_RESPONSE, response);
    return;
  }

  // handle update
  StoreKey<Message> storeKey(key);
  StoreValue<Message> storeValue(value);

  ResultCode status = RC_SUCCESS;
  if (request->has_partition_id() && request->partition_id() != -1) {
    PartitionStatus ps;
    ps.partitionId = request->partition_id();
    status = ::idgs::util::singleton<DataStore>::getInstance().insertData(request->store_name(), key, storeValue, &ps);
  } else {
    status = ::idgs::util::singleton<DataStore>::getInstance().removeData(request->store_name(), storeKey, storeValue);
  }

  response->set_result_code(getResultCode(status));

  // call store listener
  if (status == RC_SUCCESS) {
    sendStoreListener(OP_DELETE, msg);
  }

  // response result
  if (request->options() & idgs::store::pb::RETRIEVE_PREVIOUS) {
    sendResponse(msg, DATA_STORE_REMOVE_RESPONSE, response, storeValue.get());
  } else {
    sendResponse(msg, DATA_STORE_REMOVE_RESPONSE, response);
  }
}

void StoreServiceActor::processReplicatedStore(const idgs::actor::ActorMessagePtr& msg) {
  DataAggregatorActor* actor = new DataAggregatorActor();
  DVLOG(2) << "register insert aggregator actor with id " << actor->getActorId();
  ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework()->Register(actor->getActorId(), actor);
  ActorMessagePtr routeMsg = const_cast<const ActorMessagePtr&>(msg);
  actor->process(routeMsg);
}

void StoreServiceActor::handleCountStore(const idgs::actor::ActorMessagePtr& msg) {
  idgs::store::pb::SizeRequest* request = dynamic_cast<idgs::store::pb::SizeRequest*>(msg->getPayload().get());

  shared_ptr<StoreConfigWrapper> storeConfigWrapper;
  ::idgs::util::singleton<DataStore>::getInstance().loadStoreConfig(request->store_name(), storeConfigWrapper);

  // partition table store.
  if (storeConfigWrapper->getStoreConfig().partition_type() == pb::PARTITION_TABLE) {
    DataSizeAggregatorActor* actor = new DataSizeAggregatorActor;
    ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework()->Register(actor->getActorId(), actor);
    ActorMessagePtr routeMsg = const_cast<const ActorMessagePtr&>(msg);
    actor->process(routeMsg);
    // replicated store.
  } else if (storeConfigWrapper->getStoreConfig().partition_type() == pb::REPLICATED) {
    handleLocalCount(msg);
  }
}

void StoreServiceActor::handleLocalCount(const ActorMessagePtr& msg) {
  DVLOG(2) << "starting handle count data to store.";
  shared_ptr<idgs::store::pb::SizeResponse> response(new idgs::store::pb::SizeResponse);
  if (!::idgs::util::singleton<ClusterFramework>::getInstance().getLocalMember()->isLocalStore()) {
    DVLOG(1) << "This member has no local store";
    return;
  }

  idgs::store::pb::SizeRequest* request = dynamic_cast<idgs::store::pb::SizeRequest*>(msg->getPayload().get());

  size_t size = 0;
  ResultCode status = ::idgs::util::singleton<DataStore>::getInstance().storeSize(request->store_name(), request->partition(), size);

  response->set_partition(request->partition());
  response->set_result_code(getResultCode(status));
  response->set_size(size);

  // response result
  sendResponse(msg, DATA_STORE_SIZE_RESPONSE, response);
}

void StoreServiceActor::sendResponse(const ActorMessagePtr& msg, const string& opName,
    const shared_ptr<Message>& payload, const shared_ptr<Message>& returnValue) {
  // create response message and send back to client
  DVLOG(2) << "send response to client";
  std::shared_ptr<ActorMessage> am = msg->createResponse();
  am->setOperationName(opName);
  am->setPayload(payload);
  if (am->getChannel() == TC_MULTICAST) {
    am->setChannel(TC_AUTO);
  }

  if (returnValue.get() != NULL) {
    DVLOG(2) << "add return value : " << returnValue->DebugString();
    am->setAttachment(STORE_ATTACH_VALUE, returnValue);
  }

  idgs::actor::sendMessage(am);
}

idgs::store::pb::StoreResultCode StoreServiceActor::getResultCode(const ResultCode& status) {
  DVLOG(2) << "operate done , code : " << getErrorDescription(status);
  switch (status) {
  case RC_SUCCESS:
    return idgs::store::pb::SRC_SUCCESS;
  case RC_INVALID_KEY:
    return idgs::store::pb::SRC_INVALID_KEY;
  case RC_INVALID_VALUE:
    return idgs::store::pb::SRC_INVALID_VALUE;
  case RC_STORE_NOT_FOUND:
    return idgs::store::pb::SRC_TABLE_NOT_EXIST;
  case RC_PARTITION_NOT_FOUND:
    return idgs::store::pb::SRC_PARTITION_NOT_FOUND;
  case RC_DATA_NOT_FOUND:
    return idgs::store::pb::SRC_DATA_NOT_FOUND;
  case RC_PARTITION_NOT_READY:
    return idgs::store::pb::SRC_PARTITION_NOT_READY;
  case RC_NOT_LOCAL_STORE:
    return idgs::store::pb::SRC_NOT_LOCAL_STORE;
  default:
    return idgs::store::pb::SRC_UNKNOWN_ERROR;
  }
}

ResultCode StoreServiceActor::calcStorePartition(const shared_ptr<Message>& key, int32_t& partition, StoreConfigWrapper* storeConfig) {
  PartitionStatus ps;
  ResultCode rc = storeConfig->calculatePartitionStatus(key, &ps);
  partition = ps.partitionId;
  return rc;
}

bool StoreServiceActor::routeMessage(const int32_t& partition, const ActorMessagePtr& msg) {
  ClusterFramework& cluster = idgs::util::singleton<ClusterFramework>::getInstance();
  int32_t destMemberId = cluster.getPartitionManager()->getPartition(partition)->getPrimaryMemberId();
  int32_t localMemberId = cluster.getMemberManager()->getLocalMemberId();

  DVLOG(10) << cluster.getPartitionManager()->toString() << endl;
  DVLOG(10) << partition << endl;

  if (destMemberId != localMemberId) {
    // need route message
    DVLOG(2) << "partition " << partition << ", route message from member " << localMemberId << " to member " << destMemberId;
    ActorMessagePtr message = msg->createRouteMessage(destMemberId, msg->getDestActorId());

    idgs::actor::sendMessage(message);
    return true;
  }

  return false;
}

void StoreServiceActor::handleDataClear(const idgs::actor::ActorMessagePtr& msg) {
  DVLOG(2) << "data store handle data clear";
  DataAggregatorActor* actor = new DataAggregatorActor();
  DVLOG(2) << "register insert aggregator actor with id " << actor->getActorId();
  ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework()->Register(actor->getActorId(), actor);
  ActorMessagePtr routeMsg = const_cast<const ActorMessagePtr&>(msg);
  actor->process(routeMsg);
}

void StoreServiceActor::handleLocalDataClear(const idgs::actor::ActorMessagePtr& msg) {
  shared_ptr<idgs::store::pb::DataClearResponse> response(new idgs::store::pb::DataClearResponse);
  if (!::idgs::util::singleton<ClusterFramework>::getInstance().getLocalMember()->isLocalStore()) {
    DVLOG(1) << "This member has no local store";
    return;
  }

  DVLOG(2) << "aggregator handle local data clear";
  idgs::store::pb::DataClearRequest* request = dynamic_cast<idgs::store::pb::DataClearRequest*>(msg->getPayload().get());
  DataStore& store = ::idgs::util::singleton<DataStore>::getInstance();
  ResultCode code = RC_SUCCESS;
  switch (request->mode()) {
    case idgs::store::pb::CLEAR_ALL: {
      auto storeNames = store.getAllStoreNames();
      for (size_t i = 0; i < storeNames.size(); ++i) {
        code = store.clearData(storeNames[i], request->partition());
        if (code != RC_SUCCESS) {
          break;
        }

        // call store listener
        sendStoreListener(OP_TRUNCATE, msg);
      }
      break;
    }
    case idgs::store::pb::CLEAR_BY_STORE_NAME: {
      code = store.clearData(request->store_name(), request->partition());
      if (code == RC_SUCCESS) {
        // call store listener
        sendStoreListener(OP_TRUNCATE, msg);
      }
      break;
    }
    default: {
      return;
    }
  }

  response->set_result_code(getResultCode(code));
  response->set_partition(request->partition());

  sendResponse(msg, DATA_CLEAR_RESPONSE, response);
}

void StoreServiceActor::sendStoreListener(const string& operationName, const ActorMessagePtr& msg) {
  ClusterFramework& cluster = idgs::util::singleton<ClusterFramework>::getInstance();
  auto localMemberId = cluster.getMemberManager()->getLocalMemberId();

  shared_ptr<pb::StoreListenerInfo> listener = make_shared<pb::StoreListenerInfo>();
  listener->set_listener_index(0);

  ActorMessagePtr listenerMsg = msg->createRouteMessage(localMemberId, LISTENER_MANAGER);
  listenerMsg->setOperationName(operationName);
  listenerMsg->setAttachment(STORE_ATTACH_LISTENER, listener);
  if (msg->getChannel() == TC_MULTICAST) {
    msg->setChannel(TC_AUTO);
  }

  idgs::actor::sendMessage(listenerMsg);
}

} //namespace store
} // namespace idgs
