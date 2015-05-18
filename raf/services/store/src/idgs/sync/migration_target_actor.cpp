/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "migration_target_actor.h"

#include "idgs/store/store_module.h"
#include "idgs/store/listener/listener_manager.h"

#include "idgs/sync/migration_target_actor.h"
#include "idgs/sync/store_migration_target_actor.h"
#include "idgs/sync/store_migration_source_actor.h"


namespace idgs {
namespace store {

struct StoreMigrationInfo {
  std::string schemaName;
  std::string storeName;
  std::string storeActorId;
  idgs::pb::PartitionState state;
};

MigrationTargetActor::MigrationTargetActor() : sourceMemberId(-1), storeMigrationBatchSize(-1) {
  actorId = MIGRATION_TARGET_ACTOR;
  partId.store(-1);

  descriptor = generateActorDescriptor();
}

MigrationTargetActor::~MigrationTargetActor() {
  partitionQueue.clear();
  storeMigrationMap.clear();
}

const idgs::actor::ActorMessageHandlerMap& MigrationTargetActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {STORE_MIGRATION_COMPLETE,  {
          static_cast<idgs::actor::ActorMessageHandler>(&MigrationTargetActor::handleStoreMigrationComplete),
          &idgs::store::pb::StoreMigrationComplete::default_instance()
      }}
  };

  return handlerMap;
}

idgs::actor::ActorDescriptorPtr MigrationTargetActor::generateActorDescriptor() {
  static std::shared_ptr<idgs::actor::ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = std::make_shared<idgs::actor::ActorDescriptorWrapper>();

  descriptor->setName(MIGRATION_TARGET_ACTOR);
  descriptor->setDescription("Data store migration on target member");
  descriptor->setType(idgs::pb::AT_STATELESS);

  // operation MIGRATION_COMPLETE
  idgs::actor::ActorOperationDescriporWrapper migrationComplete;
  migrationComplete.setName(STORE_MIGRATION_COMPLETE);
  migrationComplete.setDescription("Handle received migration info from source member.");
  migrationComplete.setPayloadType("idgs.store.pb.StoreMigrationComplete");
  descriptor->setInOperation(migrationComplete.getName(), migrationComplete);

  // out operation of START_MIGRATION
  idgs::actor::ActorOperationDescriporWrapper migrationRequest;
  migrationRequest.setName(MIGRATION_REQUEST);
  migrationRequest.setDescription("handle request of migration.");
  migrationRequest.setPayloadType("idgs.store.pb.MigrationRequest");
  descriptor->setOutOperation(migrationRequest.getName(), migrationRequest);

  // out operation of MIGRATION_COMPLETE
  idgs::actor::ActorOperationDescriporWrapper stateChanged;
  stateChanged.setName(idgs::cluster::OID_PARTITION_STATE_CHANGED);
  stateChanged.setDescription("handle partition state changed.");
  stateChanged.setPayloadType("idgs.pb.PartitionStatusChangeEvent");
  descriptor->setOutOperation(stateChanged.getName(), stateChanged);

  // no out operation of CLEAR_PARTITION_DATA

  return descriptor;
}

void MigrationTargetActor::addPartition(const int32_t& partitionId) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  partitionQueue.push(partitionId);
}

void MigrationTargetActor::clearPartitionData(const int32_t& partitionId) {
  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    auto queueIt = partitionQueue.unsafe_begin();
    for (; queueIt != partitionQueue.unsafe_end(); ++ queueIt) {
      if (* queueIt == partitionId) {
        * queueIt = -1;
      }
    }
  }

  auto datastore = idgs_store_module()->getDataStore();
  if (partId == partitionId) {
    auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
    tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
    storeMigrationQueue.clear();
    auto it = storeMigrationMap.begin();
    for (; it != storeMigrationMap.end(); ++ it) {
      if (it->second->storeActorId != "") {
        auto payload = std::make_shared<pb::CancelMigration>();
        payload->set_partition_id(partId);
        payload->set_schema_name(it->second->schemaName);
        payload->set_store_name(it->second->storeName);

        auto msg = createActorMessage();
        msg->setOperationName(CANCEL_MIGRATION);
        msg->setDestMemberId(localMemberId);
        msg->setDestActorId(it->second->storeActorId);
        msg->setPayload(payload);

        idgs::actor::sendMessage(msg);
      } else {
        auto store = datastore->getStore(it->second->schemaName, it->second->storeName);
        auto pstore = dynamic_cast<PartitionedStore*>(store.get());
        pstore->clearData(partitionId);
      }
    }

    if (sourceMemberId != -1) {
      auto payload = std::make_shared<pb::PartitionMigrationComplete>();
      payload->set_partition_id(partId);

      auto reqMsg = createActorMessage();
      reqMsg->setOperationName(PARTITION_MIGRATION_COMPLETE);
      reqMsg->setDestMemberId(sourceMemberId);
      reqMsg->setDestActorId(MIGRATION_SOURCE_ACTOR);
      reqMsg->setPayload(payload);
      idgs::actor::sendMessage(reqMsg);
    }

    return;
  } else {
    std::vector<StorePtr> stores;
    datastore->getStores(stores);

    auto it = stores.begin();
    for (; it != stores.end(); ++ it) {
      auto& store = * it;
      if (store->getStoreConfig()->getStoreConfig().partition_type() == pb::PARTITION_TABLE) {
        auto pstore = dynamic_cast<PartitionedStore*>(store.get());
        pstore->clearData(partitionId);
      }
    }
  }
}

