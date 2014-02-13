
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include <array>
#include <gtest/gtest.h>
#include "idgs/client/client_pool.h"

using namespace idgs::pb;
using namespace idgs::client;
using namespace std;

namespace idgs {
  namespace net {
    namespace scheduler_test {
      int count(0);
      int test_server_count(0);
      const std::string test_server_id = "test_server_id";
      const std::string start_work_operation = "start_work_operation";
      const std::string new_client_message_comes = "new_client_message_comes";
      const std::string sending_reponse_succ = "sending_reponse_successfully";
      const std::string sending_total = "sending_total";

      ///////////////////////////// Call Flow //////////////////////
      //   Client                                          server
      //      |                                               |
      //      | ------- start_work_operation----------------->| 1. test_server_count++
      //      |                                               |
      //      |<------- start_work_operation_succesfully -----| 2. start Timer and send response (timer is sending_response_timeout)
      //      |                                               |
      //      |<------- sending_reponse_successfully ---------| 3. time out and send a new response, test_server_count++
      //      |                                               | 4. start timer to wait the client's resonse (timer is waiting_new_message_timeout)
      //      |                                               |
      //      |--------- new_client_message_comes ----------->| 5. Cancel timer, test_server_count++
      //
      //      Assert test_server_count == 3
      //

      int client_run() {
        function_footprint();

        using asio::ip::tcp;
        try {

          ResultCode resultCode;

          std::shared_ptr<TcpClientInterface> client =
              ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(resultCode);
          if (resultCode != RC_SUCCESS) {
            cerr << "Get Client error: " << getErrorDescription(resultCode) << endl;
            return RC_ERROR;
          }

          // step 1, 2
          std::array<char, 128> buf;

          idgs::client::ClientActorMessagePtr clientActorMsg(new idgs::client::ClientActorMessage);
          clientActorMsg->setChannel(TC_AUTO);
          clientActorMsg->setDestActorId(test_server_id);
          clientActorMsg->setDestMemberId(ANY_MEMBER);
          clientActorMsg->setSourceActorId("client_actor_id");
          clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
          clientActorMsg->setOperationName(start_work_operation);
          clientActorMsg->getRpcMessage()->set_payload("payload");
          ResultCode errorCode;
          idgs::client::ClientActorMessagePtr response = client->sendRecv(clientActorMsg, &errorCode);
          if (errorCode != RC_SUCCESS) {
            cerr << "execute the command error: " << getErrorDescription(errorCode) << endl;
            client->close();
            return RC_ERROR;
          }
          DLOG(INFO) << "write message to server" << clientActorMsg->toString();
          // step 3, 4
          response = client->receive(errorCode);
          if (errorCode != RC_SUCCESS) {
            DLOG(INFO) << "get response error " << errorCode;
            return errorCode;
          }
          if (response->getOperationName() == sending_reponse_succ) {
            response->setOperationName(new_client_message_comes);
            response->setDestActorId(test_server_id);
            response->setSourceActorId("client_actor_id");
            response->setDestMemberId(response->getSourceMemberId());
            response->setChannel(TC_TCP);
            response->setSourceMemberId(CLIENT_MEMBER);
            ResultCode resultCode;

            ResultCode errorCode;
            idgs::client::ClientActorMessagePtr tcpResponse = client->sendRecv(response, &errorCode);
            DVLOG(2) << "after sendRecv";
            if (errorCode != RC_SUCCESS) {
              cerr << "execute the command error: " << getErrorDescription(errorCode) << endl;
              client->close();
              return RC_ERROR;
            }
            if (tcpResponse->getOperationName() == sending_total) {
              idgs::pb::Long l;
              tcpResponse->parsePayload(&l);
              test_server_count = l.value();
            }
            ++count;
          } else {
            std::cerr <<"Test client error, receive error server response" ;
            return RC_ERROR;
          }

          //while (1) {}
          //socket.close();

        } catch (std::exception& e) {
          std::cerr <<"Test client error" << e.what() << std::endl;
          return RC_ERROR;
        }
        return RC_SUCCESS;
      }
    }
  }
}

using namespace idgs::pb;
using namespace idgs::net::scheduler_test;
using namespace std;
using namespace idgs;

TEST(scheduler_test, scheduler_actor_test) {
  function_footprint();

  ClientSetting setting;
  setting.clientConfig = "integration_test/rpc_it/client.conf";
  ResultCode resultCode;

  DVLOG(2) <<"start parse conf" ;
  resultCode = ::idgs::util::singleton<TcpClientPool>::getInstance().loadConfig(setting);
  if (resultCode != RC_SUCCESS) {
    cerr << "Load Configuration error: " << getErrorDescription(resultCode) << endl;
    exit(1);
  }

//  std::thread *m_pthread = new std::thread(&client_run);
//  m_pthread->join();
  client_run();

  //ASSERT_EQ(2, count);
  ASSERT_EQ(3, test_server_count);
  ::sleep(3);
  //::idgs::util::singleton<idgs::client::TcpClientPool>::getInstance().close();
  DLOG(INFO) << "Test finished";
}
