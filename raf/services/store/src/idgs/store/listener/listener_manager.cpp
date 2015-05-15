/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/

#include "listener_manager.h"

#include "idgs/actor/rpc_framework.h"

using namespace std;
using namespace idgs::actor;
using namespace idgs::store::pb;

namespace idgs {
namespace store {

ListenerManager::ListenerManager(const string& actorId) {
  setActorId(actorId);

  descriptor = ListenerManager::generateActorDescriptor();
}

ListenerManager::~ListenerManager() {
}

ActorDescriptorPtr ListenerManager::generateActorDescriptor() {
  static shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = make_shared<ActorDescriptorWrapper>();

  descriptor->setName(ACTORID_STORE_SERVCIE);
  descriptor->setDescription("Store listener manager");
  descriptor->setType(idgs::pb::AT_STATELESS);

  // option OP_INSERT
  ActorOperationDescriporWrapper opInsert;
  opInsert.setName(OP_INTERNAL_INSERT);
  opInsert.setDescription("handle listener after insert operation");
  opInsert.setPayloadType("idgs.store.pb.InsertRequest");
  descriptor->setInOperation(opInsert.getName(), opInsert);
  descriptor->setOutOperation(opInsert.getName(), opInsert);

  // option OP_UPDATE
  ActorOperationDescriporWrapper opUpdate;
  opUpdate.setName(OP_INTERNAL_UPDATE);
  opUpdate.setDescription("handle listener after update operation");
  opUpdate.setPayloadType("idgs.store.pb.UpdateRequest");
  descriptor->setInOperation(opUpdate.getName(), opUpdate);
  descriptor->setOutOperation(opUpdate.getName(), opUpdate);

  // option OP_GET
  ActorOperationDescriporWrapper opGet;
  opGet.setName(OP_INTERNAL_GET);
  opGet.setDescription("handle listener after get operation");
  opGet.setPayloadType("idgs.store.pb.GetRequest");
  descriptor->setInOperation(opGet.getName(), opGet);
  descriptor->setOutOperation(opGet.getName(), opGet);

  // option OP_DELETE
  ActorOperationDescriporWrapper opDelete;
  opDelete.setName(OP_INTERNAL_DELETE);
  opDelete.setDescription("handle listener after delete operation");
  opDelete.setPayloadType("idgs.store.pb.DeleteRequest");
  descriptor->setInOperation(opDelete.getName(), opDelete);
  descriptor->setOutOperation(opDelete.getName(), opDelete);

  // option OP_TRUNCATE
  ActorOperationDescriporWrapper opTruncate;
  opTruncate.setName(OP_INTERNAL_TRUNCATE);
  opTruncate.setDescription("handle listener after truncate operation");
  opTruncate.setPayloadType("idgs.store.pb.TruncateRequest");
  descriptor->setInOperation(opTruncate.getName(), opTruncate);
  descriptor->setOutOperation(opTruncate.getName(), opTruncate);

  return descriptor;
}

const ActorMessageHandlerMap& ListenerManager::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {OP_INTERNAL_INSERT,       static_cast<ActorMessageHandler>(&ListenerManager::handleListenerInsert)},
      {OP_INTERNAL_UPDATE,       static_cast<ActorMessageHandler>(&ListenerManager::handleListenerUpdate)},
      {OP_INTERNAL_GET,          static_cast<ActorMessageHandler>(&ListenerManager::handleListenerGet)},
      {OP_INTERNAL_DELETE,       static_cast<ActorMessageHandler>(&ListenerManager::handleListenerDelete)},
      {OP_INTERNAL_TRUNCATE,     static_cast<ActorMessageHandler>(&ListenerManager::handleListenerTruncate)}
  };

  return handlerMap;
}

void ListenerManager::setDataStore(DataStore* dataStore) {
  datastore = dataStore;
}

void ListenerManager::handleListenerInsert(const ActorMessagePtr& msg) {
  DVLOG_FIRST_N(2, 20) << "handle insert operation of store listener";
  handleListener<InsertRequest, InsertResponse>(msg, OP_INSERT_RESPONSE);
}

void ListenerManager::handleListenerUpdate(const ActorMessagePtr& msg) {
  DVLOG_FIRST_N(2, 20) << "handle update operation of store listener";
  handleListener<UpdateRequest, UpdateResponse>(msg, OP_UPDATE_RESPONSE);
}

void ListenerManager::handleListenerGet(const ActorMessagePtr& msg) {
  DVLOG_FIRST_N(2, 20) << "handle get operation of store listener";
  handleListener<GetRequest, GetResponse>(msg, OP_GET_RESPONSE);
}

void ListenerManager::handleListenerDelete(const ActorMessagePtr& msg) {
  DVLOG_FIRST_N(2, 20) << "handle delete operation of store listener";
  handleListener<DeleteRequest, DeleteResponse>(msg, OP_DELETE_RESPONSE);
}

void ListenerManager::handleListenerTruncate(const ActorMessagePtr& msg) {
  DVLOG_FIRST_N(2, 20) << "handle truncate operation of store listener";
  handleListener<TruncateRequest, TruncateResponse>(msg, OP_TRUNCATE_RESPONSE);
}

StoreResultCode ListenerManager::parseKeyValue(const ActorMessagePtr& msg, const StoreConfigWrapperPtr& config, PbMessagePtr& key, PbMessagePtr& value, PbMessagePtr& rawValue) {
  if (msg->containAttachment(STORE_ATTACH_KEY)) {
    key = config->newKey();
    if (!msg->parseAttachment(STORE_ATTACH_KEY, key.get())) {
      LOG(ERROR) << "listener manager parsed key is invalid";
      return SRC_INVALID_KEY;
    }
  }

  if (msg->containAttachment(STORE_ATTACH_VALUE)) {
    value = config->newValue();
    if (!msg->parseAttachment(STORE_ATTACH_VALUE, value.get())) {
      LOG(ERROR) << "listener manager parsed value is invalid";
      return SRC_INVALID_VALUE;
    }

    rawValue.reset(value->New());
    rawValue->CopyFrom(*value);
  }

  return SRC_SUCCESS;
}

void ListenerManager::sendResponse(const ActorMessagePtr& msg, const string& opName, const PbMessagePtr& payload, ListenerContext& ctx) {
  // create response message and send back to client
  auto am = msg->createResponse();
  am->setOperationName(opName);
  am->setPayload(payload);

  auto code = ctx.getResultCode();
  if (code == SRC_SUCCESS) {
    if (opName == OP_GET_RESPONSE) {
      auto v = ctx.getValue();
      if (v && (* v)) {
        am->setAttachment(STORE_ATTACH_VALUE, * v);
      }
    }

    auto lv = ctx.getLastValue();
    if (lv && (* lv)) {
      am->setAttachment(STORE_ATTACH_LAST_VALUE, * lv);
    }

    auto vv = ctx.getVersionValue();
    if (vv && (* vv)) {
      am->setAttachment(STORE_ATTACH_VERSION_VALUE, * vv);
    }
  } else {
    LOG(WARNING) << "operator " << opName << " failed, caused by " << idgs::store::pb::StoreResultCode_Name(code);
  }

  DVLOG_FIRST_N(2, 20) << "listener process done send response";
  sendMessage(am);
}

} /* namespace store */
} /* namespace idgs */
