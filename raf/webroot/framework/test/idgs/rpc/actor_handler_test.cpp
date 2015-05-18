
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/

#include <gtest/gtest.h>

#include "idgs/actor/stateful_actor.h"


class TestHandlerActor: public idgs::actor::StatefulActor {
public:
  virtual const std::string& getActorName() const override {
    static std::string name = "TestHandlerActor";
    return name;

  }
  virtual const idgs::actor::ActorDescriptorPtr& getDescriptor() const override {
    static idgs::actor::ActorDescriptorPtr actorDesc;
    return actorDesc;
  }

  void foo(const idgs::actor::ActorMessagePtr& msg) {
    function_footprint();
    lastOperation = msg->getOperationName();
    LOG(INFO) << "foo";
  }

  void bar(const idgs::actor::ActorMessagePtr& msg) {
    function_footprint();
    lastOperation = msg->getOperationName();
    LOG(INFO) << "bar";
  }

  virtual const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override {
    static idgs::actor::ActorMessageHandlerMap handlerMap = {
        {"foo", {
            static_cast<idgs::actor::ActorMessageHandler>(&TestHandlerActor::foo),
            NULL,
        }},
        {"bar", {
            static_cast<idgs::actor::ActorMessageHandler>(&TestHandlerActor::bar),
            NULL,
        }},
    };
    return handlerMap;
  }

  std::string lastOperation;

};

TEST(actor_handler, dummy) {
  function_footprint();
  TestHandlerActor a;

  idgs::actor::ActorMessagePtr msg = std::make_shared<idgs::actor::ActorMessage>();
  msg->setOperationName("foo");
  a.process(msg);
  LOG(INFO) << "Last operation: " << a.lastOperation;

  msg->setOperationName("bar");
  a.process(msg);
  LOG(INFO) << "Last operation: " << a.lastOperation;
}

