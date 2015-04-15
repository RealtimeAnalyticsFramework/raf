/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#pragma once

#include "idgs/actor/stateful_actor.h"
#include "idgs/tpc/pb/tpc_sync.pb.h"

namespace idgs {
namespace tpc {

class MigrationVerifyActor : public idgs::actor::StatefulActor {
public:
  MigrationVerifyActor();
  virtual ~MigrationVerifyActor();

  const std::string& getActorName() const {
    return actorName;
  }

  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

  const idgs::actor::ActorDescriptorPtr& getDescriptor() const;

  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

private:
  void handleVerifyRequest(const idgs::actor::ActorMessagePtr& msg);
  void handleLocalVerifyRequest(const idgs::actor::ActorMessagePtr& msg);
  void handleVerifyResponse(const idgs::actor::ActorMessagePtr& msg);

private:
  idgs::actor::ActorMessagePtr clientMsg;

  int32_t memberSize;
  std::map<int32_t, pb::MemberMigrationData> memberData;
  static std::string actorName;
  static idgs::actor::ActorDescriptorPtr descriptor;
};

} /* namespace tpc */
} /* namespace idgs */
