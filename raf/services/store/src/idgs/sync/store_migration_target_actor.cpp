/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "store_migration_target_actor.h"

#include "idgs/application.h"

#include "idgs/sync/pb/data_sync.pb.h"

namespace idgs {
namespace store {

idgs::actor::ActorDescriptorPtr StoreMigrationTargetActor::descriptor;

StoreMigrationTargetActor::StoreMigrationTargetActor(const int32_t& partitionId, const int32_t sourceMemberId) :
    partId(partitionId), sourceMId(sourceMemberId), dataSize(0), pstore(NULL) {
}

StoreMigrationTargetActor::~StoreMigrationTargetActor() {
}

const idgs::actor::ActorMessageHandlerMap& StoreMigrationTargetActor::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {MIGRATION_DATA,               static_cast<idgs::actor::ActorMessageHandler>(&StoreMigrationTargetActor::handleMigrationData)},
      {STORE_MIGRATION_COMPLETE,     static_cast<idgs::actor::ActorMessageHandler>(&StoreMigrationTargetActor::handleStoreMigrationComplete)},
      {CANCEL_MIGRATION,             static_cast<idgs::actor::ActorMessageHandler>(&StoreMigrationTargetActor::handleCancelMigration)},
      {SOURCE_MEMBER_LEAVE,          static_cast<idgs::actor::ActorMessageHandler>(&StoreMigrationTargetActor::handleSourceMemberLeave)}
  };

  return handlerMap;
}

const idgs::actor::ActorDescriptorPtr& StoreMigrationTargetActor::getDescriptor() const {
  return StoreMigrationTargetActor::descriptor;
}

idgs::actor::ActorDescriptorPtr StoreMigrationTargetActor::generateActorDescriptor() {
  static std::shared_ptr<idgs::actor::ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = std::make_shared<idgs::actor::ActorDescriptorWrapper>();

  descriptor->setName(STORE_MIGRATION_TARGET_ACTOR);
  descriptor->setDescription("Data store migration of each store on target member");
  descriptor->setType(idgs::pb::AT_STATEFUL);

  // operation MIGRATION_DATA
  idgs::actor::ActorOperationDescriporWrapper migrationData;
  migrationData.setName(MIGRATION_DATA);
  migrationData.setDescription("Handle received migration data from source member.");
  migrationData.setPayloadType("idgs.store.pb.MigrationData");
  descriptor->setInOperation(migrationData.getName(), migrationData);

  // operation MIGRATION_COMPLETE
  idgs::actor::ActorOperationDescriporWrapper migrationComplete;
  migrationComplete.setName(STORE_MIGRATION_COMPLETE);
  migrationComplete.setDescription("Handle received migration info from source member.");
  migrationComplete.setPayloadType("idgs.store.pb.StoreMigrationComplete");
  descriptor->setInOperation(migrationComplete.getName(), migrationComplete);

  // operation CANCEL_MIGRATION
  idgs::actor::ActorOperationDescriporWrapper cancelMigration;
  cancelMigration.setName(CANCEL_MIGRATION);
  cancelMigration.setDescription("Handle cancel migration.");
  cancelMigration.setPayloadType("idgs.store.pb.CancelMigration");
  descriptor->setInOperation(cancelMigration.getName(), cancelMigration);

  idgs::actor::ActorOperationDescriporWrapper sourceMemberLeaveEvent;
  sourceMemberLeaveEvent.setName(SOURCE_MEMBER_LEAVE);
  sourceMemberLeaveEvent.setDescription("Handle source member leave event.");
  sourceMemberLeaveEvent.setPayloadType("idgs.store.pb.SourceMemberLeaveEvent");
  descriptor->setInOperation(sourceMemberLeaveEvent.getName(), sourceMemberLeaveEvent);

  // out operation of MIGRATION_DATA
  idgs::actor::ActorOperationDescriporWrapper migrationRequest;
  migrationRequest.setName(MIGRATION_REQUEST);
  migrationRequest.setDescription("handle request of migration.");
  migrationRequest.setPayloadType("idgs.store.pb.MigrationRequest");
  descriptor->setOutOperation(migrationRequest.getName(), migrationRequest);

  // out operation of MIGRATION_COMPLETE
  descriptor->setOutOperation(migrationComplete.getName(), migrationComplete);

  // out operation CANCEL_MIGRATION
  descriptor->setOutOperation(cancelMigration.getName(), cancelMigration);

  StoreMigrationTargetActor::descriptor = descriptor;

  return descriptor;
}

