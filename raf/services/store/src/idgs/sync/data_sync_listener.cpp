
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "data_sync_listener.h"


#include "idgs/cluster/balancer/balancer_util.h"


#include "idgs/store/store_module.h"

namespace idgs {
namespace store {

DataMigraionListener::DataMigraionListener() {
}

DataMigraionListener::~DataMigraionListener() {
  function_footprint();
}

void DataMigraionListener::partitionChanged(const idgs::pb::DeltaPartitionEvent& evt) {
  function_footprint();
  DVLOG(1) << "========== partition table changed ==========";
  std::vector<idgs::cluster::MigrateAction> actions;
  auto& partitions = idgs_application()->getPartitionManager()->getPartitionTable();
  idgs::cluster::BalancerUtil::generateMigrateActionList(partitions, const_cast<idgs::pb::DeltaPartitionEvent&>(evt), actions);

  if (!actions.empty()) {
    auto memberMgr = idgs_application()->getMemberManager();
    int32_t localMemberId = memberMgr->getLocalMemberId();

    auto actor = idgs_application()->getActorframework()->getActor(MIGRATION_TARGET_ACTOR);
    MigrationTargetActor* targetActor = dynamic_cast<MigrationTargetActor*>(actor);

    auto it = actions.begin();
    for (; it != actions.end(); ++ it) {
      if (it->member_id == localMemberId) {
        auto partitionId = it->partition_id;
        if (it->command == idgs::cluster::MigrateAction::NEW) {
          targetActor->addPartition(partitionId);
        } else if (it->command == idgs::cluster::MigrateAction::DEL) {
          targetActor->clearPartitionData(partitionId);
        }
      }
    }

    targetActor->startPartitionMigration();
  }
}

DataSyncListener::DataSyncListener() {
}

DataSyncListener::~DataSyncListener() {
  function_footprint();
}

void DataSyncListener::memberStatusChanged(const idgs::cluster::MemberWrapper& member) {
  function_footprint();
  DVLOG(1) << "========== member status changed ==========";
  if (member.getState() == idgs::pb::MS_INACTIVE && member.isLocalStore()) {
    auto af = idgs_application()->getActorframework();

    // leaved member is source member of migration
    auto targetActor = af->getActor(MIGRATION_TARGET_ACTOR);
    MigrationTargetActor* migrationTargetActor = dynamic_cast<MigrationTargetActor*>(targetActor);
    migrationTargetActor->handleMemberLeave(member.getId());

    // leaved member is target member of migration
    auto sourceActor = af->getActor(MIGRATION_SOURCE_ACTOR);
    MigrationSourceActor* migrationSourceActor = dynamic_cast<MigrationSourceActor*>(sourceActor);
    migrationSourceActor->handleMemberLeave(member.getId());

    // leaved member is source member of sync
    auto syncActor = idgs_application()->getActorframework()->getActor(SYNC_TARGET_ACTOR);
    SyncTargetActor* syncTargetActor = dynamic_cast<SyncTargetActor*>(syncActor);
    syncTargetActor->handleMemberLeaveEvent(member.getId());
  }

  if ((member.getState() == idgs::pb::MS_PREPARED || member.getState() == idgs::pb::MS_ACTIVE) && member.isLocalStore()) {
    auto memberMgr = idgs_application()->getMemberManager();
    auto localMemberId = memberMgr->getLocalMemberId();

    if (member.getId() == localMemberId) {
      auto actor = idgs_application()->getActorframework()->getActor(SYNC_TARGET_ACTOR);
      SyncTargetActor* syncTargetActor = dynamic_cast<SyncTargetActor*>(actor);
      syncTargetActor->startSyncData();
    }
  }
}

} /* namespace store */
} /* namespace idgs */