void MigrationTargetActor::startPartitionMigration() {
  if (partId.load() == -1) {
    nextPartitionMigration();
  }
}

void MigrationTargetActor::nextPartitionMigration() {
  {
    int32_t partitionId = -1;
    tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
    while (partitionId == -1) {
      if (partitionQueue.empty()) {
        partId.store(-1);
        sourceMemberId = -1;
        return;
      }

      partitionQueue.try_pop(partitionId);
    }

    partId.store(partitionId);
    DVLOG(2) << "start prepared migration of partition " << partId;
    auto partitionMgr = idgs_application()->getPartitionManager();
    auto partitionWrapper = partitionMgr->getPartition(partId);
    sourceMemberId = partitionWrapper->getPrimaryMemberId();
  }

  if (sourceMemberId == -1) {
    // @todo : get data from persist
    LOG(WARNING) << "partition " << partId << " have not found source member with local store, waiting for migration from persistence.";
    multicastPartitionStateChangeEvent(partId, idgs::pb::PS_READY);
    nextPartitionMigration();
    return;
  }

  DVLOG(2) << "partition " << partId << " migrate data from member " << sourceMemberId;
  multicastPartitionStateChangeEvent(partId, idgs::pb::PS_MIGRATING);

  std::vector<StorePtr> stores;
  idgs_store_module()->getDataStore()->getStores(stores);

  int32_t batchSize = (storeMigrationBatchSize == -1) ? stores.size() : storeMigrationBatchSize;
  std::vector<idgs::actor::ActorMessagePtr> messages;

  {
    auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    for (int32_t i = 0; i < stores.size(); ++ i) {
      auto& store = stores.at(i);
      auto& configWrapper = store->getStoreConfig();
      auto& config = configWrapper->getStoreConfig();
      if (config.partition_type() == pb::PARTITION_TABLE) {
        auto& schemaName = configWrapper->getSchema();
        auto& storeName = config.name();
        std::string key = "schema : " + schemaName + ", store : " + storeName;

        std::shared_ptr<StoreMigrationInfo> info = std::make_shared<StoreMigrationInfo>();
        info->schemaName = schemaName;
        info->storeName = storeName;
        info->state = idgs::pb::PS_MIGRATING;

        if (storeMigrationMap.size() > batchSize) {
          storeMigrationQueue.push(info);
        } else {
          StoreMigrationTargetActor* actor = new StoreMigrationTargetActor(partId, sourceMemberId);
          actor->setStore(store);
          idgs_application()->registerSessionActor(actor);
          // VLOG(0) << "StoreMigrationTargetActor(" << actor->getActorId() << ")";

          auto payload = std::make_shared<pb::MigrationRequest>();
          payload->set_partition_id(partId);
          payload->set_schema_name(schemaName);
          payload->set_store_name(storeName);

          auto reqMsg = std::make_shared<idgs::actor::ActorMessage>();
          reqMsg->setOperationName(MIGRATION_REQUEST);
          reqMsg->setSourceMemberId(localMemberId);
          reqMsg->setSourceActorId(actor->getActorId());
          reqMsg->setDestMemberId(sourceMemberId);
          reqMsg->setDestActorId(MIGRATION_SOURCE_ACTOR);
          reqMsg->setPayload(payload);

          messages.push_back(reqMsg);

          info->storeActorId = actor->getActorId();
        }

        storeMigrationMap.insert(std::make_pair(key, info));
      }
    }
  }

  auto it = messages.begin();
  for (; it != messages.end(); ++ it) {
    idgs::actor::postMessage(* it);
  }
}

