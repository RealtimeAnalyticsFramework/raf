
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <queue>


#include "idgs/rdd/rdd_local.h"

#include "idgs/rdd/action/rdd_action.h"

namespace idgs {
namespace rdd {

/// base class for all RDD
class BaseRddActor: public idgs::actor::StatefulActor {
public:
  BaseRddActor();
  virtual ~BaseRddActor();

public:

  const std::string& getRddName() const;

  void setRddLocal(RddLocal* rddlocal);

  void onDestroy() override;

  void onDownstreamRemoved();

protected:
  /// action queue
  std::queue<action::RddActionPtr> actionQueue;
  std::map<std::string, action::RddActionPtr> rddActions;

  RddLocal* rddLocal = NULL;
  bool active = true;

  std::string rddName;

  static size_t partitionSize;
  static uint32_t localMemberId;

protected:

  /// @brief  Generate descriptor for Base.
  /// @param  The name of current RDD actor.
  /// @return The descriptor for Base
  static idgs::actor::ActorDescriptorPtr generateBaseActorDescriptor(const std::string& actorName);

  void handleCreateRddPartitionResponse(const idgs::actor::ActorMessagePtr& msg);
  void handleRddPartitionPrepared(const idgs::actor::ActorMessagePtr& msg);

  void handleRddTransform(const idgs::actor::ActorMessagePtr& msg);
  void handleRddTransformPrepared(const idgs::actor::ActorMessagePtr& msg);
  void handlePartitionTransformComplete(const idgs::actor::ActorMessagePtr& msg);
  void handlePartitionReady(const idgs::actor::ActorMessagePtr& msg);
  void handleUpstreamReady(const idgs::actor::ActorMessagePtr& msg);
  void handleUpstreamError(const idgs::actor::ActorMessagePtr& msg);

  void handleRddActionRequest(const idgs::actor::ActorMessagePtr& msg);
  void handleActionProcess(const idgs::actor::ActorMessagePtr& msg);
  void handleRddActionResponse(const idgs::actor::ActorMessagePtr& msg);

  void callUpstreamTransform();
  void processRddReady();
  void processRddError();
  void processRddStateChanged();
  void multicastRddMessage(const std::string& opName, const idgs::actor::PbMessagePtr& payload);
};

} // namespace rdd
} // namespace idgs 
