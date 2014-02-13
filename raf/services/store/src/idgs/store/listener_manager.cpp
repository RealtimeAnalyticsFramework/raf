/*
 * listener_manager.cpp
 *
 *  Created on: Feb 11, 2014
 *      Author: root
 */

#include "listener_manager.h"

#include "idgs/idgslogging.h"
#include "idgs/util/singleton.h"
#include "idgs/store/data_store.h"
#include "idgs/store/store_listener.h"

namespace idgs {
namespace store {

ListenerManager::ListenerManager(const std::string& actorId) {
  setActorId(actorId);

  descriptor = ListenerManager::generateActorDescriptor();
}

ListenerManager::~ListenerManager() {
}

idgs::actor::ActorDescriptorPtr ListenerManager::generateActorDescriptor() {
  static std::shared_ptr<idgs::actor::ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor.reset(new idgs::actor::ActorDescriptorWrapper);

  descriptor->setName(ACTORID_STORE_SERVCIE);
  descriptor->setDescription("Store listener manager");
  descriptor->setType(idgs::pb::AT_STATELESS);

  // option OP_INSERT
  idgs::actor::ActorOperationDescriporWrapper opInsert;
  opInsert.setName(OP_INSERT);
  opInsert.setDescription("handle listener after insert operation");
  opInsert.setPayloadType("idgs.store.pb.InsertRequest");
  descriptor->setInOperation(opInsert.getName(), opInsert);
  descriptor->setOutOperation(opInsert.getName(), opInsert);

  // option OP_UPDATE
  idgs::actor::ActorOperationDescriporWrapper opUpdate;
  opUpdate.setName(OP_UPDATE);
  opUpdate.setDescription("handle listener after update operation");
  opUpdate.setPayloadType("idgs.store.pb.UpdateRequest");
  descriptor->setInOperation(opUpdate.getName(), opUpdate);
  descriptor->setOutOperation(opUpdate.getName(), opUpdate);

  // option OP_GET
  idgs::actor::ActorOperationDescriporWrapper opGet;
  opGet.setName(OP_GET);
  opGet.setDescription("handle listener after get operation");
  opGet.setPayloadType("idgs.store.pb.GetRequest");
  descriptor->setInOperation(opGet.getName(), opGet);
  descriptor->setOutOperation(opGet.getName(), opGet);

  // option OP_DELETE
  idgs::actor::ActorOperationDescriporWrapper opDelete;
  opDelete.setName(OP_DELETE);
  opDelete.setDescription("handle listener after delete operation");
  opDelete.setPayloadType("idgs.store.pb.RemoveRequest");
  descriptor->setInOperation(opDelete.getName(), opDelete);
  descriptor->setOutOperation(opDelete.getName(), opDelete);

  // option OP_TRUNCATE
  idgs::actor::ActorOperationDescriporWrapper opTruncate;
  opTruncate.setName(OP_TRUNCATE);
  opTruncate.setDescription("handle listener after truncate operation");
  opTruncate.setPayloadType("idgs.store.pb.DataClearRequest");
  descriptor->setInOperation(opTruncate.getName(), opTruncate);
  descriptor->setOutOperation(opTruncate.getName(), opTruncate);

  return descriptor;
}

const idgs::actor::ActorMessageHandlerMap& ListenerManager::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {OP_INSERT, static_cast<idgs::actor::ActorMessageHandler>(&ListenerManager::handleListenerInsert)},
      {OP_UPDATE, static_cast<idgs::actor::ActorMessageHandler>(&ListenerManager::handleListenerUpdate)},
      {OP_GET, static_cast<idgs::actor::ActorMessageHandler>(&ListenerManager::handleListenerGet)},
      {OP_DELETE, static_cast<idgs::actor::ActorMessageHandler>(&ListenerManager::handleListenerDelete)},
      {OP_TRUNCATE, static_cast<idgs::actor::ActorMessageHandler>(&ListenerManager::handleListenerTruncate)}
  };

  return handlerMap;
}

void ListenerManager::handleListenerInsert(const idgs::actor::ActorMessagePtr& msg) {
  pb::InsertRequest* request = dynamic_cast<pb::InsertRequest*>(msg->getPayload().get());
  const std::string& storeName = request->store_name();
  std::shared_ptr<StoreConfigWrapper> config;
  auto code = idgs::util::singleton<DataStore>::getInstance().getStoreConfigWrappers().getStoreConfig(storeName, config);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "store " << storeName << " is not found.";
    return;
  }

  pb::StoreListenerInfo* listenerInfo = new pb::StoreListenerInfo;
  if (!msg->parseAttachment(STORE_ATTACH_LISTENER, listenerInfo)) {
    LOG(ERROR) << "store " << storeName << " is not found.";
    return;
  }

  uint32_t index = listenerInfo->listener_index();
  auto& listeners = config->getStoreListener();
  for (int32_t i = index; i < listeners.size(); ++ i) {
    StoreListener* listener = listeners[i];
    auto code = listener->insert(msg);
    switch (code) {
      case LRC_CONTINUE:
        continue;
      case LRC_BREAK:
      case LRC_END:
        break;
      case LRC_ERROR:
        LOG(ERROR) << "error when execute listener " << listener->getName() << ", index " << i;
        break;
    }
  }

  delete listenerInfo;
}

