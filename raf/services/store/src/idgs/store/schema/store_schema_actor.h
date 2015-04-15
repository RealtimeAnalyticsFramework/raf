/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#pragma once

#include "idgs/actor/stateless_actor.h"


#include "idgs/store/storage/aggregator_actor.h"

namespace idgs {
namespace store {

class StoreSchemaActor : public idgs::actor::StatelessActor {
public:
  StoreSchemaActor(const std::string& actorId);
  virtual ~StoreSchemaActor();

  const std::string& getActorName() const override {
    static std::string actorName = ACTORID_SCHEMA_SERVCIE;
    return actorName;
  }

  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

private:
  void handleGlobalRequest(const idgs::actor::ActorMessagePtr& msg);
  void handleShowStores(const idgs::actor::ActorMessagePtr& msg);
  void handleLocalCreateSchema(const idgs::actor::ActorMessagePtr& msg);
  void handleLocalDropSchema(const idgs::actor::ActorMessagePtr& msg);
  void handleLocalCreateStore(const idgs::actor::ActorMessagePtr& msg);
  void handleLocalDropStore(const idgs::actor::ActorMessagePtr& msg);

private:
  pb::StoreResultCode getResultCode(const ResultCode& status);

};

class StoreSchemaAggrActor : public AggregatorActor {
public:
  StoreSchemaAggrActor();
  virtual ~StoreSchemaAggrActor();

  const std::string& getActorName() const override {
    static std::string actorName = DATA_STORE_SCHEMA_AGGR_ACTOR;
    return actorName;
  }

  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

  const idgs::actor::ActorDescriptorPtr& getDescriptor() const override;

  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

private:
  static idgs::actor::ActorDescriptorPtr descriptor;

  idgs::store::pb::StoreResultCode resultCode;

private:
  void handleCreateSchema(const idgs::actor::ActorMessagePtr& msg);
  void handleCreateSchemaResponse(const idgs::actor::ActorMessagePtr& msg);
  void handleDropSchema(const idgs::actor::ActorMessagePtr& msg);
  void handleDropSchemaResponse(const idgs::actor::ActorMessagePtr& msg);
  void handleCreateStore(const idgs::actor::ActorMessagePtr& msg);
  void handleCreateStoreResponse(const idgs::actor::ActorMessagePtr& msg);
  void handleDropStore(const idgs::actor::ActorMessagePtr& msg);
  void handleDropStoreResponse(const idgs::actor::ActorMessagePtr& msg);

private:
  template <typename T>
  void handleMemberResponse(const idgs::actor::ActorMessagePtr& msg) {
    ++ responseCount;

    auto source = std::to_string(msg->getSourceMemberId());
    auto& opName = msg->getOperationName();

    DVLOG(2) << "Receive " << opName << " on member " << source << ", left " << std::to_string(requestCount - responseCount) << " members.";

    auto response = dynamic_cast<T*>(msg->getPayload().get());
    if (response->result_code() != pb::SRC_SUCCESS) {
      resultCode = response->result_code();
      LOG(ERROR) << opName << " error on member " + source << ", caused by " << pb::StoreResultCode_Name(resultCode);
    }

    if (requestCount == responseCount) {
      DVLOG(2) << "All local store members received " << opName << " response, send response to client.";
      auto payload = std::make_shared<T>();
      payload->set_result_code(resultCode);

      sendResponse(opName, payload);
      terminate();
    }
  }
};

} /* namespace store */
} /* namespace idgs */
