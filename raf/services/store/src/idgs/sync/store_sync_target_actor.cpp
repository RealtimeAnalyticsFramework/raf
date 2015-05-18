/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "store_sync_target_actor.h"

#include "idgs/application.h"

#include "idgs/sync/pb/data_sync.pb.h"

namespace idgs {
namespace store {

idgs::actor::ActorDescriptorPtr StoreSyncTargetActor::descriptor;

StoreSyncTargetActor::StoreSyncTargetActor() : dataSize(0), rstore(NULL) {
}

StoreSyncTargetActor::~StoreSyncTargetActor() {
}

const idgs::actor::ActorMessageHandlerMap& StoreSyncTargetActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {SYNC_DATA,  {
          static_cast<idgs::actor::ActorMessageHandler>(&StoreSyncTargetActor::handleSyncData),
          &idgs::store::pb::SyncData::default_instance()
      }},
      {SYNC_COMPLETE, {
          static_cast<idgs::actor::ActorMessageHandler>(&StoreSyncTargetActor::handleSyncComplete),
          &idgs::store::pb::SyncComplete::default_instance()
      }},
      {SOURCE_MEMBER_LEAVE,  {
          static_cast<idgs::actor::ActorMessageHandler>(&StoreSyncTargetActor::handleSourceMemberLeave),
          NULL
      }}
  };

  return handlerMap;
}

const idgs::actor::ActorDescriptorPtr& StoreSyncTargetActor::getDescriptor() const {
  return StoreSyncTargetActor::descriptor;
}

idgs::actor::ActorDescriptorPtr StoreSyncTargetActor::generateActorDescriptor() {
  static std::shared_ptr<idgs::actor::ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = std::make_shared<idgs::actor::ActorDescriptorWrapper>();

  descriptor->setName(STORE_SYNC_TARGET_ACTOR);
  descriptor->setDescription("Target member of each replicated store sync data.");
  descriptor->setType(idgs::pb::AT_STATEFUL);

  // operation SYNC_DATA
  idgs::actor::ActorOperationDescriporWrapper syncData;
  syncData.setName(SYNC_DATA);
  syncData.setDescription("Receive sync data from source member.");
  syncData.setPayloadType("idgs.store.pb.SyncData");
  descriptor->setInOperation(syncData.getName(), syncData);

  // operation SYNC_COMPLETE
  idgs::actor::ActorOperationDescriporWrapper syncComplete;
  syncComplete.setName(SYNC_COMPLETE);
  syncComplete.setDescription("Handle sync data of one store complete.");
  syncComplete.setPayloadType("idgs.store.pb.SyncComplete");
  descriptor->setInOperation(syncComplete.getName(), syncComplete);

  // out operation of SYNC_DATA and SYNC_COMPLETE
  idgs::actor::ActorOperationDescriporWrapper SyncRequest;
  SyncRequest.setName(SYNC_REQUEST);
  SyncRequest.setDescription("Send sync data request to source member");
  SyncRequest.setPayloadType("idgs.store.pb.SyncRequest");
  descriptor->setOutOperation(SyncRequest.getName(), SyncRequest);

  StoreSyncTargetActor::descriptor = descriptor;

  return descriptor;
}

void StoreSyncTargetActor::setStore(const StorePtr& store) {
  rstore = dynamic_cast<ReplicatedStore*>(store.get());
}

void StoreSyncTargetActor::handleSyncData(const idgs::actor::ActorMessagePtr& msg) {
  auto data = dynamic_cast<pb::SyncData*>(msg->getPayload().get());
  auto mode = static_cast<protobuf::SerdesMode>(msg->getSerdesType());

  for (int32_t i = 0; i < data->data_size(); ++ i) {
    idgs::actor::PbMessagePtr key = rstore->getStoreConfig()->newKey();
    if (!protobuf::ProtoSerdesHelper::deserialize(mode, data->data(i).key(), key.get())) {
      LOG(ERROR) << "parse key of sync data error.";
      continue;
    }

    idgs::actor::PbMessagePtr value = rstore->getStoreConfig()->newValue();
    if (!protobuf::ProtoSerdesHelper::deserialize(mode, data->data(i).value(), value.get())) {
      LOG(ERROR) << "parse value of sync data error.";
      continue;
    }

    ResultCode code = RC_SUCCESS;
    auto& opName = data->data(i).operation_name();

    if (opName == OP_INTERNAL_INSERT || opName == OP_INTERNAL_UPDATE) {
      StoreKey<google::protobuf::Message> storeKey(key);
      StoreValue<google::protobuf::Message> storeValue(value);
      code = rstore->put(storeKey, storeValue);
    } else if (opName == OP_INTERNAL_DELETE) {
      StoreKey<google::protobuf::Message> storeKey(key);
      StoreValue<google::protobuf::Message> storeValue(value);
      code = rstore->remove(storeKey, storeValue);
    } else {
      code = RC_NOT_SUPPORT;
    }

    if (code != RC_SUCCESS) {
      LOG(ERROR) << "saving data error, caused by " << getErrorDescription(code);
      continue;
    }

    ++ dataSize;
  }

  auto& configWrapper = rstore->getStoreConfig();
  auto& schemaName = configWrapper->getSchema();
  auto& storeName = configWrapper->getStoreConfig().name();

  auto payload = std::make_shared<pb::SyncRequest>();
  payload->set_schema_name(schemaName);
  payload->set_store_name(storeName);

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(SYNC_REQUEST);
  respMsg->setPayload(payload);

  idgs::actor::sendMessage(respMsg);
}

void StoreSyncTargetActor::handleSyncComplete(const idgs::actor::ActorMessagePtr& msg) {
  auto request = dynamic_cast<pb::SyncComplete*>(msg->getPayload().get());
  DVLOG(2) << "store " << request->schema_name() << "." << request->store_name() << " sync complete, sync data " << dataSize << "/" << request->data_size();

  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
  auto routeMsg = msg->createRouteMessage(localMemberId, SYNC_TARGET_ACTOR, true);
  idgs::actor::sendMessage(routeMsg);

  terminate();
}

void StoreSyncTargetActor::handleSourceMemberLeave(const idgs::actor::ActorMessagePtr& msg) {
  rstore->removeAll();
  terminate();
}

} /* namespace store */
} /* namespace idgs */
