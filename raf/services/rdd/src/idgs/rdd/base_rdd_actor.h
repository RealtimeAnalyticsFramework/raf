
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <map>
#include <queue>
#include <vector>
#include <string.h>

#include "idgs/actor/rpc_framework.h"
#include "idgs/actor/stateful_actor.h"
#include "idgs/actor/actor_descriptor.h"
#include "idgs/rdd/action/rdd_action.h"
#include "idgs/rdd/rdd_info.h"
#include "idgs/rdd/pb/rdd_common.pb.h"
#include "idgs/pb/rpc_message.pb.h"
#include "idgs/store/pb/metadata.pb.h"

#include "google/protobuf/message.h"

#include "idgs/rdd/rdd_snapshot.h"


namespace idgs {
namespace rdd {

static std::vector<std::string> RDD_STATES_ARRAY =
   {"INIT","PREPARED","DEPENDENCY_PROCESSING","PROCESSING","TRANSFORM_COMPLETE","READY","ERROR"};

/// base class for all RDD
class BaseRddActor: public idgs::actor::StatefulActor {
public:

  /// @brief Constructor
  BaseRddActor();

  /// @brief Destructor
  virtual ~BaseRddActor();

  /// @brief  Add depending RDD.
  /// @param  rddActorId RDD actor ID.
  void addDependingRdd(const idgs::pb::ActorId& rddActorId, const std::string& rddName="");

  std::vector<RddInfo>& getDependingRdd();

  /// @brief  Add depended RDD.
  /// @param  rddActorId RDD actor ID.
  void addDependedRdd(const idgs::pb::ActorId& rddActorId);

  std::vector<idgs::pb::ActorId>& getDependedRdd();

  /// @brief  Get the state of RDD.
  /// @return The state of RDD.
  const idgs::rdd::pb::RddState& getRddState() const;

  /// @brief  Get the action information of RDD with the given action ID.
  /// @param  actionId Action ID.
  /// @return The action information.
  const action::RddActionPtr& getRddAction(const std::string& actionId) const;

  /// @brief  Remove the action information of RDD with the given action ID.
  /// @param  actionId Action ID.
  void removeAction(const std::string& actionId);

  /// @brief  Set the state of partition when handle action.
  /// @param  actionId  Action ID.
  /// @param  partition Partition of store.
  /// @param  state     State of action partition.
  void setActionPartitionState(const std::string& actionId, const uint32_t& partition, pb::RddState state);

  void setRddName(const std::string& rddName) {
    name = rddName;
    rddInfo.setRddName(rddName);
  }

  const std::string& getRddName() const {
    return name;
  }

  int32_t getLocalMemberId() const {
    return localMemberId;
  }

  std::shared_ptr<RddSnapshot> takeSnapShot();

protected:
  /// protobuf type of key.
  std::string keyType;

  /// protobuf type of key.
  std::string valueType;

  /// size of partition
  static size_t partitionSize;

  /// local member id
  static int32_t localMemberId;

  /// to backup client message
  idgs::actor::ActorMessagePtr rawMsg;

  /// to backup transform message
  idgs::actor::ActorMessagePtr transformMsg;

  /// the memeber id and actor id of depended rdds
  std::vector<idgs::pb::ActorId> dependedRdds;

  RddInfo rddInfo;

  std::shared_ptr<idgs::store::pb::MetadataPair> metadata;

  /// the memeber id and actor id with rdd state of depending rdds
  std::vector<RddInfo> dependingRdds;

  /// action queue
  std::queue<action::RddActionPtr> actionQueue;

  /// the actions of rdd
  std::map<std::string, action::RddActionPtr> rddAction;

  /// @brief  Generate descriptor for Base.
  /// @param  The name of current RDD actor.
  /// @return The descriptor for Base
  static idgs::actor::ActorDescriptorPtr generateBaseActorDescriptor(const std::string& actorName);

private:
  size_t dependingRddResponse, partitionResponse;
  std::string name;

  void setDependingRddState(const idgs::pb::ActorId& actorId, const pb::RddState& state);
  void processRddPrepared();
  void processRddReady();

  void onDestroy() override;

protected:

  void handleRddCreatePartitionResponse(const idgs::actor::ActorMessagePtr& msg);
  void handleGetPartitionActor(const idgs::actor::ActorMessagePtr& msg);
  void handleReceiveDependedRdd(const idgs::actor::ActorMessagePtr& msg);
  void handleGetPartitionActorResponse(const idgs::actor::ActorMessagePtr& msg);
  void handleSendRddInfoResponse(const idgs::actor::ActorMessagePtr& msg);
  void handleTransformComplete(const idgs::actor::ActorMessagePtr& msg);
  void handlePartitionPreparedResposne(const idgs::actor::ActorMessagePtr& msg);

  void handleRddTransform(const idgs::actor::ActorMessagePtr& msg);
  void handleDependingRddReady(const idgs::actor::ActorMessagePtr& msg);
  void handleRddProcess(const idgs::actor::ActorMessagePtr& msg);
  void handleRddPartitionReady(const idgs::actor::ActorMessagePtr& msg);

  void handleRddStateRequest(const idgs::actor::ActorMessagePtr& msg);
  void handleRddStateResponse(const idgs::actor::ActorMessagePtr& msg);

  void handleRddActionRequest(const idgs::actor::ActorMessagePtr& msg);
  void handleRddActionResponse(const idgs::actor::ActorMessagePtr& msg);
  void handleActionProcess(const idgs::actor::ActorMessagePtr& msg);

  void handleInsertRDDInfoResponse(const idgs::actor::ActorMessagePtr& msg);
};

} // namespace rdd
} // namespace idgs 
