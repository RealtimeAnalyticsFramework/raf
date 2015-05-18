/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#pragma once

#include "idgs/actor/stateful_actor.h"

#include "idgs/store/datastore_const.h"
#include "idgs/store/partitioned_store.h"


namespace idgs {
namespace store {

class StoreMigrationTargetActor : public idgs::actor::StatefulActor {
public:
  StoreMigrationTargetActor(const int32_t& partitionId, const int32_t sourceMemberId);
  virtual ~StoreMigrationTargetActor();

  const std::string& getActorName() const override {
    static std::string actorName = STORE_MIGRATION_TARGET_ACTOR;
    return actorName;
  }

  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

  const idgs::actor::ActorDescriptorPtr& getDescriptor() const;

  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

  void setStore(idgs::store::StorePtr store);

private:
  void handleMigrationData(const idgs::actor::ActorMessagePtr& msg);
  void handleStoreMigrationComplete(const idgs::actor::ActorMessagePtr& msg);
  void handleCancelMigration(const idgs::actor::ActorMessagePtr& msg);
  void handleSourceMemberLeave(const idgs::actor::ActorMessagePtr& msg);

private:
  int32_t partId;
  int32_t sourceMId;

  size_t dataSize;
  idgs::store::StorePtr pstore;

  static idgs::actor::ActorDescriptorPtr descriptor;

};

} /* namespace store */
} /* namespace idgs */
