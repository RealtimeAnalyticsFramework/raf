
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

TEST(partition_store, insert) {
  TEST_TIMEOUT(10);

  std::shared_ptr<CustomerKey> key(new CustomerKey);
  key->set_c_custkey(10000);

  std::shared_ptr<Customer> customer(new Customer);
  customer->set_c_name("Tom");
  customer->set_c_nationkey(10);
  customer->set_c_address("address");
  customer->set_c_phone("13800138000");
  customer->set_c_acctbal(100.123);
  customer->set_c_comment("customer store test");

  std::shared_ptr<idgs::store::pb::InsertRequest> request(new idgs::store::pb::InsertRequest);
  request->set_store_name("Customer");

  ClientActorMessagePtr clientActorMsg(new ClientActorMessage);
  clientActorMsg->setOperationName(OP_INSERT);
  clientActorMsg->setChannel(TC_TCP);
  clientActorMsg->setDestActorId(ACTORID_STORE_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setPayload(request);

  clientActorMsg->setAttachment(STORE_ATTACH_KEY, key);
  clientActorMsg->setAttachment(STORE_ATTACH_VALUE, customer);

  std::string result;
  protobuf::ProtoSerdes<DEFAULT_PB_SERDES>::serialize(key.get(), &result);
  LOG(INFO) << "serialized key: " << result;
  protobuf::ProtoSerdes<DEFAULT_PB_SERDES>::serialize(customer.get(), &result);
  LOG(INFO) << "serialized value: " << result;

  ClientSetting setting;
  setting.clientConfig = "integration_test/store_it/client.conf";
  ResultCode code;

  code = ::idgs::util::singleton<TcpClientPool>::getInstance().loadConfig(setting);
  ASSERT_EQ(RC_SUCCESS, code);

  std::shared_ptr<TcpClientInterface> client = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(code);

  if (code != RC_SUCCESS) {
    LOG(ERROR) << "initial TcpSynchronousClient error: " << getErrorDescription(code);
  }
  ASSERT_EQ(RC_SUCCESS, code);

  // response
  ClientActorMessagePtr tcpResponse = client->sendRecv(clientActorMsg, &code);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in insert data to partition store, cause by " << getErrorDescription(code);
  }
  ASSERT_EQ(RC_SUCCESS, code);

  if (tcpResponse.get() == NULL) {
    LOG(ERROR) << "No response back.";
  }
  ASSERT_TRUE(tcpResponse.get() != NULL);

  idgs::store::pb::InsertResponse response;
  bool res = tcpResponse->parsePayload(&response);
  if (!res) {
    LOG(ERROR) << "Response cannot be parsed as insert response.";
  }
  ASSERT_TRUE(res);

  if (response.result_code() != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR) << "Error in insert data to partition store, cause by error code : " << StoreResultCode_Name(response.result_code());
  }
  ASSERT_EQ(idgs::store::pb::SRC_SUCCESS, response.result_code());

  LOG(INFO) << "==================inser value===================";
  LOG(INFO) << customer->DebugString();
  LOG(INFO) << "================//insert value===================";

  client->close();
  ::idgs::util::singleton<TcpClientPool>::getInstance().close();
}

TEST(partition_store, update) {
  TEST_TIMEOUT(10);

  std::shared_ptr<CustomerKey> key(new CustomerKey);
  key->set_c_custkey(10000);

  std::shared_ptr<Customer> customer(new Customer);
  customer->set_c_name("Tom");
  customer->set_c_nationkey(10);
  customer->set_c_address("address");
  customer->set_c_phone("13800138000");
  customer->set_c_acctbal(100.123);
  customer->set_c_comment("customer store test update");

  std::shared_ptr<idgs::store::pb::UpdateRequest> request(new idgs::store::pb::UpdateRequest);
  request->set_store_name("Customer");
  request->set_options(1);


  ClientActorMessagePtr clientActorMsg(new ClientActorMessage);
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
  setting.clientConfig = "integration_test/store_it/client.conf";
  ResultCode code;

  code = ::idgs::util::singleton<TcpClientPool>::getInstance().loadConfig(setting);
  ASSERT_EQ(RC_SUCCESS, code);

  std::shared_ptr<TcpClientInterface> client = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(code);

  if (code != RC_SUCCESS) {
    LOG(ERROR) << "initial TcpSynchronousClient error: " << getErrorDescription(code);
  }
  ASSERT_EQ(RC_SUCCESS, code);

  // response
  ClientActorMessagePtr tcpResponse = client->sendRecv(clientActorMsg, &code);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in update data to partition store, cause by " << getErrorDescription(code);
  }
  ASSERT_EQ(RC_SUCCESS, code);

  if (tcpResponse.get() == NULL) {
    LOG(ERROR) << "No response back.";
  }
  ASSERT_TRUE(tcpResponse.get() != NULL);

  idgs::store::pb::UpdateResponse response;
  bool res = tcpResponse->parsePayload(&response);
  if (!res) {
    LOG(ERROR) << "Response cannot be parsed as insert response.";
  }
  ASSERT_TRUE(res);

  std::shared_ptr<Customer> old_customer(new Customer);
  if(tcpResponse->getRawAttachments().find(STORE_ATTACH_VALUE) != tcpResponse->getRawAttachments().end()) {
    if (!tcpResponse->parseAttachment(STORE_ATTACH_VALUE, old_customer.get())) {
      LOG(ERROR) << "parse old value error, message: " << tcpResponse->toString();
    }
  }
  LOG(INFO) << "==================old value===================";
  LOG(INFO) << old_customer->DebugString();
  LOG(INFO) << "================//old value===================";

  if (response.result_code() != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR) << "Error in update data to partition store, cause by error code : " << StoreResultCode_Name(response.result_code());
  }
  ASSERT_EQ(idgs::store::pb::SRC_SUCCESS, response.result_code());

  LOG(INFO) << "Success";

  client->close();
  ::idgs::util::singleton<TcpClientPool>::getInstance().close();
}
