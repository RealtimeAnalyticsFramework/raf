/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "store_sync_source_actor.h"

#include "idgs/application.h"

#include "idgs/sync/pb/data_sync.pb.h"

namespace idgs {
namespace store {

struct SyncRedoLog {
  std::string opName;
  idgs::actor::PbMessagePtr key;
  idgs::actor::PbMessagePtr value;
};

idgs::actor::ActorDescriptorPtr StoreSyncSourceActor::descriptor;

StoreSyncSourceActor::StoreSyncSourceActor() : batchSize(200), rstore(NULL) {
}

StoreSyncSourceActor::~StoreSyncSourceActor() {
}

const idgs::actor::ActorMessageHandlerMap& StoreSyncSourceActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {SYNC_REQUEST,   {
          static_cast<idgs::actor::ActorMessageHandler>(&StoreSyncSourceActor::handleSyncRequest),
          &idgs::store::pb::SyncRequest::default_instance()
      }}
  };

  return handlerMap;
}

const idgs::actor::ActorDescriptorPtr& StoreSyncSourceActor::getDescriptor() const {
  return StoreSyncSourceActor::descriptor;
}

idgs::actor::ActorDescriptorPtr StoreSyncSourceActor::generateActorDescriptor() {
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

  // out operation SYNC_REQUEST
  idgs::actor::ActorOperationDescriporWrapper syncData;
  syncData.setName(SYNC_DATA);
  syncData.setDescription("Receive sync data from source member.");
  syncData.setPayloadType("idgs.store.pb.SyncData");
  descriptor->setOutOperation(syncData.getName(), syncData);

  // out operation SYNC_REQUEST
  idgs::actor::ActorOperationDescriporWrapper syncComplete;
  syncComplete.setName(SYNC_COMPLETE);
  syncComplete.setDescription("Handle sync data of one store complete.");
  syncComplete.setPayloadType("idgs.store.pb.SyncComplete");
  descriptor->setOutOperation(syncComplete.getName(), syncComplete);

  StoreSyncSourceActor::descriptor = descriptor;

  return descriptor;
}

void StoreSyncSourceActor::setStore(const StorePtr& store) {
  rstore = dynamic_cast<ReplicatedStore*>(store.get());
  batchSize = rstore->getStoreConfig()->getStoreConfig().migration_batch_size();

  rstore->snapshotStore(map);
  it = map->iterator();

  rstore->addSyncActor(this);
}

void StoreSyncSourceActor::addToRedoLog(const std::string& opName, const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
  SyncRedoLog redolog;
  redolog.opName = opName;
  redolog.key = key;
  redolog.value = value;

  redoLog.push(redolog);
}

void StoreSyncSourceActor::handleSyncRequest(const idgs::actor::ActorMessagePtr& msg) {
  // sync data
  if (it->hasNext()) {
    std::shared_ptr<pb::SyncData> payload = std::make_shared<pb::SyncData>();

    size_t cnt = 0;
    auto mode = static_cast<protobuf::SerdesMode>(msg->getSerdesType());
    while (it->hasNext() && cnt < batchSize) {
      auto data = payload->add_data();
      data->set_operation_name(OP_INTERNAL_INSERT);
      protobuf::ProtoSerdesHelper::serialize(mode, it->key().get(), data->mutable_key());
      protobuf::ProtoSerdesHelper::serialize(mode, it->value().get().get(), data->mutable_value());

      ++ cnt;
      it->next();
    }

    auto reqMsg = msg->createResponse();
    reqMsg->setOperationName(SYNC_DATA);
    reqMsg->setPayload(payload);
    idgs::actor::postMessage(reqMsg);

    return;
  }

  // sync store redolog
  if (!redoLog.empty()) {
    std::shared_ptr<pb::SyncData> payload = std::make_shared<pb::SyncData>();

    auto mode = static_cast<protobuf::SerdesMode>(msg->getSerdesType());
    size_t cnt = 0;

    while (!redoLog.empty() && cnt < batchSize) {
      SyncRedoLog redolog;
      redoLog.try_pop(redolog);

      if (!redolog.key) {
        continue;
      }

      auto data = payload->add_data();
      data->set_operation_name(redolog.opName);
      protobuf::ProtoSerdesHelper::serialize(mode, redolog.key.get(), data->mutable_key());
      if (redolog.value) {
        protobuf::ProtoSerdesHelper::serialize(mode, redolog.value.get(), data->mutable_value());
      }

      ++ cnt;
    }

    auto reqMsg = msg->createResponse();
    reqMsg->setOperationName(SYNC_DATA);
    reqMsg->setPayload(payload);
    idgs::actor::postMessage(reqMsg);

    return;
  }

  auto& wrapper = rstore->getStoreConfig();
  auto& schemaName = wrapper->getSchema();
  auto& storeName = wrapper->getStoreConfig().name();

  auto payload = std::make_shared<pb::SyncComplete>();
  payload->set_schema_name(schemaName);
  payload->set_store_name(storeName);
  payload->set_data_size(map->size());

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(SYNC_COMPLETE);
  respMsg->setPayload(payload);
  DVLOG(2) << "store " << schemaName << "." << storeName << " sync complete on source member ";
  idgs::actor::sendMessage(respMsg);

  terminate();
}

void StoreSyncSourceActor::onDestroy() {
  state.store(TERMINATE);
  msg_queue.clear();

  map->clear();
  rstore->removeSyncActor(this);

  auto af = idgs_application()->getRpcFramework()->getActorManager();
  af->unregisterSessionActor(this->getActorId());

  delete this;
}

} /* namespace store */
} /* namespace idgs */
