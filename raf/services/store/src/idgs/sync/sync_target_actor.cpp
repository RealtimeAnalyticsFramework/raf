/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "sync_target_actor.h"

#include "idgs/store/store_module.h"
#include "idgs/store/listener/listener_manager.h"

#include "idgs/sync/store_migration_source_actor.h"
#include "idgs/sync/sync_target_actor.h"
#include "idgs/sync/store_sync_target_actor.h"



namespace idgs {
namespace store {

struct StoreSyncInfo {
  std::string schemaName;
  std::string storeName;
  std::string storeActorId;
  bool syncronizing;
};

SyncTargetActor::SyncTargetActor() : sourceMId(-1), storeSyncBatchSize(-1) {
  actorId = SYNC_TARGET_ACTOR;

  syncronizing.store(false);
  descriptor = generateActorDescriptor();
}

SyncTargetActor::~SyncTargetActor() {
}

const idgs::actor::ActorMessageHandlerMap& SyncTargetActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {SYNC_COMPLETE,  {
          static_cast<idgs::actor::ActorMessageHandler>(&SyncTargetActor::handleSyncComplete),
          &idgs::store::pb::SyncComplete::default_instance()
      }}
  };

  return handlerMap;
}

idgs::actor::ActorDescriptorPtr SyncTargetActor::generateActorDescriptor() {
  static std::shared_ptr<idgs::actor::ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = std::make_shared<idgs::actor::ActorDescriptorWrapper>();

  descriptor->setName(SYNC_TARGET_ACTOR);
  descriptor->setDescription("Target member of replicated store sync data.");
  descriptor->setType(idgs::pb::AT_STATELESS);

  // operation of SYNC_COMPLETE
  idgs::actor::ActorOperationDescriporWrapper syncComplete;
  syncComplete.setName(SYNC_COMPLETE);
  syncComplete.setDescription("Handle sync data of one store complete.");
  syncComplete.setPayloadType("idgs.store.pb.SyncComplete");
  descriptor->setInOperation(syncComplete.getName(), syncComplete);

  // out operation of SYNC_COMPLETE
  idgs::actor::ActorOperationDescriporWrapper SyncRequest;
  SyncRequest.setName(SYNC_REQUEST);
  SyncRequest.setDescription("Send sync data request to source member");
  SyncRequest.setPayloadType("idgs.store.pb.SyncRequest");
  descriptor->setOutOperation(SyncRequest.getName(), SyncRequest);

  return descriptor;
}

void SyncTargetActor::startSyncData() {
  DVLOG(2) << "starting sync replicate store";
  syncronizing.store(true);

  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    sourceMId = getSourceMember();
    if (sourceMId == -1) {
      LOG(WARNING) << "no valid member with local store found.";
      syncronizing.store(false);
      return;
    }
  }

  DVLOG(2) << "getting source member " << sourceMId << ", prepare to call sync request";
  auto datastore = idgs_store_module()->getDataStore();
  std::vector<StorePtr> stores;
  datastore->getStores(stores);

  auto batchSize = (storeSyncBatchSize == -1) ? stores.size() : storeSyncBatchSize;
  std::vector<idgs::actor::ActorMessagePtr> messages;

  {
    auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    for (int32_t i = 0; i < stores.size(); ++ i) {
      auto& store = stores.at(i);
      auto& wrapper = store->getStoreConfig();
      auto& config = wrapper->getStoreConfig();
      if (config.partition_type() == pb::REPLICATED) {
        auto& schemaName = wrapper->getSchema();
        auto& storeName = config.name();

        std::shared_ptr<StoreSyncInfo> info = std::make_shared<StoreSyncInfo>();
        info->schemaName = schemaName;
        info->storeName = storeName;
        info->syncronizing = true;

        if (storeSyncMap.size() > batchSize) {
          storeSyncQueue.push(info);
        } else {
          StoreSyncTargetActor* actor = new StoreSyncTargetActor;
          actor->setStore(store);
          idgs_application()->registerSessionActor(actor);

          auto request = std::make_shared<pb::SyncRequest>();
          request->set_schema_name(wrapper->getSchema());
          request->set_store_name(config.name());

          idgs::actor::ActorMessagePtr reqMsg = std::make_shared<idgs::actor::ActorMessage>();
          reqMsg->setOperationName(SYNC_REQUEST);
          reqMsg->setSourceActorId(actor->getActorId());
          reqMsg->setSourceMemberId(localMemberId);
          reqMsg->setDestActorId(SYNC_SOURCE_ACTOR);
          reqMsg->setDestMemberId(sourceMId);
          reqMsg->setPayload(request);

          DVLOG(2) << "call sync request of store " << schemaName << "." << storeName;
          messages.push_back(reqMsg);

          info->storeActorId = actor->getActorId();
        }

        std::string key = "schema : " + schemaName + ", store : " + storeName;
        storeSyncMap.insert(std::make_pair(key, info));
      }
    }
  }

  auto it = messages.begin();
  for (; it != messages.end(); ++ it) {
    idgs::actor::postMessage(* it);
  }
}

