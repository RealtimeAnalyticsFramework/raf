
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

extern const char OP_DESTROY[];
extern const char OP_ACTOR_NOT_FOUND[];

class Actor;
typedef void (idgs::actor::Actor:: * ActorMessageHandler) (const idgs::actor::ActorMessagePtr& msg);

struct OperationDescriptor {
  const ActorMessageHandler handler;
  const google::protobuf::Message* payload;

  OperationDescriptor (const idgs::actor::ActorMessageHandler handler_) : handler(handler_), payload(NULL) {
  }

  OperationDescriptor(const idgs::actor::ActorMessageHandler handler_, const google::protobuf::Message* payload_) :
      handler(handler_), payload(payload_) {
  }

  OperationDescriptor () : handler(NULL), payload(NULL) {
  }
};

typedef std::map<std::string, idgs::actor::OperationDescriptor> ActorMessageHandlerMap;

enum ProcessStatus {
  DEFAULT,
  TERMINATE
};

class Actor {
public:
  Actor() {}
  virtual ~Actor() {}

  /// Unique ID of actor instance in a member.
  /// A actor instance is identified as [member_id, actor_id].
  inline const std::string& getActorId() const {
    return actorId;
  }

  /// Actor's user friend name, e.g. class name.
  /// All actor instances of same class share the same name.
  virtual const std::string& getActorName() const = 0;

  /// terminate itself
  void terminate();

  /// terminate a actor by sending a 'DESTROY' message to the actor.
  void terminate(const std::string& actor_id, int member_id = idgs::pb::ANY_MEMBER);

public:
  virtual const idgs::actor::ActorDescriptorPtr& getDescriptor() const = 0;

  virtual const ActorMessageHandlerMap& getMessageHandlerMap() const {
    static idgs::actor::ActorMessageHandlerMap handlerMap;
    return handlerMap;
  }

  virtual bool stateful() = 0;

  /// create an ActorMessage whose source actor is this actor
  /// @return the ActorMessage
  virtual std::shared_ptr<ActorMessage> createActorMessage() const;

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

  /// This method should be invoked by process, and should NEVER called directly by other method.
  /// User usually needn't override this method while the logic should be put into ActorMessageHandler
  /// @see #process
  /// @see #ActorMessageHandler
  virtual ProcessStatus innerProcess(const ActorMessagePtr& msg);

  /// Unregister actor and release all resource. Delete itself when necessary.
  /// This method is called when a 'DESTROY' message arrives, user logic should never call it directly.
  /// @see #terminate
  virtual void onDestroy() = 0;

protected:
  std::string actorId;
};

} // namespace rpc
} // namespace idgs
