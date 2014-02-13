
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "datastore_listener.h"

#include "data_store.h"
#include "idgs/cluster/cluster_framework.h"
#include "data_sync_actor.h"

using namespace idgs::pb;
using namespace idgs::cluster;
using namespace idgs::pb;

namespace idgs {
namespace store {

PartitionChangeListener::PartitionChangeListener() {
}

PartitionChangeListener::~PartitionChangeListener() {
  function_footprint();
}

/// @todo migration
void PartitionChangeListener::partitionChanged(const DeltaPartitionEvent& evt) {
  DVLOG(3) << evt.DebugString();
  function_footprint();
  auto localMember = ::idgs::util::singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMember();
  if (!localMember) {
    return;
  }
  if (localMember->isLocalStore()) {
    int32_t localMemberId = localMember->getId();
    for (int32_t i = 0; i < evt.items_size(); ++i) {
      auto& item = evt.items(i);
      if (item.position() == 0) {
        if (item.currmemberid() == item.newmemberid()) {
          break;
        }
        ::idgs::util::singleton<DataStore>::getInstance().migrateData(item.partitionid(), localMemberId,
            item.currmemberid(), item.newmemberid());

        if (item.needmigrate()) {
          ::idgs::util::singleton<ClusterFramework>::getInstance().getPartitionManager()->getPartition(
              item.partitionid())->setState(item.position(), true);
        }
      }
    }
  }
}

MemberJoinedListener::MemberJoinedListener() {
}

MemberJoinedListener::~MemberJoinedListener() {
  function_footprint();
}

void MemberJoinedListener::statusChanged(const MemberWrapper& member) {
  function_footprint();
  if (member.isPrepared() && member.isLocalStore()) {
    int32_t local_member_id = ::idgs::util::singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();
    if (member.getId() == local_member_id) {
      ReplicatedStoreSyncStatefulActor* syncActor = new ReplicatedStoreSyncStatefulActor;
      syncActor->handleDataSyncRequest();
    }
  }
}

} /* namespace store */
} /* namespace idgs */
