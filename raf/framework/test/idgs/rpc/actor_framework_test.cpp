
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include <gtest/gtest.h>
#include "idgs/actor/rpc_framework.h"
#include "idgs/actor/actor_message_queue.h"

using namespace idgs::actor;


namespace idgs {
namespace actor {
namespace actor_framework_test {
static int count_ = 0;

class TestStatefulActor: public StatefulActor {
public:

  TestStatefulActor() {}

  ~TestStatefulActor() {
    msg_queue.clear();
  }

  /// @todo
  virtual const idgs::actor::ActorDescriptorPtr& getDescriptor() const {
    static idgs::actor::ActorDescriptorPtr nullDesc(NULL);
    DLOG(WARNING) << __FUNCTION__ << " should be overridden, class: " << typeid(*this).name();
    return nullDesc;
  }

  const std::string& getActorName() const override{
    static const std::string actorName("TestStatefulActor");
    return actorName;
  }



  void process(const ActorMessagePtr& msg) override{
    DVLOG(1) << "Stateful Actor process is called" << std::endl;
    count_++;
  }
};

class TestStatelessActor: public StatelessActor {
public:
  void process(const ActorMessagePtr& msg) override {
    DVLOG(1) << "Stateless Actor process is called" << std::endl;
    count_++;
  }
};

}
}
}

using namespace idgs::pb;
using namespace idgs::actor::actor_framework_test;
using namespace std;

TEST(ActorManager, statfull)
{
  DVLOG(1)<<"--------------------- TEST(ActorManager, TestStatefulActor) -----------------" << endl;

  ActorManager actorHandle;

  TestStatefulActor* statefulActor1 = new TestStatefulActor();
  string id1 = actorHandle.generateActorId();
  DVLOG(1) << "generate id1: " << id1 << endl;
  actorHandle.registerSessionActor(id1, statefulActor1);

  TestStatefulActor* statefulActor2 = new TestStatefulActor();
  string id2 = actorHandle.generateActorId();
  DVLOG(1) << "generate id2: " << id2 << endl;
  actorHandle.registerSessionActor(id2, statefulActor2);

  // test actor 2 put and get
  StatefulActor *testActor;
  testActor = (idgs::actor::StatefulActor*)actorHandle.getActor(id2);
  DVLOG(1) << "testActor : " << testActor << endl;
  DVLOG(1) << "statefulActor2 : " << statefulActor2 << endl;
  ASSERT_EQ(testActor, statefulActor2);

  ActorMessagePtr msg;
  testActor->process(msg);
  ASSERT_EQ(1, count_);
  DVLOG(1) << " count_ equals " << count_ << endl;

  // test actor 1 put and get
  testActor = (idgs::actor::StatefulActor*)actorHandle.getActor(id1);
  DVLOG(1) << "testActor : " << testActor << endl;
  DVLOG(1) << "statefulActor1 : " << statefulActor1 << endl;
  ASSERT_EQ(testActor, statefulActor1);

  testActor->process(msg);
  ASSERT_EQ(2, count_);
  DVLOG(1) << " count_ equals " << count_ << endl;

  //unregister actor 1
  DVLOG(1) << "actorHandle -> " << &actorHandle << endl;
  actorHandle.unregisterSessionActor(id1);
  DVLOG(1) << " actor is unRegistered " << endl;
  testActor = (idgs::actor::StatefulActor*)actorHandle.getActor(id1);
  DVLOG(1) << "after unregister actor 1, the test actor is " << testActor << endl;
  ASSERT_EQ(testActor, (StatefulActor *)NULL);

  testActor = (idgs::actor::StatefulActor*)actorHandle.getActor(id2);
  DVLOG(1) << "after unregister actor 1, the test actor 2 is " << testActor << endl;
  ASSERT_EQ(testActor, statefulActor2);
  count_ = 0;
}

TEST(ActorManager, stateless)
{
  DVLOG(1)<<"--------------------- TEST(ActorManager, TestStatelessActor) -----------------" << endl;
  ActorManager actorHandle;

  TestStatelessActor* statelessActor1 = new TestStatelessActor();
  string ope1 = "operation_1";
  actorHandle.registerServiceActor(ope1, statelessActor1);

  TestStatelessActor* statelessActor2 = new TestStatelessActor();
  string ope2 = "operation_2";
  actorHandle.registerServiceActor(ope2, statelessActor2);

  // test actor 2 get and process
  StatelessActor *testActor;
  testActor = (idgs::actor::StatelessActor*) actorHandle.getActor(ope2);
  DVLOG(1) << "testActor : " << testActor << endl;
  DVLOG(1) << "statelessActor2 : " << statelessActor2 << endl;
  ASSERT_EQ(testActor, statelessActor2);

  ActorMessagePtr msg;
  testActor->process(msg);
  ASSERT_EQ(1, count_);
  DVLOG(1) << " count_ equals " << count_ << endl;

  // test actor 1 get and process
  testActor = (idgs::actor::StatelessActor*) actorHandle.getActor(ope1);
  DVLOG(1) << "testActor : " << testActor << endl;
  DVLOG(1) << "statelessActor1 : " << statelessActor1 << endl;
  ASSERT_EQ(testActor, statelessActor1);

  testActor->process(msg);
  ASSERT_EQ(2, count_);
  DVLOG(1) << " count_ equals " << count_ << endl;

  //unregister actor 1
  DVLOG(1) << "actorHandle -> " << &actorHandle << endl;
  actorHandle.unregisterServiceActor(ope1);
  DVLOG(1) << " actor is unRegistered " << testActor << endl;
  testActor = (idgs::actor::StatelessActor*) actorHandle.getActor(ope1);
  DVLOG(1) << "after unregister actor 1, the test actor is " << testActor << endl;
  ASSERT_EQ(testActor, (StatelessActor *)NULL);

  testActor = (idgs::actor::StatelessActor*) actorHandle.getActor(ope2);
  DVLOG(1) << "after unregister actor 1, the test actor 2 is " << testActor << endl;
  ASSERT_EQ(testActor, statelessActor2);

  count_=0;
}

TEST(ActorMessageQueue, push_pop) {

  std::shared_ptr<RpcMessage> msg_ptr = std::make_shared<RpcMessage>();
  msg_ptr->set_operation_name("add the message");
  msg_ptr->mutable_dest_actor()->set_actor_id("test_server_id");
  msg_ptr->mutable_source_actor()->set_actor_id("sourceactorid");
  msg_ptr->mutable_dest_actor()->set_member_id(1);
  msg_ptr->set_channel(TC_TCP);
  msg_ptr->mutable_source_actor()->set_member_id(1);
  msg_ptr->set_payload("payload");

  std::shared_ptr<ActorMessage> actorMsg(new ActorMessage(msg_ptr));

  ActorMessageQueue queue;
  //rw->setNetworkActorId("5");
  queue.push(actorMsg);
  //VLOG(1) << rw->getNetworkActorId() << std::endl;
  DVLOG(2) << "before push ActorMessage" << actorMsg->toString();

  std::shared_ptr<ActorMessage> actorMsg2;
  queue.try_pop(&actorMsg2);
  DVLOG(2) << "after pop ActorMessage " << actorMsg2->getRpcMessage()->payload();

  //VLOG(1) << "popped network actor id " << result->getNetworkActorId() << std::endl;
  //ASSERT_EQ("5", result->getNetworkActorId());
}

