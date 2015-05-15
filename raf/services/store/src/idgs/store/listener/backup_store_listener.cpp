/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intela�?s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include "backup_store_listener.h"

#include "idgs/store/store_module.h"
#include "idgs/store/listener/listener_manager.h"
#include "idgs/store/listener/backup_store_listener.h"

#include "idgs/sync/store_migration_source_actor.h"

namespace idgs {
namespace store {

BackupStoreListener::BackupStoreListener() {
}

BackupStoreListener::~BackupStoreListener() {
}

ListenerResultCode BackupStoreListener::insert(ListenerContext* ctx) {
  if (getStoreConfig().partition_type() == pb::REPLICATED) {
    return LRC_CONTINUE;
  }

  if (getStoreConfig().replica_count() <= 1) {
    return LRC_CONTINUE;
  }

  const std::string& storeName = getStoreConfig().name();
  DVLOG_FIRST_N(2, 20) << "starting handle insert backup data to store " << storeName
                       << " from local member with store listener(" << ctx->getListenerIndex() << ").";

  auto partId = ctx->getPartitionId();
  auto partition = idgs_application()->getPartitionManager()->getPartition(partId);

  auto priMemberId = ctx->getPrmaryMemberId();
  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();

  if (localMemberId == priMemberId) {
    auto replicaCount = partition->getReplicaCount();
    auto storeReplicaCount = getStoreConfig().replica_count();
    auto cnt = storeReplicaCount <= replicaCount ? storeReplicaCount : replicaCount;
    for (int32_t i = 0; i < cnt; ++ i) {
      auto memberId = partition->getMemberId(i);
      if (memberId != -1 && memberId != priMemberId) {
        auto routeMsg = ctx->getMessage()->createRouteMessage(memberId, LISTENER_MANAGER);
        idgs::actor::sendMessage(routeMsg);
      }
    }

    ctx->setResultCode(idgs::store::pb::SRC_SUCCESS);
    return LRC_CONTINUE;
  } else {
    StoreOption ps;
    ps.partitionId = partId;

    auto& key = * ctx->getKey();
    auto& value = * ctx->getValue();
    StoreValue<google::protobuf::Message> storeValue(value);

    auto store = idgs_store_module()->getDataStore()->getStore(storeName);
    auto code = store->put(key, storeValue, &ps);

    auto state = partition->getMemberState(localMemberId);
    if (state == idgs::pb::PS_SOURCE) {
      addToRedoLog(store, partId, OP_INTERNAL_INSERT, key, value);
    }

    if (code == RC_SUCCESS) {
      DVLOG_FIRST_N(2, 20) << "insert store " << storeName << " by partition " << partId << ", key : "
                           << key->ShortDebugString() << ", value : " << value->ShortDebugString();

      ctx->setResultCode(pb::SRC_SUCCESS);
      return LRC_BREAK;
    } else {
      LOG(ERROR)<< "Insert backup store " << storeName << " on member " << localMemberId << " failed, caused by " << getErrorDescription(code);
      ctx->setResultCode(getResultCode(code));
      return LRC_ERROR;
    }
  }
}

ListenerResultCode BackupStoreListener::update(ListenerContext* ctx) {
  if (getStoreConfig().partition_type() == pb::REPLICATED) {
    return LRC_CONTINUE;
  }

  if (getStoreConfig().replica_count() <= 1) {
    return LRC_CONTINUE;
  }

  const std::string& storeName = getStoreConfig().name();
  DVLOG_FIRST_N(2, 20) << "starting handle insert backup data to store " << storeName
                       << " from local member with store listener(" << ctx->getListenerIndex() << ").";

  auto partId = ctx->getPartitionId();
  auto partition = idgs_application()->getPartitionManager()->getPartition(partId);

  auto priMemberId = ctx->getPrmaryMemberId();
  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();

  if (localMemberId == priMemberId) {
    auto replicaCount = partition->getReplicaCount();
    auto storeReplicaCount = getStoreConfig().replica_count();
    auto cnt = storeReplicaCount <= replicaCount ? storeReplicaCount : replicaCount;
    for (int32_t i = 0; i < cnt; ++ i) {
      auto memberId = partition->getMemberId(i);
      if (memberId != -1 && memberId != priMemberId) {
        auto routeMsg = ctx->getMessage()->createRouteMessage(memberId, LISTENER_MANAGER);
        idgs::actor::sendMessage(routeMsg);
      }
    }

    ctx->setResultCode(pb::SRC_SUCCESS);
    return LRC_CONTINUE;
  } else {
    StoreOption ps;
    ps.partitionId = partId;

    auto& key = * ctx->getKey();
    auto& value = * ctx->getValue();
    StoreValue<google::protobuf::Message> storeValue(value);

    auto store = idgs_store_module()->getDataStore()->getStore(storeName);
    auto code = store->put(key, storeValue, &ps);

    auto state = partition->getMemberState(localMemberId);
    if (state == idgs::pb::PS_SOURCE) {
      addToRedoLog(store, partId, OP_INTERNAL_UPDATE, key, value);
    }

    if (ctx->getOptions() & pb::RETRIEVE_PREVIOUS) {
      if (!ctx->getLastValue() && (* ctx->getLastValue())) {
        ctx->setLastValue(&storeValue.get());
      }
    }

    if (code == RC_SUCCESS) {
      DVLOG_FIRST_N(2, 20) << "update store " << storeName << " by partition " << partId << ", key : "
                           << key->ShortDebugString() << ", value : " << value->ShortDebugString();

      ctx->setResultCode(pb::SRC_SUCCESS);
      return LRC_BREAK;
    } else {
      LOG(ERROR)<< "update backup store " << storeName << " on member " << localMemberId << " failed, caused by " << getErrorDescription(code);
      ctx->setResultCode(getResultCode(code));
      return LRC_ERROR;
    }
  }
}

ListenerResultCode BackupStoreListener::remove(ListenerContext* ctx) {
  if (getStoreConfig().partition_type() == pb::REPLICATED) {
    return LRC_CONTINUE;
  }

  if (getStoreConfig().replica_count() <= 1) {
    return LRC_CONTINUE;
  }

  const std::string& storeName = getStoreConfig().name();
  DVLOG_FIRST_N(2, 20) << "starting handle Remove backup data to store " << storeName
                       << " from local member with store listener(" << ctx->getListenerIndex() << ").";

  auto partId = ctx->getPartitionId();
  auto partition = idgs_application()->getPartitionManager()->getPartition(partId);

  auto priMemberId = ctx->getPrmaryMemberId();
  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();

  if (localMemberId == priMemberId) {
    auto replicaCount = partition->getReplicaCount();
    auto storeReplicaCount = getStoreConfig().replica_count();
    auto cnt = storeReplicaCount <= replicaCount ? storeReplicaCount : replicaCount;
    for (int32_t i = 0; i < cnt; ++ i) {
      auto memberId = partition->getMemberId(i);
      if (memberId != -1 && memberId != priMemberId) {
        auto routeMsg = ctx->getMessage()->createRouteMessage(memberId, LISTENER_MANAGER);
        idgs::actor::sendMessage(routeMsg);
      }
    }

    ctx->setResultCode(pb::SRC_SUCCESS);
    return LRC_CONTINUE;
  } else {
    StoreOption ps;
    ps.partitionId = partId;

    auto& key = * ctx->getKey();
    auto& value = * ctx->getValue();
    StoreValue<google::protobuf::Message> storeValue(value);

    auto store = idgs_store_module()->getDataStore()->getStore(storeName);
    auto code = store->remove(key, storeValue, &ps);

    auto state = partition->getMemberState(localMemberId);
    if (state == idgs::pb::PS_SOURCE) {
      addToRedoLog(store, partId, OP_INTERNAL_DELETE, key, idgs::actor::PbMessagePtr());
    }

    if (ctx->getOptions() & pb::RETRIEVE_PREVIOUS) {
      if (!ctx->getLastValue() && (* ctx->getLastValue())) {
        ctx->setLastValue(&storeValue.get());
      }
    }

    if (code == RC_SUCCESS) {
      DVLOG_FIRST_N(2, 20) << "delete store " << storeName << " by partition " << partId << ", key : " << key->ShortDebugString();
      ctx->setResultCode(pb::SRC_SUCCESS);
      return LRC_BREAK;
    } else {
      LOG(ERROR)<< "delete backup store " << storeName << " on member " << localMemberId << " failed, caused by " << getErrorDescription(code);
      ctx->setResultCode(getResultCode(code));
      return LRC_ERROR;
    }
  }
}

void BackupStoreListener::addToRedoLog(StorePtr& store, const int32_t& partition, const std::string& opName, const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
  auto type = store->getStoreConfig()->getStoreConfig().partition_type();
  if (type == pb::PARTITION_TABLE) {
    auto pstore = dynamic_cast<PartitionedStore*>(store.get());
    auto actors = pstore->getMigrationActors();
    auto it = actors.begin() ;
    for (; it != actors.end(); ++ it) {
      auto actor = dynamic_cast<StoreMigrationSourceActor*>(* it);
      if (actor->getPartitionid() == partition) {
        actor->addToRedoLog(opName, key, value);
      }
    }
  }
}

pb::StoreResultCode BackupStoreListener::getResultCode(const ResultCode& status) {
  static std::map<ResultCode, pb::StoreResultCode> codeMapping = {
      {RC_SUCCESS,                    pb::SRC_SUCCESS},
      {RC_INVALID_KEY,                pb::SRC_INVALID_KEY},
      {RC_INVALID_VALUE,              pb::SRC_INVALID_VALUE},
      {RC_STORE_NOT_FOUND,            pb::SRC_STORE_NOT_FOUND},
      {RC_PARTITION_NOT_FOUND,        pb::SRC_PARTITION_NOT_FOUND},
      {RC_DATA_NOT_FOUND,             pb::SRC_DATA_NOT_FOUND},
      {RC_PARTITION_NOT_READY,        pb::SRC_PARTITION_NOT_READY},
      {RC_NOT_LOCAL_STORE,            pb::SRC_NOT_LOCAL_STORE}
  };
  auto it = codeMapping.find(status);
  if (it == codeMapping.end()) {
    return pb::SRC_UNKNOWN_ERROR;
  }

  return it->second;
}

} /* namespace store */
} /* namespace idgs */

