
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $

#include "client_test.h"


const std::string mtest_server_id = "MultiThreadsClientStatelessActor";

class MultiThreadsClientStatelessActor: public StatelessActor {
  public:
    MultiThreadsClientStatelessActor() {
      setActorId(mtest_server_id);
      count.store(0);
    }

    ~MultiThreadsClientStatelessActor() {
      LOG(WARNING)<<"MultiThreadsClientStatelessActor is call with count " << count.load();
      //ASSERT_EQ(count.load(), 3000);
    }

    const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override {
      static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap;
      return handlerMap;
    }


    void process(const ActorMessagePtr& msg) override {
      DVLOG(2) << "MultiThreadsClientStatelessActor is called";
      /*{
          std::lock_guard<std::mutex> lockGuard(lock);
      }*/
      count.fetch_add(1);
      ActorMessagePtr response = msg->createResponse();
      std::shared_ptr<idgs::pb::Long> rsp = std::make_shared<idgs::pb::Long>();
      rsp->set_value(count.load());
      response->setPayload(rsp);
      response->setOperationName("test");
      idgs::actor::sendMessage(response);
    }
  private:
    std::atomic<int> count;
    std::mutex lock;
};

TEST(asynch_client_multi_threads, asynch_tcp_client_multi_threads) {
  std::atomic<int> count;
  ResultCode code;
  ::sleep(5);

  ApplicationSetting setting1;
  setting1.clusterConfig = "conf/cluster.conf";

  code = RC_SUCCESS;

  MultiThreadsClientStatelessActor* actor = new MultiThreadsClientStatelessActor();

  pid_t server1 = startApp(setting1, actor, 30, defaultChecker);
  if (server1 == 0) {
    LOG(ERROR) << "child process exit";
    exit(1);
  }

  ::sleep(5);


  ClientSetting setting;
  setting.clientConfig = "integration_test/client_it/test_client.conf";
  setting.storeConfig = "conf/data_store.conf";

  code = getTcpClientPool().loadConfig(setting);
  ASSERT_EQ(code, RC_SUCCESS);

  vector<thread> threadPool;
  threadPool.reserve(10);
  int THREAD_COUNT = 3;
  int TIMES_COUNT = 10000;
  for(int i = 0; i < THREAD_COUNT; i++) {
    threadPool.push_back(thread([&](){
      ClientActorMessagePtr response;
      int times = 0;
      while(times < TIMES_COUNT) {
        sendMessage(code, mtest_server_id, response);
        std::shared_ptr<idgs::pb::Long> rsp = std::make_shared<idgs::pb::Long>();
        ASSERT_EQ(code, RC_SUCCESS);
        if (response.get()) {
          if (response->getRpcMessage()) {
            response->parsePayload(rsp.get());
            count.store(rsp->value());
            times++;
          } else {
            LOG(ERROR) << "can not get response->getRpcMessage()";
          }
        } else {
          LOG(ERROR) << "can not get response.get()";
        }
      }
    }));
  }


  for(auto it = threadPool.begin(); it != threadPool.end(); ++it) {
    it->join();
  }
  threadPool.clear();

  ::sleep(1);

  LOG(WARNING)<< "get response count " << count.load();
  ASSERT_EQ(THREAD_COUNT * TIMES_COUNT, count.load());

  kill(server1, 9);
  ::sleep(2);

  int status;
  waitpid(server1, &status, WEXITED|WSTOPPED);
  LOG(WARNING) << "get child process status " << status;
}


