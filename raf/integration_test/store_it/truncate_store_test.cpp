
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "gtest/gtest.h"

#include "idgs/store/datastore_const.h"
#include "idgs/client/client_pool.h"
#include "idgs/cancelable_timer.h"

using namespace idgs;
using namespace idgs::pb;
using namespace idgs::pb;
using namespace idgs::client;
using namespace idgs::store;

void initClient() {
  ClientSetting setting;
  setting.clientConfig = "conf/client.conf";
  ResultCode code = getTcpClientPool().loadConfig(setting);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "initial TcpSynchronousClient error: " << getErrorDescription(code);
    exit(1);
  }
}

TEST(partition_store, truncate_customer) {
  TEST_TIMEOUT(30);

  LOG(INFO) << "start initialize client.";
  initClient();

  std::shared_ptr<idgs::store::pb::TruncateRequest> request = std::make_shared<idgs::store::pb::TruncateRequest>();
  request->set_schema_name("tpch");
  request->set_store_name("Customer");

  idgs::client::ClientActorMessagePtr clientActorMsg = std::make_shared<idgs::client::ClientActorMessage>();
  clientActorMsg->setOperationName(OP_TRUNCATE);
  clientActorMsg->setDestActorId(ACTORID_STORE_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  ResultCode code = RC_SUCCESS;
  auto client = getTcpClientPool().getTcpClient(code);
  idgs::client::ClientActorMessagePtr tcpResponse;
  code = client->sendRecv(clientActorMsg, tcpResponse);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in get data to partition store, cause by " << getErrorDescription(code);
    exit(1);
  }

  idgs::store::pb::TruncateResponse response;
  tcpResponse->parsePayload(&response);
  EXPECT_EQ(idgs::store::pb::SRC_SUCCESS, response.result_code());

  client->close();
}

TEST(partition_store, truncate_nation) {
  TEST_TIMEOUT(30);

  std::shared_ptr<idgs::store::pb::TruncateRequest> request = std::make_shared<idgs::store::pb::TruncateRequest>();
  request->set_schema_name("tpch");
  request->set_store_name("Nation");

  idgs::client::ClientActorMessagePtr clientActorMsg = std::make_shared<idgs::client::ClientActorMessage>();
  clientActorMsg->setOperationName(OP_TRUNCATE);
  clientActorMsg->setDestActorId(ACTORID_STORE_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  ResultCode code = RC_SUCCESS;
  auto client = getTcpClientPool().getTcpClient(code);
  idgs::client::ClientActorMessagePtr tcpResponse;
  code = client->sendRecv(clientActorMsg, tcpResponse);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in get data to partition store, cause by " << getErrorDescription(code);
    exit(1);
  }

  idgs::store::pb::TruncateResponse response;
  tcpResponse->parsePayload(&response);
  EXPECT_EQ(idgs::store::pb::SRC_SUCCESS, response.result_code());

  client->close();

  getTcpClientPool().close();
}

TEST(partition_store, truncate_lineitem) {
  TEST_TIMEOUT(30);

  LOG(INFO) << "start initialize client.";
  initClient();

  std::shared_ptr<idgs::store::pb::TruncateRequest> request = std::make_shared<idgs::store::pb::TruncateRequest>();
  request->set_schema_name("tpch");
  request->set_store_name("LineItem");

  idgs::client::ClientActorMessagePtr clientActorMsg = std::make_shared<idgs::client::ClientActorMessage>();
  clientActorMsg->setOperationName(OP_TRUNCATE);
  clientActorMsg->setDestActorId(ACTORID_STORE_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  ResultCode code = RC_SUCCESS;
  auto client = getTcpClientPool().getTcpClient(code);
  idgs::client::ClientActorMessagePtr tcpResponse;
  code = client->sendRecv(clientActorMsg, tcpResponse);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in get data to partition store, cause by " << getErrorDescription(code);
    exit(1);
  }

  idgs::store::pb::TruncateResponse response;
  tcpResponse->parsePayload(&response);
  EXPECT_EQ(idgs::store::pb::SRC_SUCCESS, response.result_code());

  client->close();
}
