/*
 * listener_manager.h
 *
 *  Created on: Feb 11, 2014
 *      Author: root
 */
#pragma once

#include "idgs/actor/actor_message.h"
#include "idgs/actor/stateless_actor.h"
#include "idgs/store/datastore_const.h"

namespace idgs {
namespace store {

class ListenerManager: public idgs::actor::StatelessActor {
public:

  /// @brief  Construction.
  /// @param  actorId Actor id
  ListenerManager(const std::string& actorId);

  /// @brief  Destruction.
  virtual ~ListenerManager();

  /// @brief  Generate descriptor for DataStoreStatelessActor.
  /// @return The descriptor for DataStoreStatelessActor
  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

  const std::string& getActorName() const override {
    static std::string actorName = LISTENER_MANAGER;
    return actorName;
  }

  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

private:

  void handleListenerInsert(const idgs::actor::ActorMessagePtr& msg);
  void handleListenerUpdate(const idgs::actor::ActorMessagePtr& msg);
  void handleListenerGet(const idgs::actor::ActorMessagePtr& msg);
  void handleListenerDelete(const idgs::actor::ActorMessagePtr& msg);
  void handleListenerTruncate(const idgs::actor::ActorMessagePtr& msg);

};

} /* namespace store */
} /* namespace idgs */
