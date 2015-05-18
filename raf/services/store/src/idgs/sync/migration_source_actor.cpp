/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "migration_source_actor.h"

#include "idgs/store/store_module.h"
#include "idgs/store/listener/listener_manager.h"

#include "idgs/sync/migration_source_actor.h"
#include "idgs/sync/store_migration_source_actor.h"


namespace idgs {
namespace store {

MigrationSourceActor::MigrationSourceActor() {
  actorId = MIGRATION_SOURCE_ACTOR;

  descriptor = generateActorDescriptor();
}

MigrationSourceActor::~MigrationSourceActor() {
}

const idgs::actor::ActorMessageHandlerMap& MigrationSourceActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {MIGRATION_REQUEST,  {
          static_cast<idgs::actor::ActorMessageHandler>(&MigrationSourceActor::handleMigrationRequest),
          &idgs::store::pb::MigrationRequest::default_instance()
      }},
      {PARTITION_MIGRATION_COMPLETE,   {
          static_cast<idgs::actor::ActorMessageHandler>(&MigrationSourceActor::handlePartitionMigrationComplete),
          &idgs::store::pb::PartitionMigrationComplete::default_instance()
      }},
      {CANCEL_MIGRATION,  {
          static_cast<idgs::actor::ActorMessageHandler>(&MigrationSourceActor::handleCancelMigration),
          &idgs::store::pb::CancelMigration::default_instance()
      }}
  };

  return handlerMap;
}

idgs::actor::ActorDescriptorPtr MigrationSourceActor::generateActorDescriptor() {
  static std::shared_ptr<idgs::actor::ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = std::make_shared<idgs::actor::ActorDescriptorWrapper>();

  descriptor->setName(MIGRATION_SOURCE_ACTOR);
  descriptor->setDescription("Data store migration on source member");
  descriptor->setType(idgs::pb::AT_STATELESS);

  // operation MIGRATION_REQUEST
  idgs::actor::ActorOperationDescriporWrapper migrationRequest;
  migrationRequest.setName(MIGRATION_REQUEST);
  migrationRequest.setDescription("handle request of migration.");
  migrationRequest.setPayloadType("idgs.store.pb.MigrationRequest");
  descriptor->setInOperation(migrationRequest.getName(), migrationRequest);

  // operation PARTITION_MIGRATION_COMPLETE
  idgs::actor::ActorOperationDescriporWrapper pMigrationComplete;
  pMigrationComplete.setName(PARTITION_MIGRATION_COMPLETE);
  pMigrationComplete.setDescription("handle migration complete of the whole partition.");
  pMigrationComplete.setPayloadType("idgs.store.pb.PartitionMigrationComplete");
  descriptor->setInOperation(pMigrationComplete.getName(), pMigrationComplete);

  // operation CANCEL_MIGRATION
  idgs::actor::ActorOperationDescriporWrapper cancelMigration;
  cancelMigration.setName(CANCEL_MIGRATION);
  cancelMigration.setDescription("Handle cancel migration.");
  cancelMigration.setPayloadType("idgs.store.pb.CancelMigration");
  descriptor->setInOperation(cancelMigration.getName(), cancelMigration);

  // out operation of MIGRATION_REQUEST
  descriptor->setOutOperation(migrationRequest.getName(), migrationRequest);

  // out operation of PARTITION_MIGRATION_COMPLETE
  idgs::actor::ActorOperationDescriporWrapper stateChanged;
  stateChanged.setName(idgs::cluster::OID_PARTITION_STATE_CHANGED);
  stateChanged.setDescription("handle partition state changed.");
  stateChanged.setPayloadType("idgs.pb.PartitionStatusChangeEvent");
  descriptor->setOutOperation(stateChanged.getName(), stateChanged);

  // out operation CANCEL_MIGRATION
  idgs::actor::ActorOperationDescriporWrapper destroy;
  destroy.setName(OP_DESTROY);
  destroy.setDescription("handle destroy store migration source actor.");
  destroy.setPayloadType("idgs.store.pb.Destroy");
  descriptor->setOutOperation(destroy.getName(), destroy);

  return descriptor;
}

void MigrationSourceActor::handleMemberLeave(const int32_t& memberId) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  auto it = partitionStateMap.begin();
  for (; it != partitionStateMap.end(); ++ it) {
    auto mit = it->second.find(memberId);
    if (mit != it->second.end()) {
      it->second.erase(mit);

      if (it->second.empty()) {
        multicastPartitionStateChangeEvent(it->first, idgs::pb::PS_READY);
      }
    }
  }
}

