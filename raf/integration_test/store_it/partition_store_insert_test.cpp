
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

  std::shared_ptr<CustomerKey> key = std::make_shared<CustomerKey>();
  key->set_c_custkey(10000);

  std::shared_ptr<Customer> customer = std::make_shared<Customer>();
  customer->set_c_name("Tom");
  customer->set_c_nationkey(10);
  customer->set_c_address("address");
  customer->set_c_phone("13800138000");
  customer->set_c_acctbal(100.123);
  customer->set_c_comment("customer store test");

  std::shared_ptr<idgs::store::pb::InsertRequest> request = std::make_shared<idgs::store::pb::InsertRequest>();
  request->set_schema_name("tpch");
  request->set_store_name("Customer");

  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
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
    LOG(ERROR) << "Error in insert data to partition store, cause by " << getErrorDescription(code);
  }
  ASSERT_EQ(RC_SUCCESS, code);

  if (tcpResponse.get() == NULL) {
    LOG(ERROR) << "No response back.";
  }
  ASSERT_NE((void*)NULL, (tcpResponse.get()));

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
  getTcpClientPool().close();
}
