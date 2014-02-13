
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include <gtest/gtest.h>
#include "idgs/application.h"
#include "idgs/client/client_pool.h"
#include "idgs/application.h"
#include "idgs/signal_handler.h"

using namespace idgs::client;
using namespace idgs::store;
using namespace idgs::cluster;
using namespace google::protobuf;
using namespace idgs;
using namespace idgs::pb;
using namespace idgs::actor;
using namespace std;


struct ApplicationSetting {
  ApplicationSetting():clusterConfig("") {}
  std::string clusterConfig;
};

void defaultChecker() {

}

template<typename CheckFun>
pid_t startApp(ApplicationSetting& setting, StatelessActor* actor, int time_sec, CheckFun checker = defaultChecker) {
  // default value

  pid_t pid = fork();

  if (pid == 0) { // child process
    Application* app = new Application();
    ResultCode rc;

    LOG(INFO) << "Loading configuration.";
    rc = app->init(setting.clusterConfig);
    if (rc != RC_SUCCESS) {
      LOG(ERROR) << "Failed to initialize server: " << getErrorDescription(rc);
      exit(1);
    }

    app->regsiterActor(actor);

    LOG(INFO) << "Server is starting.";
    rc = app->start();
    if (rc != RC_SUCCESS) {
      LOG(ERROR) << "Failed to start server: " << getErrorDescription(rc);
      exit(1);
    }

    SignalHandler sh;
    sh.setup();

    ::sleep(60);

    checker();

    DVLOG(2)<<" service is going to finish";
  }

  return pid;
}

void sendMessage(ResultCode& code, string test_server_id, ClientActorMessagePtr& response) {
  std::shared_ptr<TcpClientInterface> client;
  client = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(code);
  ASSERT_EQ(code, RC_SUCCESS);

  std::string env_port = getenv(ENV_VAR_PORT);

  ASSERT_EQ(env_port, client->getServerAddress().port());

  idgs::client::ClientActorMessagePtr clientActorMsg(
      new idgs::client::ClientActorMessage);
  // set operation name
  clientActorMsg->setDestActorId(test_server_id);
  clientActorMsg->setOperationName("test");
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setChannel(TC_AUTO);
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->getRpcMessage()->set_payload("payload");

  DVLOG(2) << " try to send actor message";
  response = client->sendRecv(clientActorMsg, &code);
}