void StoreMigrationTargetActor::setStore(const idgs::store::StorePtr& store) {
  pstore = dynamic_cast<PartitionStore*>(store.get());
}

void StoreMigrationTargetActor::handleMigrationData(const idgs::actor::ActorMessagePtr& msg) {
  auto data = dynamic_cast<pb::MigrationData*>(msg->getPayload().get());
  auto mode = static_cast<protobuf::SerdesMode>(msg->getSerdesType());

  auto configWrapper = pstore->getStoreConfigWrapper();
  for (int32_t i = 0; i < data->data_size(); ++ i) {
    idgs::actor::PbMessagePtr key = configWrapper->newKey();
    if (!protobuf::ProtoSerdesHelper::deserialize(mode, data->data(i).key(), key.get())) {
      LOG(ERROR) << "parse key of migration data error.";
      continue;
    }

    idgs::actor::PbMessagePtr value = configWrapper->newValue();
    if (!protobuf::ProtoSerdesHelper::deserialize(mode, data->data(i).value(), value.get())) {
      LOG(ERROR) << "parse value of migration data error.";
      continue;
    }

    ResultCode code = RC_SUCCESS;
    auto& opName = data->data(i).operation_name();

    PartitionInfo pi;
    pi.partitionId = partId;

    if (opName == OP_INTERNAL_INSERT || opName == OP_INTERNAL_UPDATE) {
      StoreKey<google::protobuf::Message> storeKey(key);
      StoreValue<google::protobuf::Message> storeValue(value);
      code = pstore->setData(storeKey, storeValue, &pi);
    } else if (opName == OP_INTERNAL_DELETE) {
      StoreKey<google::protobuf::Message> storeKey(key);
      StoreValue<google::protobuf::Message> storeValue(value);
      code = pstore->removeData(storeKey, storeValue, &pi);
    } else {
      code = RC_NOT_SUPPORT;
    }

    if (code != RC_SUCCESS) {
      LOG(ERROR) << "saving data error, caused by " << getErrorDescription(code);
      continue;
    }

    ++ dataSize;
  }

  auto& schemaName = configWrapper->getSchema();
  auto& storeName = configWrapper->getStoreConfig().name();

  auto payload = std::make_shared<pb::MigrationRequest>();
  payload->set_partition_id(partId);
  payload->set_schema_name(schemaName);
  payload->set_store_name(storeName);

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(MIGRATION_REQUEST);
  respMsg->setPayload(payload);

  idgs::actor::sendMessage(respMsg);
}

void StoreMigrationTargetActor::handleStoreMigrationComplete(const idgs::actor::ActorMessagePtr& msg) {
  auto request = dynamic_cast<pb::StoreMigrationComplete*>(msg->getPayload().get());
  DVLOG(2) << "store " << request->schema_name() << "." << request->store_name() << " partition " << partId
           << " migration complete, migration data " << dataSize << "(" << pstore->dataSize(partId) << ")/" << request->data_size();

  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
  auto routeMsg = msg->createRouteMessage(localMemberId, MIGRATION_TARGET_ACTOR, true);
  idgs::actor::sendMessage(routeMsg);

  terminate();
}

void StoreMigrationTargetActor::handleCancelMigration(const idgs::actor::ActorMessagePtr& msg) {
  pstore->clearData(partId);
  auto routeMsg = msg->createRouteMessage(sourceMId, MIGRATION_SOURCE_ACTOR);
  idgs::actor::sendMessage(routeMsg);
  terminate();
}

void StoreMigrationTargetActor::handleSourceMemberLeave(const idgs::actor::ActorMessagePtr& msg) {
  pstore->clearData(partId);
  terminate();
}

} /* namespace store */
} /* namespace idgs */
