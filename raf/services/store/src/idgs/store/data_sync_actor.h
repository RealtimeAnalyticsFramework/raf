
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/actor/stateless_actor.h"
#include "idgs/actor/stateful_actor.h"
#include "idgs/actor/actor_message.h"
#include "datastore_const.h"

namespace idgs {
namespace store {

/// The stateless actor
/// To syncronize data.
class StoreSyncStatelessActor: public idgs::actor::StatelessActor {
public:

  /// @brief  Construction.
  /// @param  actorId Actor id
  StoreSyncStatelessActor(const std::string& actorId);
  ~StoreSyncStatelessActor();

  /// @brief  Generate descriptor for StoreSyncStatelessActor.
  /// @return The descriptor for StoreSyncStatelessActor
  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

  const std::string& getActorName() const {
    static std::string actorName = DATA_STORE_SYNC_ACTOR;
    return actorName;
  }

  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

private:
  void handleDataSync(const idgs::actor::ActorMessagePtr& msg);
};

/// The stateful actor
/// To send request and receive response of syncronize data.
class ReplicatedStoreSyncStatefulActor: public idgs::actor::StatefulActor {
public:

  ReplicatedStoreSyncStatefulActor();
  virtual ~ReplicatedStoreSyncStatefulActor();

  /// @brief  Generate descriptor for ReplicatedStoreSyncStatefulActor.
  /// @return The descriptor for ReplicatedStoreSyncStatefulActor
  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

  /// @brief  Get descriptor for ReplicatedStoreSyncStatefulActor.
  /// @return The descriptor for ReplicatedStoreSyncStatefulActor.
  const idgs::actor::ActorDescriptorPtr& getDescriptor() const;

  /// @brief  Handle data synchronization request for replicated store.
  void handleDataSyncRequest();

  const std::string& getActorName() const {
    static std::string actorName = DATA_REPLICATED_STORE_SYNC_ACTOR;
    return actorName;
  }

  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

protected:

  /// Overwrite super class, whether local store process
  void innerProcess(const idgs::actor::ActorMessagePtr& msg) override;

private:
  typedef idgs::actor::StatefulActor super;
  void handleDataSyncResponse(const idgs::actor::ActorMessagePtr& msg);
  void getReplicatedStores();

  static idgs::actor::ActorDescriptorPtr descriptor;
  int32_t storeCnt;
  std::vector<std::string> replicatedStores;
};

} // namespace store
} // namespace idgs
