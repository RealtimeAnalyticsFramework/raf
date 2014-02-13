
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $


namespace idgs {
  namespace rpc {
    namespace multicast_test {
      const std::string test_server_id = "test_server_id";
      const std::string start_work_operation = "start_work_operation";
      const std::string multicast_operation = "multicast_operation";
      int test_server_count = 0;

      class TestStatelessActor: public StatelessActor {
          typedef std::function<void(idgs::actor::ActorMessagePtr msg)> ProcessType;
        public:
          void init() {
            DLOG(INFO)<< "init actor " << test_server_id;
            this->setActorId(test_server_id);

            idgs::actor::ActorOperationMap& opMap = getActorOperationMap();

            ProcessType startTimerProcess =
            [&] (idgs::actor::ActorMessagePtr msg) {
              DLOG(INFO) << "TestStatelessActor receive operation: start_work_operation";
              ++test_server_count;

              ActorMessagePtr multMsg(new ActorMessage());
              multMsg->setSourceActorId(test_server_id);
              multMsg->setDestActorId(test_server_id);
              multMsg->setOperationName(multicast_operation);

              int32_t localMemId = ::idgs::util::singleton<RpcMemberListener>::getInstance().getLocalMemberId();
              multMsg->setDestMemberId(localMemId);
              multMsg->setSourceMemberId(localMemId);
              multMsg->setChannel(TC_MULTICAST);
              ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework()->send(multMsg);
            };

            ProcessType multMessageProcess =
            [&] (idgs::actor::ActorMessagePtr msg) {
              DLOG(INFO) << "TestStatelessActor receive operation: multicast_operation with channel " << msg->getChannel();

              if (msg->getChannel() == TC_MULTICAST) {
                ++test_server_count;
              }
            };

            opMap[start_work_operation] = startTimerProcess;
            opMap[multicast_operation] = multMessageProcess;

            startTime(2000, start_work_operation);
          }
        private:
          std::shared_ptr<ScheduledFuture> startTime(TimerType timer_dur, const std::string& operName) {
            ActorMessagePtr timeOutMsg(new ActorMessage());
            timeOutMsg->setSourceActorId(test_server_id);
            timeOutMsg->setDestActorId(test_server_id);
            timeOutMsg->setOperationName(operName);
            ScheduledMessageService& service = ::idgs::util::singleton<RpcFramework>::getInstance().getScheduler();
            return service.schedule(timeOutMsg, timer_dur);
          }
        };
    }
  }
}


void startServer() {
  ApplicationSetting setting;
  setting.clusterConfig = "framework/conf/cluster.conf";

  Application& app = ::idgs::util::singleton<Application>::getInstance();
  ResultCode rc;
  rc = app.init(setting.clusterConfig);
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to initialize server: " << getErrorDescription(rc);
    exit(1);
  }
  rc = app.start();
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to start server: " << getErrorDescription(rc);
    exit(1);
  }
  SignalHandler sh;
  sh.setup();
}

TEST(scheduler_test, scheduler_actor_test) {

  /// start server
  startServer();

  //////////////// Start Test ////////////////////////////////////////////////////
  TestStatelessActor *actor = new TestStatelessActor();
  actor->init();
  RpcFramework& rpc = ::idgs::util::singleton<RpcFramework>::getInstance();
  rpc.getActorFramework()->Register(actor->getActorId(),actor);
  rpc.getScheduler().start();
  DLOG(INFO) << "starting test.";
  ::sleep(3);
  DLOG(INFO) << "finish test.";
  ASSERT_EQ(2, test_server_count);


  /// stop server
  Application& app = ::idgs::util::singleton<Application>::getInstance();
  app.stop();

  DLOG(INFO) << " Multicast Test is over";
}
