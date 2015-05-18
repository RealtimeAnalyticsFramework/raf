/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#include "idgs/actor/stateless_actor.h"

#include "idgs/rdd/rdd_local.h"


namespace idgs {
namespace rdd {

class RddInternalServiceActor : public idgs::actor::StatelessActor {
public:
  RddInternalServiceActor();
  virtual ~RddInternalServiceActor();

public:
  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;
  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

private:
  void handleCreateStoreDelegate(const idgs::actor::ActorMessagePtr& msg);
  void handleCreateDelegatePartition(const idgs::actor::ActorMessagePtr& msg);
  void handleCreateRdd(const idgs::actor::ActorMessagePtr& msg);
  void handleCreateRddPartition(const idgs::actor::ActorMessagePtr& msg);
  void handlePartitionCreated(const idgs::actor::ActorMessagePtr& msg);

  void handleRddActionRequest(const idgs::actor::ActorMessagePtr& msg);

  void handleRddDestroy(const idgs::actor::ActorMessagePtr& msg);
  void handleRemoveRddLocal(const idgs::actor::ActorMessagePtr& msg);

  void handleRddStateSync(const idgs::actor::ActorMessagePtr& msg);
  void handlePersistTypeSync(const idgs::actor::ActorMessagePtr& msg);

  void handleMemberEvent(const idgs::actor::ActorMessagePtr& msg);

  void handleListRdd(const idgs::actor::ActorMessagePtr& msg);

private:
  void registerPbMessage(const std::string& protoFile);
  idgs::rdd::pb::RddResultCode registerPbMessage(const idgs::rdd::pb::OutRddInfo& out, const std::string& suffix);
  void sendResponse(const idgs::actor::ActorMessagePtr& msg, const std::string& opName, const idgs::actor::PbMessagePtr& payload);

  void destroyAllRddActor();

  virtual void onDestroy() override;

private:
  std::map<std::string, std::shared_ptr<RddLocal>> rddLocalMap;
  tbb::spin_rw_mutex mutex;
};
} // namespace rdd
} // namespace idgs
