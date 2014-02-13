
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

      tbb::atomic<int> test_server_count;
      const std::string test_server_id = "test_server_id";
      const std::string start_work_operation = "start_work_operation";
      const std::string start_work_operation_resp = "start_work_operation_succesfully";
      const std::string sending_response_timeout = "sending_response_timeout";
      const std::string sending_reponse_succ = "sending_reponse_successfully";
      const std::string waiting_new_message_timeout = "waiting_new_message_timeout";
      const std::string new_client_message_comes = "new_client_message_comes";
      const std::string sending_total = "sending_total";
      const TimerType _timer_dur = 2000;

      class TestStatelessActor: public StatelessActor {
        typedef std::function<void (idgs::actor::ActorMessagePtr msg)> ProcessType;
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

          void init() {
            DLOG(INFO) << "init actor " << test_server_id;
            this->setActorId(test_server_id);
            test_server_count.store(0);
          }

          void startTimerProcess (const idgs::actor::ActorMessagePtr& msg) {
            DLOG(INFO) << "TestStatelessActor receive operation: start_work_operation";
            test_server_count.fetch_and_add(1);

            ActorMessagePtr response = msg->createResponse();

            response->getRpcMessage()->set_operation_name(start_work_operation_resp);

            //Store the last message
            this->lastMsg.reset(new ActorMessage(*response));

            idgs::actor::sendMessage(response);


            lastTimer = startTime(_timer_dur, sending_response_timeout);
            DLOG(INFO) << "started timer";
          };

          void sendRespTimerOutProcess(const idgs::actor::ActorMessagePtr& msg) {
            DLOG(INFO) << "TestStatelessActor is time out";
            test_server_count.fetch_and_add(1);
            DLOG(INFO) << this << "Last Message: " << this->lastMsg->toString();

            ActorMessagePtr newMsg = this->lastMsg->createSessionRequest();
            newMsg->getRpcMessage()->set_operation_name(sending_reponse_succ);
            std::shared_ptr<idgs::pb::Long> L(new idgs::pb::Long());
            L->set_value(test_server_count.load());
            newMsg->setPayload(L);

            DLOG(INFO) << "New Message: " << newMsg->toString();


            idgs::actor::sendMessage(newMsg);

            DLOG(INFO) << "send session message back";
            lastTimer = startTime(4000, waiting_new_message_timeout);
            DLOG(INFO) << "started a new timer";
          };

          void newMsgComesProcess (const idgs::actor::ActorMessagePtr& msg) {
            DLOG(INFO) << "New Client Request comes ";
            test_server_count.fetch_and_add(1);
            lastTimer->cancel();
            DLOG(INFO) << "the timer " << lastTimer->getDispachTime() << ":" << lastTimer.get() << " is canceled";
            ActorMessagePtr newMsg = msg->createResponse();
            newMsg->getRpcMessage()->set_operation_name(sending_total);
            std::shared_ptr<idgs::pb::Long> L(new idgs::pb::Long());
            L->set_value(test_server_count.load());
            newMsg->setPayload(L);
            idgs::actor::sendMessage(newMsg);
          };

          void waitNewMsgTimeOutProcess(const idgs::actor::ActorMessagePtr& msg) {
            DLOG(INFO) << "Can not receive the new message from client";
            test_server_count.fetch_and_add(1);
          };


          const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override {
            static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
                {start_work_operation,                         static_cast<idgs::actor::ActorMessageHandler>(&TestStatelessActor::startTimerProcess)},
                {sending_response_timeout,                         static_cast<idgs::actor::ActorMessageHandler>(&TestStatelessActor::sendRespTimerOutProcess)},
                {waiting_new_message_timeout,                         static_cast<idgs::actor::ActorMessageHandler>(&TestStatelessActor::waitNewMsgTimeOutProcess)},
                {new_client_message_comes,                         static_cast<idgs::actor::ActorMessageHandler>(&TestStatelessActor::newMsgComesProcess)},
            };
            return handlerMap;
          }


        private:
          std::shared_ptr<ScheduledFuture> startTime(TimerType timer_dur, const std::string& operName) {
            ActorMessagePtr timeOutMsg = this->createActorMessage();
            // timeOutMsg->setSourceActorId(test_server_id);
            timeOutMsg->setDestActorId(test_server_id);
            timeOutMsg->setDestMemberId(timeOutMsg->getSourceMemberId());
            timeOutMsg->getRpcMessage()->set_operation_name(operName);
            ScheduledMessageService& service = ::idgs::util::singleton<ScheduledMessageService>::getInstance();
            return service.schedule(timeOutMsg, timer_dur);
          }

          ActorMessagePtr lastMsg;
          std::shared_ptr<ScheduledFuture> lastTimer;
        };
    }
  }
}




using idgs::net::tcp_test::TestStatelessActor;

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

  Application& app = startApp();

  TestStatelessActor* statelessActor1 = new TestStatelessActor();
  statelessActor1->init();
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
}


