
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/actor/stateful_actor.h"
#include "idgs/store/datastore_const.h"
#include "idgs/store/pb/store_service.pb.h"

namespace idgs {
namespace store {

/// The stateful actor
/// To aggregate response and handle result.
class AggregatorActor: public idgs::actor::StatefulActor {
protected:

  /// @brief  Multicast message to all members.
  /// @param  msg     Actor message from request.
  /// @param  actorId The actor id of destination.
  /// @param  opName  The operation name to handle.
  /// @return size of multicast members
  void multicast(const idgs::actor::ActorMessagePtr& msg, const std::string& actorId, const std::string& opName);

  /// @brief  Send response message to client.
  /// @param  opName    The operation name to handle.
  /// @param  response  The actor message of protobuf response to send.
  void sendResponse(const std::string& opName, const std::shared_ptr<google::protobuf::Message>& response);

  /// The actor message from client.
  idgs::actor::ActorMessagePtr clientMsg;
  std::atomic_int requestCount;
  std::atomic_int responseCount;
};

/// The stateful actor
/// To aggregate insert data response and send result back to client.
class DataAggregatorActor: public AggregatorActor {
public:

  /// @brief  Generate descriptor for DataAggregatorActor.
  /// @return The descriptor for DataAggregatorActor.
  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

  /// @brief  Get descriptor for DataAggregatorActor.
  /// @return The descriptor for DataAggregatorActor.
  const idgs::actor::ActorDescriptorPtr& getDescriptor() const;

  const std::string& getActorName() const {
    static std::string actorName = DATA_AGGREGATOR_ACTOR;
    return actorName;
  }

  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

private:
  void handleGlobalInsert(const idgs::actor::ActorMessagePtr& msg);
  void handleInsertResponse(const idgs::actor::ActorMessagePtr& msg);
  void handleGlobalUpdate(const idgs::actor::ActorMessagePtr& msg);
  void handleUpdateResponse(const idgs::actor::ActorMessagePtr& msg);
  void handleGlobalDelete(const idgs::actor::ActorMessagePtr& msg);
  void handleDeleteResponse(const idgs::actor::ActorMessagePtr& msg);
  void handleGlobalTruncate(const idgs::actor::ActorMessagePtr& msg);
  void handleTruncateResponse(const idgs::actor::ActorMessagePtr& msg);
  void handleGlobalCount(const idgs::actor::ActorMessagePtr& msg);
  void handleCountResponse(const idgs::actor::ActorMessagePtr& msg);

private:
  std::map<int32_t, size_t> dataSizeMap;
  idgs::store::pb::StoreResultCode resultCode;
  static idgs::actor::ActorDescriptorPtr descriptor;
};

} // namespace store
} // namespace idgs
