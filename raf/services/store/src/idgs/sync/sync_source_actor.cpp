/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "sync_source_actor.h"

#include "idgs/store/store_module.h"
#include "idgs/store/listener/listener_manager.h"

#include "idgs/sync/store_migration_source_actor.h"
#include "idgs/sync/sync_source_actor.h"
#include "idgs/sync/store_sync_source_actor.h"


namespace idgs {
namespace store {

SyncSourceActor::SyncSourceActor() {
  actorId = SYNC_SOURCE_ACTOR;

  descriptor = generateActorDescriptor();
}

SyncSourceActor::~SyncSourceActor() {
}

const idgs::actor::ActorMessageHandlerMap& SyncSourceActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {SYNC_REQUEST,    {
          static_cast<idgs::actor::ActorMessageHandler>(&SyncSourceActor::handleSyncRequest),
          &idgs::store::pb::SyncRequest::default_instance()
      }}
  };

  return handlerMap;
}

idgs::actor::ActorDescriptorPtr SyncSourceActor::generateActorDescriptor() {
  static std::shared_ptr<idgs::actor::ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = std::make_shared<idgs::actor::ActorDescriptorWrapper>();

  descriptor->setName(SYNC_SOURCE_ACTOR);
  descriptor->setDescription("Source member of replicated store sync data.");
  descriptor->setType(idgs::pb::AT_STATELESS);

  // operation SYNC_REQUEST
  idgs::actor::ActorOperationDescriporWrapper SyncRequest;
  SyncRequest.setName(SYNC_REQUEST);
  SyncRequest.setDescription("Send sync data request to source member");
  SyncRequest.setPayloadType("idgs.store.pb.SyncRequest");
  descriptor->setInOperation(SyncRequest.getName(), SyncRequest);

  // out operation of SYNC_REQUEST
  descriptor->setOutOperation(SyncRequest.getName(), SyncRequest);

  return descriptor;
}

void SyncSourceActor::handleSyncRequest(const idgs::actor::ActorMessagePtr& msg) {
  auto request = dynamic_cast<pb::SyncRequest*>(msg->getPayload().get());
  auto& schemaName = request->schema_name();
  auto& storeName = request->store_name();

  DVLOG(2) << "receive sync request of " << schemaName << "." << storeName;

  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
  auto store = idgs_store_module()->getDataStore()->getStore(schemaName, storeName);
  if (!store) {
    LOG(ERROR) << "store " << schemaName << "." << storeName << " is not found on member " << localMemberId;
    return;
  }

  if (store->size() == 0) {
    DVLOG(2) << "store " << schemaName << "." << storeName << " has no data.";
    auto payload = std::make_shared<pb::SyncComplete>();
    payload->set_schema_name(schemaName);
    payload->set_store_name(storeName);
    payload->set_data_size(0);

    auto respMsg = msg->createResponse();
    respMsg->setOperationName(SYNC_COMPLETE);
    respMsg->setPayload(payload);
    idgs::actor::sendMessage(respMsg);
    return;
  }

  StoreSyncSourceActor* actor = new StoreSyncSourceActor;
  actor->setStore(store);
  idgs_application()->registerSessionActor(actor);

  auto routeMsg = msg->createRouteMessage(localMemberId, actor->getActorId());
  idgs::actor::sendMessage(routeMsg);
}

void SyncSourceActor::onDestroy() {
  std::vector<StorePtr> stores;
  idgs_store_module()->getDataStore()->getStores(stores);
  auto it = stores.begin();
  for (; it != stores.end(); ++ it) {
    auto& store = * it;
    if (store->getStoreConfig()->getStoreConfig().partition_type() == pb::REPLICATED) {
      auto rstore = dynamic_cast<ReplicatedStore*>(store.get());
      auto actors = rstore->getSyncActors();
      for (auto actor: actors) {
        actor->terminate();
      }
    }
  }

  StatelessActor::onDestroy();
}

} /* namespace store */
} /* namespace idgs */
