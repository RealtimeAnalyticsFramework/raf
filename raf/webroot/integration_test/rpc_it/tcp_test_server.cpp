
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
using namespace idgs::actor;
namespace {
struct ApplicationSetting {
  ApplicationSetting():clusterConfig("") {}
  std::string clusterConfig;
};
}


std::atomic<int> tcp_test_count;
const char test_server_id[] = "test_server_id";
class TestStatelessActor: public StatelessActor {
public:
  TestStatelessActor() {
    setActorId(test_server_id);
  }
  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override {
    static idgs::actor::ActorMessageHandlerMap map;
    return map;
  }

  ProcessStatus innerProcess(const ActorMessagePtr& msg) override {
    tcp_test_count.fetch_add(1);
    DLOG(INFO)<< "Stateless Actor process is called, the count is " << (tcp_test_count);
    std::string payload = msg->getRpcMessage()->payload();
    DVLOG(3) << "payload: " << payload;

    std::shared_ptr<ActorMessage> routeMessage = msg->createRouteMessage(1, test_server_id);
    idgs::actor::sendMessage(routeMessage);
    return DEFAULT;
  }
};

Application& startApp() {
  ApplicationSetting setting;
  // default value
  setting.clusterConfig = "conf/cluster.conf";

  Application& app = *idgs_application();
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

TEST(tcp_test_server, tcp_server) {

  tcp_test_count.store(0);
  Application& app = startApp();

  TestStatelessActor* statelessActor1 = new TestStatelessActor();
  app.registerServiceActor(statelessActor1);

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
}