void MigrationSourceActor::multicastPartitionStateChangeEvent(const int32_t& partitionId, const idgs::pb::PartitionState& state) {
  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();

  auto payload = std::make_shared<idgs::pb::PartitionStatusChangeEvent>();
  payload->set_partition_id(partitionId);
  payload->set_member_id(localMemberId);
  payload->set_state(state);

  auto reqMsg = std::make_shared<idgs::actor::ActorMessage>();
  reqMsg->setOperationName(idgs::cluster::OID_PARTITION_STATE_CHANGED);
  reqMsg->setSourceActorId(MIGRATION_TARGET_ACTOR);
  reqMsg->setSourceMemberId(localMemberId);
  reqMsg->setDestMemberId(idgs::pb::ALL_MEMBERS);
  reqMsg->setDestActorId(idgs::cluster::AID_PARTITION);
  reqMsg->setChannel(idgs::pb::TC_MULTICAST);
  reqMsg->setPayload(payload);

  DVLOG(1) << "partition " << partitionId << " member " << localMemberId << " change state " << idgs::pb::PartitionState_Name(state);
  idgs::actor::sendMessage(reqMsg);
}

void MigrationSourceActor::handleMigrationRequest(const idgs::actor::ActorMessagePtr& msg) {
  auto request = dynamic_cast<pb::MigrationRequest*>(msg->getPayload().get());
  auto partId = request->partition_id();
  auto& schemaName = request->schema_name();
  auto& storeName = request->store_name();
  auto targetMemberId = msg->getSourceMemberId();

  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    auto it = partitionStateMap.find(partId);
    if (it == partitionStateMap.end()) {
      partitionStateMap[partId].insert(targetMemberId);
      multicastPartitionStateChangeEvent(partId, idgs::pb::PS_SOURCE);
    } else {
      auto mit = it->second.find(targetMemberId);
      if (mit == it->second.end()) {
        it->second.insert(targetMemberId);
      }
    }
  }

  DVLOG(2) << "receive request of " << schemaName << "." << storeName << " of partition " << partId;

  std::string key = "schema : " + schemaName + ", store : " + storeName;
  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();

  auto store = idgs_store_module()->getDataStore()->getStore(schemaName, storeName);
  if (!store) {
    LOG(ERROR) << "store " << schemaName << "." << storeName << " is not found on member " << localMemberId;
    return;
  }

  auto pstore = dynamic_cast<PartitionedStore*>(store.get());
  if (pstore->dataSize(partId) == 0) {
    DVLOG(2) << "partition " << partId << " of store " << schemaName << "." << storeName << " has no data.";
    auto payload = std::make_shared<pb::StoreMigrationComplete>();
    payload->set_schema_name(schemaName);
    payload->set_store_name(storeName);
    payload->set_data_size(0);

    auto respMsg = msg->createResponse();
    respMsg->setOperationName(STORE_MIGRATION_COMPLETE);
    respMsg->setPayload(payload);
    idgs::actor::sendMessage(respMsg);
    return;
  }

  StoreMigrationSourceActor* actor = new StoreMigrationSourceActor(partId);
  actor->setStore(store);
  idgs_application()->registerSessionActor(actor);
  // VLOG(0) << "StoreMigrationSourceActor(" << actor->getActorId() << ")";

  auto routeMsg = msg->createRouteMessage(localMemberId, actor->getActorId());
  idgs::actor::sendMessage(routeMsg);
}

void MigrationSourceActor::handlePartitionMigrationComplete(const idgs::actor::ActorMessagePtr& msg) {
  auto request = dynamic_cast<pb::PartitionMigrationComplete*>(msg->getPayload().get());
  auto partId = request->partition_id();
  auto targetMemberId = msg->getSourceMemberId();
  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    auto it = partitionStateMap.find(partId);
    if (it == partitionStateMap.end()) {
      LOG(WARNING) << "partition id " << partId << " is not found.";
      return;
    }

    auto mit = it->second.find(targetMemberId);
    if (mit == it->second.end()) {
      LOG(WARNING) << "member " << targetMemberId << " is not a target member in partition " << partId;
      return;
    }

    it->second.erase(mit);
    if (it->second.empty()) {
      partitionStateMap.erase(it);
      multicastPartitionStateChangeEvent(partId, idgs::pb::PS_READY);
    }
  }
}

void MigrationSourceActor::handleCancelMigration(const idgs::actor::ActorMessagePtr& msg) {
  auto request = dynamic_cast<pb::CancelMigration*>(msg->getPayload().get());
  auto store = idgs_store_module()->getDataStore()->getStore(request->schema_name(), request->store_name());
  auto pstore = dynamic_cast<PartitionedStore*>(store.get());
  auto actors = pstore->getMigrationActors();
  auto it = actors.begin();
  for (; it != actors.end(); ++ it) {
    auto actor = dynamic_cast<StoreMigrationSourceActor*>(* it);
    if (actor->getPartitionid() == request->partition_id()) {
      actor->terminate();
    }
  }
}

void MigrationSourceActor::onDestroy() {
  std::vector<StorePtr> stores;
  idgs_store_module()->getDataStore()->getStores(stores);
  auto it = stores.begin();
  for (; it != stores.end(); ++ it) {
    auto& store = * it;
    if (store->getStoreConfig()->getStoreConfig().partition_type() == pb::PARTITION_TABLE) {
      auto pstore = dynamic_cast<PartitionedStore*>(store.get());
      auto actors = pstore->getMigrationActors();
      for (auto actor: actors) {
        actor->terminate();
      }
    }
  }

  StatelessActor::onDestroy();
}

} // namespace store
} // namespace idgs
