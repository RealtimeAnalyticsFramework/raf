/*
 * save_action_action.h
 *
 *  Created on: Jan 9, 2014
 *      Author: root
 */


namespace idgs {
namespace rdd {
namespace action {
namespace actor {

class SaveActionActor : public idgs::actor::StatelessActor {
public:

  SaveActionActor();
  virtual ~SaveActionActor();

  static std::shared_ptr<idgs::actor::ActorDescriptorWrapper> generateActorDescriptor();

  virtual const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

private:
  void handleSave(const idgs::actor::ActorMessagePtr& msg);
};

}
}
}
}
