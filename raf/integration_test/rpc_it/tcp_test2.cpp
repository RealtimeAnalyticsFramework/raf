
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include <gtest/gtest.h>
#include "idgs/cancelable_timer.h"
#include "idgs/client/client_pool.h"

using namespace idgs::pb;

namespace idgs {
  namespace net {
    namespace tcp_test {
      int count(0);
      const std::string test_server_id = "test_server_id";
      int client_run() {
        using asio::ip::tcp;
        try {
          sleep(5);

          idgs::client::ClientActorMessagePtr clientActorMsg(new idgs::client::ClientActorMessage);
          // set operation name
          clientActorMsg->setOperationName("add the message");
          clientActorMsg->setDestActorId(test_server_id);
          clientActorMsg->setOperationName("test");
          clientActorMsg->setSourceActorId("client_actor_id");
          clientActorMsg->setDestMemberId(ANY_MEMBER);
          clientActorMsg->setChannel(TC_AUTO);
          clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
          clientActorMsg->getRpcMessage()->set_payload("payload");

          // Get TCP Client
          idgs::ResultCode code;
          std::shared_ptr<idgs::client::TcpClientInterface> client =
              ::idgs::util::singleton<idgs::client::TcpClientPool>::getInstance().getTcpClient(code);
          if (code != idgs::RC_SUCCESS) {
            LOG(ERROR) << "Error in get client, cause by " << idgs::getErrorDescription(code);
            exit(1);
          }

          if (code != idgs::RC_SUCCESS) {
            LOG(ERROR) << "Error in initialize client, cause by " << idgs::getErrorDescription(code);
            exit(1);
          }
          ++count;
          idgs::client::ClientActorMessagePtr response = client->sendRecv(clientActorMsg, &code);
          LOG(INFO) << "Get message from peer " << (response)->toString();
          std::string destActorId = response->getSourceActorId();
          idgs::pb::Long lValue;
          response->parsePayload(&lValue);
          EXPECT_EQ("test_server_id", destActorId);
          client->close();
        } catch (std::exception& e) {
          std::cerr << "### ERROR:" << e.what() << std::endl;
          LOG(ERROR) << "### ERROR:" << e.what() << std::endl;
          return RC_ERROR;
        }
        return RC_SUCCESS;
      }
    }
  }
}

using namespace idgs::pb;
using namespace idgs::net::tcp_test;
using namespace idgs;

TEST(tcp, tcp_client) {
  TEST_TIMEOUT(30);

  idgs::client::ClientSetting setting;
  setting.clientConfig = "integration_test/rpc_it/client2.conf";

  idgs::ResultCode code = ::idgs::util::singleton<idgs::client::TcpClientPool>::getInstance().loadConfig(setting);
  if (code != idgs::RC_SUCCESS) {
    LOG(ERROR) << "Error in load client setting, cause by " << idgs::getErrorDescription(code);
    exit(1);
  }

  ASSERT_EQ(RC_SUCCESS, client_run());
  DLOG(INFO)<< "Client has sent the message";
  ASSERT_EQ(RC_SUCCESS, client_run());
  DLOG(INFO)<< "Client has sent the message";
  ::sleep(2);

  ::idgs::util::singleton<idgs::client::TcpClientPool>::getInstance().close();

  DLOG(INFO) << "End client test";
}

