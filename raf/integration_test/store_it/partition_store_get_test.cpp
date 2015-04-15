
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

TEST(partition_store, get) {
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

  ClientSetting setting;
  setting.clientConfig = "conf/client_8800.conf";
  ResultCode code;

  auto& pool = getTcpClientPool();
  code = pool.loadConfig(setting);
  ASSERT_EQ(RC_SUCCESS, code);

  auto client = pool.getTcpClient(code);
  ASSERT_EQ(RC_SUCCESS, code);

  // response
  ClientActorMessagePtr tcpResponse;
  code = client->sendRecv(clientActorMsg, tcpResponse);

  DVLOG(2) << " partition store test 1";

  DVLOG(2) << tcpResponse->toString();

  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in insert data to partition store, cause by " << getErrorDescription(code);
  }
  ASSERT_EQ(RC_SUCCESS, code);

  DVLOG(2) << " partition store test 2";

  if (tcpResponse.get() == NULL) {
    LOG(ERROR) << "No response back.";
  }
  ASSERT_NE((void*)NULL, tcpResponse.get());

  DVLOG(2) << " partition store test 3";

  idgs::store::pb::GetResponse response;
  bool res = tcpResponse->parsePayload(&response);
  if (!res) {
    LOG(ERROR) << "Response cannot be parsed as insert response.";
  }
  ASSERT_TRUE(res);

  DVLOG(2) << " partition store test 4";

  if (response.result_code() != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR) << "Error in get data to partition store, cause by error code : " << response.result_code();
  }
  ASSERT_EQ(idgs::store::pb::SRC_SUCCESS, response.result_code());

  DVLOG(2) << " partition store test 5";

  Customer customer;
  tcpResponse->parseAttachment(STORE_ATTACH_VALUE, &customer);

  DVLOG(2) << " partition store test 5.1";

  LOG(INFO) << customer.DebugString();

  DVLOG(2) << " partition store test 6";

  ASSERT_EQ("Tom", customer.c_name());
  ASSERT_EQ("address", customer.c_address());
  ASSERT_EQ(10, customer.c_nationkey());
  ASSERT_EQ("13800138000", customer.c_phone());
  ASSERT_EQ(100.123, customer.c_acctbal());

  client->close();

  DVLOG(2) << " partition store test 7";

  getTcpClientPool().close();

  DVLOG(2) << " partition store test 8";
}
