/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#pragma once

#include "idgs/actor/stateful_actor.h"

#include "idgs/store/datastore_const.h"
#include "idgs/store/store.h"

#include "idgs/sync/pb/data_sync.pb.h"

namespace idgs {
namespace store {

struct MigrationRedoLog;

class StoreMigrationSourceActor : public idgs::actor::StatefulActor {
public:
  StoreMigrationSourceActor(const int32_t& partitionId);
  virtual ~StoreMigrationSourceActor();

  const std::string& getActorName() const override {
    static std::string actorName = STORE_MIGRATION_SOURCE_ACTOR;
    return actorName;
  }

  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

  const idgs::actor::ActorDescriptorPtr& getDescriptor() const;

  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

  void setStore(const idgs::store::StorePtr& store);

  void addToRedoLog(const std::string& opName, const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value);

  int32_t getPartitionid() const;

  virtual void onDestroy() override;

private:
  void handleMigrationRequest(const idgs::actor::ActorMessagePtr& msg);

private:
  int32_t partId;

  int32_t batchSize;
  PartitionStore* pstore;

  std::shared_ptr<StoreMap> map;
  std::shared_ptr<StoreMap::Iterator> it;

  tbb::concurrent_queue<MigrationRedoLog> redoLog;

  static idgs::actor::ActorDescriptorPtr descriptor;

};

} /* namespace store */
} /* namespace idgs */
