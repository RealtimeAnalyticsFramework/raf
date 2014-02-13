
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/application.h"
#include <gtest/gtest.h>
#include "idgs/signal_handler.h"

using namespace idgs;
using namespace idgs::pb;
using namespace idgs::actor;
namespace {
  struct ApplicationSetting {
    ApplicationSetting():clusterConfig("") {}
    std::string clusterConfig;
  };
}

namespace idgs {
  namespace net {
    namespace tcp_test {

      tbb::atomic<int> tcp_test_count;
      const std::string test_server_id = "test_server_id";
      class TestStatelessActor: public StatelessActor {
        public:
          TestStatelessActor() {
            setActorId(test_server_id);
            ActorDescriptorPtr descriptor(new ActorDescriptorWrapper);
            descriptor->setName("test_server_id");
            descriptor->setDescription("");
            descriptor->setType(AT_STATELESS);
            ActorOperationDescriporWrapper dataSyncRequest;
            dataSyncRequest.setName("test");
            dataSyncRequest.setDescription("");
            dataSyncRequest.setPayloadType("idgs.pb.Long");
            descriptor->setInOperation(dataSyncRequest.getName(), dataSyncRequest);
            ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().registerActorDescriptor(descriptor->getName(), descriptor);
            this->descriptor = descriptor;
          }

          const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override {
            static idgs::actor::ActorMessageHandlerMap map;
            return map;
          }

          void innerProcess(const ActorMessagePtr& msg) override  {
            DLOG(INFO) << msg->getRpcMessage()->DebugString();
            tcp_test_count.fetch_and_add(1);
            std::string payload = msg->getRpcMessage()->payload();
            ASSERT_EQ("payload", payload);
            DLOG(INFO)<< "Stateless Actor process is called, the count is " << (tcp_test_count);
            //ASSERT_EQ(idgs::pb::TransportChannel::TC_ACTO, msg->getChannel());
            std::shared_ptr<ActorMessage> response = msg->createResponse();
            std::shared_ptr<idgs::pb::Long> L(new idgs::pb::Long());
            L->set_value(tcp_test_count.load());
            response->setPayload(L);
	          response->setOperationName("test");
	          idgs::actor::sendMessage(response);
          }
        };
    }
  }
}




using idgs::net::tcp_test::TestStatelessActor;
using idgs::net::tcp_test::tcp_test_count;

Application& startApp() {
  ApplicationSetting setting;
  // default value
  setting.clusterConfig = "framework/conf/cluster.conf";

  Application& app = ::idgs::util::singleton<Application>::getInstance();
  ResultCode rc;

  LOG(INFO) << "Loading configuration.";
  rc = app.init(setting.clusterConfig);
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to initialize server: " << getErrorDescription(rc);
    exit(1);
  }

  LOG(INFO) << "Server is starting.";
  rc = app.start();
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to start server: " << getErrorDescription(rc);
    exit(1);
  }

			 
  LOG(INFO) << "";
  SignalHandler sh;
  sh.setup();

  return app;
}

TEST(tcp, tcp_server) {

  tcp_test_count.store(0);
  Application& app = startApp();

  TestStatelessActor* statelessActor1 = new TestStatelessActor();
  app.regsiterActor(statelessActor1);

  ResultCode rc;
  // dead loop
  while(app.isRunning()) {
    sleep(1);
  }

  //sleep(20);

  LOG(INFO) << "Server is shutting down.";
  rc = app.stop();

  ASSERT_EQ(RC_SUCCESS, rc);
  ASSERT_EQ(tcp_test_count.load(), 2);
  LOG(INFO) << "Server 2 is shutting down";
}


