
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

TEST(replicated_store, insert) {
  TEST_TIMEOUT(10);

  std::shared_ptr<NationKey> key(new NationKey);
  key->set_n_nationkey(10000);

  std::shared_ptr<Nation> nation(new Nation);
  nation->set_n_name("China");
  nation->set_n_regionkey(35000);
  nation->set_n_comment("Intel");

  std::shared_ptr<idgs::store::pb::InsertRequest> request(new idgs::store::pb::InsertRequest);
  request->set_store_name("Nation");

  ClientActorMessagePtr clientActorMsg(new ClientActorMessage);
  clientActorMsg->setOperationName(OP_INSERT);
  clientActorMsg->setChannel(TC_TCP);
  clientActorMsg->setDestActorId(ACTORID_STORE_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  clientActorMsg->setAttachment(STORE_ATTACH_KEY, key);
  clientActorMsg->setAttachment(STORE_ATTACH_VALUE, nation);

  ClientSetting setting;
  setting.clientConfig = "integration_test/store_it/client.conf";
  ResultCode code;

  code = ::idgs::util::singleton<TcpClientPool>::getInstance().loadConfig(setting);
  ASSERT_EQ(RC_SUCCESS, code);

  std::shared_ptr<TcpClientInterface> client = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(code);
  ASSERT_EQ(RC_SUCCESS, code);

  // response
  ClientActorMessagePtr tcpResponse = client->sendRecv(clientActorMsg, &code);
  ASSERT_EQ(RC_SUCCESS, code);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in get data to partition store, cause by " << getErrorDescription(code);
    return;
  }

  ASSERT_TRUE(tcpResponse.get() != NULL);
  if (tcpResponse.get() == NULL) {
    LOG(ERROR) << "No response back.";
    return;
  }

  idgs::store::pb::InsertResponse response;
  bool res = tcpResponse->parsePayload(&response);
  ASSERT_TRUE(res);
  if (!res) {
    LOG(ERROR) << "Response cannot be parsed as insert response.";
    return;
  }

  ASSERT_EQ(idgs::store::pb::SRC_SUCCESS, response.result_code());
  if (response.result_code() != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR) << "Error in insert data to partition store, cause by error code : " << response.result_code();
    return;
  }

  LOG(INFO) << "Success";

  client->close();
  ::idgs::util::singleton<TcpClientPool>::getInstance().close();
}
