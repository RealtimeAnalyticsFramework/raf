
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $

#include "client_test.h"


const std::string test_server_id = "TimeOutStatelessActor";
class TimeOutStatelessActor: public StatelessActor {
  public:
    TimeOutStatelessActor() {
      setActorId(test_server_id);
    }
    void process(const ActorMessagePtr& msg) override {
    }
    const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override {
      static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap ;
      return handlerMap;
    }

};

bool running;
ResultCode code;

TEST(tcp, asynch_tcp_client) {
  ::sleep(5);

  ApplicationSetting setting1;
  setting1.clusterConfig = "framework/conf/cluster.conf";

  TimeOutStatelessActor* actor = new TimeOutStatelessActor();

  pid_t server1 = startApp(setting1, actor, 60, defaultChecker);
  if (server1 == 0) {
    return;
  }

  ::sleep(5);


  ClientSetting setting;
  setting.clientConfig = "integration_test/client_it/ut_test_four_clients.conf";
  setting.storeConfig = "integration_test/client_it/data_store.conf";

  code = ::idgs::util::singleton<TcpClientPool>::getInstance().loadConfig(setting);
  ASSERT_EQ(code, RC_SUCCESS);

  ClientActorMessagePtr response;
  std::thread m_thread(std::bind(::sendMessage, std::ref(code), test_server_id, std::ref(response)));

  ::sleep(5);

  DVLOG(2) << ("before check joinable");

  if (!m_thread.joinable()) {
    LOG(ERROR) << " client is still waiting for response, error!";
    ASSERT_FALSE(1);
  }

  m_thread.join();

  DVLOG(2) << ("after check joinable");

  ASSERT_EQ(code, RC_ERROR);

  ::sleep(1);

  kill(server1, 9);
  int status;
  waitpid(server1, &status, 0);
}