void MigrationTargetActor::handleMemberLeave(const int32_t& memberId) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  if (partId != -1 && memberId == sourceMemberId) {
    auto datastore = idgs_store_module()->getDataStore();
    auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
    storeMigrationQueue.clear();
    auto it = storeMigrationMap.begin();
    for (; it != storeMigrationMap.end(); ++ it) {
      if (it->second->storeActorId != "") {
        auto msg = createActorMessage();
        msg->setOperationName(SOURCE_MEMBER_LEAVE);
        msg->setDestMemberId(localMemberId);
        msg->setDestActorId(it->second->storeActorId);
        msg->setPayload(std::make_shared<pb::SourceMemberLeaveEvent>());

        idgs::actor::sendMessage(msg);
      } else {
        auto store = datastore->getStore(it->second->schemaName, it->second->storeName);
        auto pstore = dynamic_cast<PartitionedStore*>(store.get());
        pstore->clearData(partId);
      }
    }

    storeMigrationMap.clear();
    partitionQueue.push(partId);
    nextPartitionMigration();
  }
}

void MigrationTargetActor::multicastPartitionStateChangeEvent(const int32_t& partitionId, const idgs::pb::PartitionState& state) {
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

void MigrationTargetActor::handleStoreMigrationComplete(const idgs::actor::ActorMessagePtr& msg) {
  auto request = dynamic_cast<pb::StoreMigrationComplete*>(msg->getPayload().get());
  auto& schemaName = request->schema_name();
  auto& storeName = request->store_name();
  std::string key = "schema : " + schemaName + ", store : " + storeName;
  DVLOG(2) << "store " << schemaName << "." << storeName << " migrate complete";

  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    auto it = storeMigrationMap.find(key);
    if (it == storeMigrationMap.end()) {
      LOG(ERROR) << "";
      return;
    }
    it->second->state = idgs::pb::PS_READY;
    it->second->storeActorId = "";

    if (!storeMigrationQueue.empty()) {
      std::shared_ptr<StoreMigrationInfo> info;
      storeMigrationQueue.try_pop(info);
      auto store = idgs_store_module()->getDataStore()->getStore(info->schemaName, info->storeName);

      StoreMigrationTargetActor* actor = new StoreMigrationTargetActor(partId, sourceMemberId);
      actor->setStore(store);
      idgs_application()->registerSessionActor(actor);
      info->storeActorId = actor->getActorId();

      auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();

      auto payload = std::make_shared<pb::MigrationRequest>();
      payload->set_partition_id(partId);
      payload->set_schema_name(schemaName);
      payload->set_store_name(storeName);

      auto reqMsg = std::make_shared<idgs::actor::ActorMessage>();
      reqMsg->setOperationName(MIGRATION_REQUEST);
      reqMsg->setSourceMemberId(localMemberId);
      reqMsg->setSourceActorId(actor->getActorId());
      reqMsg->setDestMemberId(sourceMemberId);
      reqMsg->setDestActorId(MIGRATION_SOURCE_ACTOR);
      reqMsg->setPayload(payload);

      idgs::actor::postMessage(reqMsg);

      return;
    }

    it = storeMigrationMap.begin();
    for (; it != storeMigrationMap.end(); ++ it) {
      if (it->second->state != idgs::pb::PS_READY) {
        return;
      }
    }
  }

  DVLOG(1) << "all stores of partition " << partId << " migration complete, start migration of next partition.";
  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    storeMigrationMap.clear();
  }

  multicastPartitionStateChangeEvent(partId, idgs::pb::PS_READY);

  auto payload = std::make_shared<pb::PartitionMigrationComplete>();
  payload->set_partition_id(partId);

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(PARTITION_MIGRATION_COMPLETE);
  respMsg->setDestActorId(MIGRATION_SOURCE_ACTOR);
  respMsg->setPayload(payload);
  idgs::actor::sendMessage(respMsg);

  nextPartitionMigration();
}

void MigrationTargetActor::onDestroy() {
  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    partitionQueue.clear();
    storeMigrationQueue.clear();

    auto it = storeMigrationMap.begin();
    for (; it != storeMigrationMap.end(); ++ it) {
      if (it->second->storeActorId != "") {
        DVLOG(2) << "terminate StoreMigrationTargetActor(" << it->second->storeActorId << ")";
        terminate(it->second->storeActorId, localMemberId);
      }
    }
    storeMigrationMap.clear();
  }
  StatelessActor::onDestroy();
}

} /* namespace store */
} /* namespace idgs */
