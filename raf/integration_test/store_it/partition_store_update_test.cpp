
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
#include "tpch.pb.h"
#include "idgs/client/client_pool.h"
#include "idgs/cancelable_timer.h"

using namespace idgs;
using namespace idgs::pb;
using namespace idgs::sample::tpch::pb;
using namespace idgs::pb;
using namespace idgs::client;
using namespace idgs::store;

TEST(partition_store, update) {
  TEST_TIMEOUT(10);

  std::shared_ptr<CustomerKey> key = std::make_shared<CustomerKey>();
  key->set_c_custkey(10000);

  std::shared_ptr<Customer> customer = std::make_shared<Customer>();
  customer->set_c_name("Jerry");
  customer->set_c_nationkey(11);
  customer->set_c_address("address2");
  customer->set_c_phone("1101000010");
  customer->set_c_acctbal(200.321);
  customer->set_c_comment("customer store test update");

  std::shared_ptr<idgs::store::pb::UpdateRequest> request = std::make_shared<idgs::store::pb::UpdateRequest>();
  request->set_schema_name("tpch");
  request->set_store_name("Customer");

  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName(OP_UPDATE);
  clientActorMsg->setChannel(TC_TCP);
  clientActorMsg->setDestActorId(ACTORID_STORE_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setPayload(request);

  clientActorMsg->setAttachment(STORE_ATTACH_KEY, key);
  clientActorMsg->setAttachment(STORE_ATTACH_VALUE, customer);

  ClientSetting setting;
  setting.clientConfig = "conf/client.conf";
  ResultCode code;

  auto& pool = getTcpClientPool();
  code = pool.loadConfig(setting);
  ASSERT_EQ(RC_SUCCESS, code);

  auto client = pool.getTcpClient(code);

  if (code != RC_SUCCESS) {
    LOG(ERROR) << "initial TcpSynchronousClient error: " << getErrorDescription(code);
  }
  ASSERT_EQ(RC_SUCCESS, code);

  // response
  ClientActorMessagePtr tcpResponse;
  code = client->sendRecv(clientActorMsg, tcpResponse);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in update data to partition store, cause by " << getErrorDescription(code);
  }
  ASSERT_EQ(RC_SUCCESS, code);

  if (tcpResponse.get() == NULL) {
    LOG(ERROR) << "No response back.";
  }
  ASSERT_NE((void*)NULL, tcpResponse.get());

  idgs::store::pb::UpdateResponse response;
  bool res = tcpResponse->parsePayload(&response);
  if (!res) {
    LOG(ERROR) << "Response cannot be parsed as insert response.";
  }
  ASSERT_TRUE(res);

  std::shared_ptr<Customer> old_customer = std::make_shared<Customer>();
  if(tcpResponse->getRawAttachments().find(STORE_ATTACH_VALUE) != tcpResponse->getRawAttachments().end()) {
    if (!tcpResponse->parseAttachment(STORE_ATTACH_VALUE, old_customer.get())) {
      LOG(ERROR) << "parse old value error, message: " << tcpResponse->toString();
    }
  }

  if (response.result_code() != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR) << "Error in update data to partition store, cause by error code : " << StoreResultCode_Name(response.result_code());
  }
  ASSERT_EQ(idgs::store::pb::SRC_SUCCESS, response.result_code());

  LOG(INFO) << "Success";

  client->close();
}

TEST(partition_store, get) {
  TEST_TIMEOUT(10);

  std::shared_ptr<CustomerKey> key = std::make_shared<CustomerKey>();
  key->set_c_custkey(10000);

  std::shared_ptr<idgs::store::pb::GetRequest> request = std::make_shared<idgs::store::pb::GetRequest>();
  request->set_schema_name("tpch");
  request->set_store_name("Customer");

  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();

  clientActorMsg->setOperationName(OP_GET);
  clientActorMsg->setChannel(TC_TCP);
  clientActorMsg->setDestActorId(ACTORID_STORE_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  clientActorMsg->setAttachment(STORE_ATTACH_KEY, key);

  ResultCode code = RC_SUCCESS;
  auto client = getTcpClientPool().getTcpClient(code);
  ASSERT_EQ(RC_SUCCESS, code);

  // response
  ClientActorMessagePtr tcpResponse;
  code = client->sendRecv(clientActorMsg, tcpResponse);

  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in insert data to partition store, cause by " << getErrorDescription(code);
  }
  ASSERT_EQ(RC_SUCCESS, code);

  if (tcpResponse.get() == NULL) {
    LOG(ERROR) << "No response back.";
  }
  ASSERT_NE((void*)NULL, tcpResponse.get());

  idgs::store::pb::GetResponse response;
  bool res = tcpResponse->parsePayload(&response);
  if (!res) {
    LOG(ERROR) << "Response cannot be parsed as insert response.";
  }
  ASSERT_TRUE(res);

  if (response.result_code() != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR) << "Error in get data to partition store, cause by error code : " << response.result_code();
  }
  ASSERT_EQ(idgs::store::pb::SRC_SUCCESS, response.result_code());

  Customer customer;
  tcpResponse->parseAttachment(STORE_ATTACH_VALUE, &customer);

  LOG(INFO) << customer.DebugString();

  ASSERT_EQ("Jerry", customer.c_name());
  ASSERT_EQ("address2", customer.c_address());
  ASSERT_EQ(11, customer.c_nationkey());
  ASSERT_EQ("1101000010", customer.c_phone());
  ASSERT_EQ(200.321, customer.c_acctbal());
  ASSERT_EQ("customer store test update", customer.c_comment());

  client->close();

  getTcpClientPool().close();
}