void SyncTargetActor::handleMemberLeaveEvent(const int32_t memberId) {
  bool isLeave = false;
  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
    isLeave = (memberId == sourceMId);
  }

  if (isLeave) {
    DVLOG(2) << "finding source member " << sourceMId << " is leaving, restart sync data";
    auto datastore = idgs_store_module()->getDataStore();
    auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
    {
      tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
      storeSyncQueue.clear();
      auto it = storeSyncMap.begin();
      for (; it != storeSyncMap.end(); ++ it) {
        if (it->second->storeActorId != "") {
          auto msg = createActorMessage();
          msg->setOperationName(SOURCE_MEMBER_LEAVE);
          msg->setDestMemberId(localMemberId);
          msg->setDestActorId(it->second->storeActorId);
          msg->setPayload(std::make_shared<pb::SourceMemberLeaveEvent>());

          idgs::actor::sendMessage(msg);
        } else {
          auto store = datastore->getStore(it->second->schemaName, it->second->storeName);
          store->removeAll();
        }
      }

      storeSyncMap.clear();
      syncronizing.store(false);
    }

    startSyncData();
  }
}

void SyncTargetActor::handleSyncComplete(const idgs::actor::ActorMessagePtr& msg) {
  auto request = dynamic_cast<pb::SyncComplete*>(msg->getPayload().get());
  auto& schemaName = request->schema_name();
  auto& storeName = request->store_name();
  std::string key = "schema : " + schemaName + ", store : " + storeName;
  DVLOG(2) << "store " << schemaName << "." << storeName << " sync complete";

  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    auto it = storeSyncMap.find(key);
    if (it == storeSyncMap.end()) {
      return;
    }

    it->second->syncronizing = false;
    it->second->storeActorId = "";

    if (!storeSyncQueue.empty()) {
      std::shared_ptr<StoreSyncInfo> info;
      storeSyncQueue.try_pop(info);

      auto store = idgs_store_module()->getDataStore()->getStore(info->schemaName, info->storeName);
      StoreSyncTargetActor* actor = new StoreSyncTargetActor;
      actor->setStore(store);
      idgs_application()->registerSessionActor(actor);
      info->storeActorId = actor->getActorId();

      auto request = std::make_shared<pb::SyncRequest>();
      request->set_schema_name(info->schemaName);
      request->set_store_name(info->storeName);

      auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();

      idgs::actor::ActorMessagePtr reqMsg = std::make_shared<idgs::actor::ActorMessage>();
      reqMsg->setOperationName(SYNC_REQUEST);
      reqMsg->setSourceActorId(actor->getActorId());
      reqMsg->setSourceMemberId(localMemberId);
      reqMsg->setDestActorId(SYNC_SOURCE_ACTOR);
      reqMsg->setDestMemberId(sourceMId);
      reqMsg->setPayload(request);

      idgs::actor::sendMessage(reqMsg);

      return;
    }

    it = storeSyncMap.begin();
    for (; it != storeSyncMap.end(); ++ it) {
      if (it->second->syncronizing) {
        return;
      }
    }
  }

  DVLOG(1) << "all stores sync complete.";
  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    storeSyncMap.clear();
    sourceMId = -1;
    syncronizing.store(false);
  }
}

int32_t SyncTargetActor::getSourceMember() {
  int32_t sourceMemberId = -1;

  auto memberMgr = idgs_application()->getMemberManager();
  auto localMemberId = memberMgr->getLocalMemberId();

  auto& memberTable = memberMgr->getMemberTable();
  for (int i = 0; i < memberTable.size(); ++i) {
    auto& member = memberTable.at(i);
    if (member.getId() == localMemberId) {
      continue;
    }

    if ((member.getState() == idgs::pb::MS_PREPARED || member.getState() == idgs::pb::MS_ACTIVE) && member.isLocalStore()) {
      sourceMemberId = member.getId();
      break;
    }
  }

  return sourceMemberId;
}

void SyncTargetActor::onDestroy() {
  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    storeSyncQueue.clear();

    auto it = storeSyncMap.begin();
    for (; it != storeSyncMap.end(); ++ it) {
      if (it->second->storeActorId != "") {
        terminate(it->second->storeActorId, localMemberId);
      }
    }
    storeSyncMap.clear();
  }
  StatelessActor::onDestroy();
}

} /* namespace store */
} /* namespace idgs */