void ListenerManager::handleListenerUpdate(const idgs::actor::ActorMessagePtr& msg) {
  pb::UpdateRequest* request = dynamic_cast<pb::UpdateRequest*>(msg->getPayload().get());
  const std::string& storeName = request->store_name();
  std::shared_ptr<StoreConfigWrapper> config;
  auto code = idgs::util::singleton<DataStore>::getInstance().getStoreConfigWrappers().getStoreConfig(storeName, config);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "store " << storeName << " is not found.";
    return;
  }

  pb::StoreListenerInfo* listenerInfo = new pb::StoreListenerInfo;
  if (!msg->parseAttachment(STORE_ATTACH_LISTENER, listenerInfo)) {
    LOG(ERROR) << "store " << storeName << " is not found.";
    return;
  }

  uint32_t index = listenerInfo->listener_index();
  auto& listeners = config->getStoreListener();
  for (int32_t i = index; i < listeners.size(); ++ i) {
    StoreListener* listener = listeners[i];
    auto code = listener->update(msg);
      switch (code) {
      case LRC_CONTINUE:
        continue;
      case LRC_BREAK:
      case LRC_END:
        break;
      case LRC_ERROR:
        LOG(ERROR) << "error when execute listener " << listener->getName() << ", index " << i;
        break;
    }
  }

  delete listenerInfo;
}

void ListenerManager::handleListenerGet(const idgs::actor::ActorMessagePtr& msg) {
  pb::GetRequest* request = dynamic_cast<pb::GetRequest*>(msg->getPayload().get());
  const std::string& storeName = request->store_name();
  std::shared_ptr<StoreConfigWrapper> config;
  auto code = idgs::util::singleton<DataStore>::getInstance().getStoreConfigWrappers().getStoreConfig(storeName, config);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "store " << storeName << " is not found.";
    return;
  }

  pb::StoreListenerInfo* listenerInfo = new pb::StoreListenerInfo;
  if (!msg->parseAttachment(STORE_ATTACH_LISTENER, listenerInfo)) {
    LOG(ERROR) << "store " << storeName << " is not found.";
    return;
  }

  uint32_t index = listenerInfo->listener_index();
  auto& listeners = config->getStoreListener();
  for (int32_t i = index; i < listeners.size(); ++ i) {
    StoreListener* listener = listeners[i];
    auto code = listener->get(msg);
    switch (code) {
      case LRC_CONTINUE:
        continue;
      case LRC_BREAK:
      case LRC_END:
        break;
      case LRC_ERROR:
        LOG(ERROR) << "error when execute listener " << listener->getName() << ", index " << i;
        break;
    }
  }

  delete listenerInfo;
}

void ListenerManager::handleListenerDelete(const idgs::actor::ActorMessagePtr& msg) {
  pb::RemoveRequest* request = dynamic_cast<pb::RemoveRequest*>(msg->getPayload().get());
  const std::string& storeName = request->store_name();
  std::shared_ptr<StoreConfigWrapper> config;
  auto code = idgs::util::singleton<DataStore>::getInstance().getStoreConfigWrappers().getStoreConfig(storeName, config);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "store " << storeName << " is not found.";
    return;
  }

  pb::StoreListenerInfo* listenerInfo = new pb::StoreListenerInfo;
  if (!msg->parseAttachment(STORE_ATTACH_LISTENER, listenerInfo)) {
    LOG(ERROR) << "store " << storeName << " is not found.";
    return;
  }

  uint32_t index = listenerInfo->listener_index();
  auto& listeners = config->getStoreListener();
  for (int32_t i = index; i < listeners.size(); ++ i) {
    StoreListener* listener = listeners[i];
    auto code = listener->remove(msg);
    switch (code) {
      case LRC_CONTINUE:
        continue;
      case LRC_BREAK:
      case LRC_END:
        break;
      case LRC_ERROR:
        LOG(ERROR) << "error when execute listener " << listener->getName() << ", index " << i;
        break;
    }
  }

  delete listenerInfo;
}

void ListenerManager::handleListenerTruncate(const idgs::actor::ActorMessagePtr& msg) {
  pb::DataClearRequest* request = dynamic_cast<pb::DataClearRequest*>(msg->getPayload().get());
  const std::string& storeName = request->store_name();
  std::shared_ptr<StoreConfigWrapper> config;
  auto code = idgs::util::singleton<DataStore>::getInstance().getStoreConfigWrappers().getStoreConfig(storeName, config);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "store " << storeName << " is not found.";
    return;
  }

  pb::StoreListenerInfo* listenerInfo = new pb::StoreListenerInfo;
  if (!msg->parseAttachment(STORE_ATTACH_LISTENER, listenerInfo)) {
    LOG(ERROR) << "store " << storeName << " is not found.";
    return;
  }

  uint32_t index = listenerInfo->listener_index();
  auto listeners = config->getStoreListener();
  for (int32_t i = index; i < listeners.size(); ++ i) {
    StoreListener* listener = listeners[i];
    listener->setListenerIndex(i);
    auto code = listener->truncate(msg);
    switch (code) {
      case LRC_CONTINUE:
        continue;
      case LRC_BREAK:
      case LRC_END:
        break;
      case LRC_ERROR:
        LOG(ERROR) << "error when execute listener " << listener->getName() << ", index " << i;
        break;
    }
  }

  delete listenerInfo;
}

} /* namespace store */
} /* namespace idgs */
