
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/actor/actor_descriptor.h"

namespace idgs {
namespace actor {
//
// System Operations
//

extern const std::string OP_DESTROY;
extern const std::string OP_ACTOR_NOT_FOUND;

class Actor;
typedef void (idgs::actor::Actor:: * ActorMessageHandler) (const idgs::actor::ActorMessagePtr& msg);
typedef std::map<std::string, ActorMessageHandler> ActorMessageHandlerMap;

class Actor {
public:
  Actor() {}
  virtual ~Actor() {}
  inline const std::string& getActorId() const {
    return actorId;
  }
  virtual const std::string& getActorName() const = 0;
  void terminate();
public:
  virtual const idgs::actor::ActorDescriptorPtr& getDescriptor() const = 0;
  virtual const ActorMessageHandlerMap& getMessageHandlerMap() const = 0;

  /// create an ActorMessage whose source actor is this actor
  /// @return the ActorMessage
  std::shared_ptr<ActorMessage> createActorMessage() const;

  /// create an ActorMessage whose source actor is given by the parameter
  /// @return the ActorMessage
  static ActorMessagePtr createActorMessage(const std::string& sourceActorId);

  /// parse only payload here
  /// should be overridden to parse attachments
  ///
  virtual bool parse(ActorMessagePtr& msg);

  /// the business logic to handle the message
  /// @param msg reference to the message
  virtual void process(const ActorMessagePtr& msg);

protected:
  bool handleSystemOperation(const ActorMessagePtr& msg);
  virtual void innerProcess(const ActorMessagePtr& msg);
  virtual void onDestroy() {}


protected:
  std::string actorId;
};

} // namespace rpc
} // namespace idgs
