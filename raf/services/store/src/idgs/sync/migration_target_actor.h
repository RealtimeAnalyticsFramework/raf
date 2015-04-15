/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#pragma once

#include <atomic>
#include <tbb/spin_rw_mutex.h>
#include <tbb/concurrent_queue.h>

#include "idgs/actor/stateless_actor.h"

#include "idgs/store/datastore_const.h"

#include "idgs/pb/cluster_event.pb.h"

namespace idgs {
namespace store {

struct StoreMigrationInfo;

class MigrationTargetActor : public idgs::actor::StatelessActor {
public:
  MigrationTargetActor();
  virtual ~MigrationTargetActor();

  const std::string& getActorName() const override {
    static std::string actorName = MIGRATION_TARGET_ACTOR;
    return actorName;
  }

  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

  void addPartition(const int32_t& partitionId);

  void clearPartitionData(const int32_t& partitionId);

  void startPartitionMigration();

  void handleMemberLeave(const int32_t& memberId);

private:
  void handleStoreMigrationComplete(const idgs::actor::ActorMessagePtr& msg);

  void nextPartitionMigration();

  void multicastPartitionStateChangeEvent(const int32_t& partitionId, const idgs::pb::PartitionState& state);

  virtual void onDestroy() override;

private:
  tbb::spin_rw_mutex mutex;

  std::atomic_int partId;

  int32_t sourceMemberId;

  int32_t storeMigrationBatchSize;

  tbb::concurrent_queue<int32_t> partitionQueue;
  tbb::concurrent_queue<std::shared_ptr<StoreMigrationInfo>> storeMigrationQueue;
  std::map<std::string, std::shared_ptr<StoreMigrationInfo>> storeMigrationMap;

};

} /* namespace store */
} /* namespace idgs */
