
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

using namespace std;
using namespace idgs;
using namespace idgs::pb;
using namespace idgs::sample::tpch::pb;
using namespace idgs::pb;
using namespace idgs::client;
using namespace idgs::store;

idgs::ResultCode initializeClient(const std::string& clientConfig) {
  idgs::client::ClientSetting setting;
  setting.clientConfig = clientConfig;

  return ::idgs::util::singleton<idgs::client::TcpClientPool>::getInstance().loadConfig(setting);
}

ResultCode getData(const string& storeName, const std::shared_ptr< ::google::protobuf::Message>& key, idgs::store::pb::GetResponse& response) {
  std::shared_ptr<idgs::store::pb::GetRequest> request(new idgs::store::pb::GetRequest);
  request->set_store_name(storeName);

  idgs::client::ClientActorMessagePtr clientActorMsg(new idgs::client::ClientActorMessage);
  clientActorMsg->setOperationName(OP_GET);
  clientActorMsg->setDestActorId(ACTORID_STORE_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setChannel(TC_AUTO);
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);
  clientActorMsg->setAttachment(STORE_ATTACH_KEY, key);

  ResultCode code;
  shared_ptr<TcpClientInterface> client = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(code);
  // response
  idgs::client::ClientActorMessagePtr tcpResponse = client->sendRecv(clientActorMsg, &code);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in get data to replicated store, cause by " << getErrorDescription(code);
    return code;
  }

  if (!tcpResponse->parsePayload(&response)) {
    code = RC_INVALID_MESSAGE;
    LOG(ERROR) << "Error in get data to replicated store, cannot parse response";
    return code;
  }

  client->close();

  return RC_SUCCESS;
}

TEST(partition_store, get) {
  TEST_TIMEOUT(10);

  ResultCode code = initializeClient("integration_test/store_it/client.conf");
  EXPECT_EQ(RC_SUCCESS, code);

  std::shared_ptr<CustomerKey> key(new CustomerKey);
  key->set_c_custkey(10000);

  idgs::store::pb::GetResponse response;
  code = getData("Customer", key, response);
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(idgs::store::pb::SRC_DATA_NOT_FOUND, response.result_code());

  ::idgs::util::singleton<TcpClientPool>::getInstance().close();
}

TEST(replicated_store, get) {
  TEST_TIMEOUT(10);

  ResultCode code = initializeClient("integration_test/store_it/client1.conf");
  EXPECT_EQ(RC_SUCCESS, code);

  std::shared_ptr<NationKey> key(new NationKey);
  key->set_n_nationkey(10000);

  idgs::store::pb::GetResponse response;
  code = getData("Nation", key, response);
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(idgs::store::pb::SRC_DATA_NOT_FOUND, response.result_code());

  ::idgs::util::singleton<TcpClientPool>::getInstance().close();
}
