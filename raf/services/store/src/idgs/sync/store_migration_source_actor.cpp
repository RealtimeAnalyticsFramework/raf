/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "store_migration_source_actor.h"

#include "idgs/application.h"


namespace idgs {
namespace store {

struct MigrationRedoLog {
  std::string opName;
  idgs::actor::PbMessagePtr key;
  idgs::actor::PbMessagePtr value;
};

idgs::actor::ActorDescriptorPtr StoreMigrationSourceActor::descriptor;

StoreMigrationSourceActor::StoreMigrationSourceActor(const int32_t& partitionId) : partId(partitionId), batchSize(100), pstore(NULL) {
}

StoreMigrationSourceActor::~StoreMigrationSourceActor() {
}

const idgs::actor::ActorMessageHandlerMap& StoreMigrationSourceActor::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {MIGRATION_REQUEST, static_cast<idgs::actor::ActorMessageHandler>(&StoreMigrationSourceActor::handleMigrationRequest)}
  };

  return handlerMap;
}

const idgs::actor::ActorDescriptorPtr& StoreMigrationSourceActor::getDescriptor() const {
  return StoreMigrationSourceActor::descriptor;
}

idgs::actor::ActorDescriptorPtr StoreMigrationSourceActor::generateActorDescriptor() {
  static std::shared_ptr<idgs::actor::ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = std::make_shared<idgs::actor::ActorDescriptorWrapper>();

  descriptor->setName(STORE_MIGRATION_SOURCE_ACTOR);
  descriptor->setDescription("Data store migration on source member");
  descriptor->setType(idgs::pb::AT_STATELESS);

  // operation MIGRATION_REQUEST
  idgs::actor::ActorOperationDescriporWrapper migrationRequest;
  migrationRequest.setName(MIGRATION_REQUEST);
  migrationRequest.setDescription("handle request of migration.");
  migrationRequest.setPayloadType("idgs.store.pb.MigrationRequest");
  descriptor->setInOperation(migrationRequest.getName(), migrationRequest);

  // out operation of MIGRATION_REQUEST
  idgs::actor::ActorOperationDescriporWrapper migrationData;
  migrationData.setName(MIGRATION_DATA);
  migrationData.setDescription("Handle received migration data from source member.");
  migrationData.setPayloadType("idgs.store.pb.MigrationData");
  descriptor->setInOperation(migrationData.getName(), migrationData);

  // out operation of MIGRATION_REQUEST
  idgs::actor::ActorOperationDescriporWrapper stateChanged;
  stateChanged.setName(idgs::cluster::PARTITION_STATE_CHANGED);
  stateChanged.setDescription("handle partition state changed.");
  stateChanged.setPayloadType("idgs.pb.PartitionStatusChangeEvent");
  descriptor->setOutOperation(stateChanged.getName(), stateChanged);

  StoreMigrationSourceActor::descriptor = descriptor;

  return descriptor;
}

void StoreMigrationSourceActor::setStore(const idgs::store::StorePtr& store) {
  pstore = dynamic_cast<PartitionStore*>(store.get());
  batchSize = pstore->getStoreConfigWrapper()->getStoreConfig().migration_batch_size();

  pstore->snapshotStore(partId, map);
  it = map->iterator();

  pstore->addMigrationActor(this);
}

void StoreMigrationSourceActor::addToRedoLog(const std::string& opName, const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
  MigrationRedoLog redolog;
  redolog.opName = opName;
  redolog.key = key;
  redolog.value = value;

  redoLog.push(redolog);
}

int32_t StoreMigrationSourceActor::getPartitionid() const {
  return partId;
}

void StoreMigrationSourceActor::handleMigrationRequest(const idgs::actor::ActorMessagePtr& msg) {
  // migration data
  if (it->hasNext()) {
    std::shared_ptr<pb::MigrationData> payload = std::make_shared<pb::MigrationData>();

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
    reqMsg->setOperationName(MIGRATION_DATA);
    reqMsg->setPayload(payload);
    idgs::actor::postMessage(reqMsg);

    return;
  }

  // migration store cache
  if (!redoLog.empty()) {
    std::shared_ptr<pb::MigrationData> payload = std::make_shared<pb::MigrationData>();

    auto mode = static_cast<protobuf::SerdesMode>(msg->getSerdesType());
    size_t cnt = 0;

    while (!redoLog.empty() && cnt < batchSize) {
      MigrationRedoLog info;
      redoLog.try_pop(info);

      if (!info.key) {
        continue;
      }

      auto data = payload->add_data();
      data->set_operation_name(info.opName);
      protobuf::ProtoSerdesHelper::serialize(mode, info.key.get(), data->mutable_key());
      if (info.value) {
        protobuf::ProtoSerdesHelper::serialize(mode, info.value.get(), data->mutable_value());
      }

      ++ cnt;
    }

    auto reqMsg = msg->createResponse();
    reqMsg->setOperationName(MIGRATION_DATA);
    reqMsg->setPayload(payload);
    idgs::actor::postMessage(reqMsg);

    return;
  }

  auto& wrapper = pstore->getStoreConfigWrapper();
  auto& schemaName = wrapper->getSchema();
  auto& storeName = wrapper->getStoreConfig().name();

  auto payload = std::make_shared<pb::StoreMigrationComplete>();
  payload->set_schema_name(schemaName);
  payload->set_store_name(storeName);
  payload->set_data_size(map->size());

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(STORE_MIGRATION_COMPLETE);
  respMsg->setPayload(payload);
  DVLOG(2) << "store " << schemaName << "." << storeName << " of partition " << partId << " migration complete on source member ";
  idgs::actor::sendMessage(respMsg);

  terminate();
}

void StoreMigrationSourceActor::onDestroy() {
  state.store(TERMINATE);
  msg_queue.clear();

  map->clear();
  pstore->removeMigrationActor(this);

  auto af = idgs_application()->getRpcFramework()->getActorManager();
  af->unregisterSessionActor(this->getActorId());

  delete this;
}

} /* namespace store */
} /* namespace idgs */
