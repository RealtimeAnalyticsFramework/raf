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

using namespace idgs;
using namespace idgs::pb;
using namespace idgs::client;
using namespace idgs::store;
using namespace idgs::store::pb;

TEST(store_schema, show_tpch_stores) {
  // initialize clinet pool
  ClientSetting setting;
  setting.clientConfig = "conf/client.conf";

  auto& pool = getTcpClientPool();
  auto code = pool.loadConfig(setting);
  ASSERT_EQ(RC_SUCCESS, code);

  LOG(INFO) << "#################################### Step 1 ####################################";
  LOG(INFO) << "#  1. test show stores of schema tpch.                                         #";
  LOG(INFO) << "#  2. test show stores of schema tpch1, expect error schema not found.         #";
  LOG(INFO) << "################################################################################";
  std::shared_ptr<ShowStoresRequest> request = std::make_shared<ShowStoresRequest>();
  request->set_schema_name("tpch");

  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName(OP_SHOWSTORES);
  clientActorMsg->setDestActorId(ACTORID_SCHEMA_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  auto client = pool.getTcpClient(code);
  ASSERT_EQ(RC_SUCCESS, code);

  ClientActorMessagePtr tcpResponse;
  code = client->sendRecv(clientActorMsg, tcpResponse);
  ASSERT_EQ(RC_SUCCESS, code);

  ShowStoresResponse response;
  ASSERT_TRUE(tcpResponse->parsePayload(&response));
  ASSERT_EQ(SRC_SUCCESS, response.result_code());

  auto it = response.store_name().begin();
  for (; it != response.store_name().end(); ++ it) {
    LOG(INFO) << "     " << * it;
  }

  request = std::make_shared<ShowStoresRequest>();
  request->set_schema_name("tpch1");

  clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName(OP_SHOWSTORES);
  clientActorMsg->setDestActorId(ACTORID_SCHEMA_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  code = client->sendRecv(clientActorMsg, tcpResponse);
  ASSERT_EQ(RC_SUCCESS, code);

  ASSERT_TRUE(tcpResponse->parsePayload(&response));
  ASSERT_EQ(SRC_SCHEMA_NOT_FOUND, response.result_code());

  client->close();
}

TEST(store_schema, create_schema) {
  LOG(INFO) << " #################################### Step 2 ####################################";
  LOG(INFO) << " #  1. create schema intel and test, intel has store PbTest, test has no store. #";
  std::shared_ptr<CreateSchemaRequest> request = std::make_shared<CreateSchemaRequest>();

  auto schema = request->mutable_config()->add_schemas();
  schema->set_schema_name("intel");

  auto config = schema->add_store_config();
  config->set_name("PbTest");
  config->set_store_type(ORDERED);
  config->set_partition_type(PARTITION_TABLE);
  config->set_key_type("idgs.intel.pb.TestKey");
  config->set_value_type("idgs.intel.pb.TestValue");

  schema->set_proto_content("package idgs.intel.pb;\nmessage TestKey {\n  required int32 key = 1;\n}\nmessage TestValue {\n  optional string value = 1;\n}");

  request->set_config_content("{\"schemas\": [{\"schema_name\":\"test\"}]}");

  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName(OP_CREATE_SCHEMA);
  clientActorMsg->setDestActorId(ACTORID_SCHEMA_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  ResultCode code = RC_SUCCESS;
  auto client = getTcpClientPool().getTcpClient(code);
  ASSERT_EQ(RC_SUCCESS, code);

  ClientActorMessagePtr tcpResponse;
  code = client->sendRecv(clientActorMsg, tcpResponse);
  ASSERT_EQ(RC_SUCCESS, code);

  CreateSchemaResponse response;
  ASSERT_TRUE(tcpResponse->parsePayload(&response));
  ASSERT_EQ(SRC_SUCCESS, response.result_code());

  client->close();
  LOG(INFO) << "################################################################################";
}

TEST(store_schema, drop_schema) {
  LOG(INFO) << "#################################### Step 3 ####################################";
  LOG(INFO) << "#  1. drop schema intel.                                                       #";
  std::shared_ptr<DropSchemaRequest> request = std::make_shared<DropSchemaRequest>();
  request->set_schema_name("intel");

  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName(OP_DROP_SCHEMA);
  clientActorMsg->setDestActorId(ACTORID_SCHEMA_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  ResultCode code = RC_SUCCESS;
  auto client = getTcpClientPool().getTcpClient(code);
  ASSERT_EQ(RC_SUCCESS, code);

  ClientActorMessagePtr tcpResponse;
  code = client->sendRecv(clientActorMsg, tcpResponse);
  ASSERT_EQ(RC_SUCCESS, code);

  LOG(INFO) << "#  2. drop schema intel again, expect error schema not found.                  #";
  DropSchemaResponse response;
  ASSERT_TRUE(tcpResponse->parsePayload(&response));
  ASSERT_EQ(SRC_SUCCESS, response.result_code());

  request = std::make_shared<DropSchemaRequest>();
  request->set_schema_name("intel");

  clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName(OP_DROP_SCHEMA);
  clientActorMsg->setDestActorId(ACTORID_SCHEMA_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  code = client->sendRecv(clientActorMsg, tcpResponse);
  ASSERT_EQ(RC_SUCCESS, code);

  ASSERT_TRUE(tcpResponse->parsePayload(&response));
  ASSERT_EQ(SRC_SCHEMA_NOT_FOUND, response.result_code());

  client->close();
  LOG(INFO) << "################################################################################";
}

TEST(store_schema, create_store) {
  LOG(INFO) << "#################################### Step 4 ####################################";
  LOG(INFO) << "#  1. create store PbTest and Test of schema intel.                            #";
  std::shared_ptr<CreateStoreRequest> request = std::make_shared<CreateStoreRequest>();

  auto schema = request->mutable_schema();
  schema->set_schema_name("intel");

  auto config = schema->add_store_config();
  config->set_name("PbTest");
  config->set_store_type(ORDERED);
  config->set_partition_type(PARTITION_TABLE);
  config->set_key_type("idgs.intel.pb.TestKey");
  config->set_value_type("idgs.intel.pb.TestValue");

  schema->set_proto_content("package idgs.intel.pb;\nmessage TestKey {\n  required int32 key = 1;\n}\nmessage TestValue {\n  optional string value = 1;\n}");

  request->set_schema_content("{\"schema_name\":\"intel\", \"store_config\": "
      "[{\"name\":\"Test\", \"store_type\":\"ORDERED\", \"partition_type\":\"PARTITION_TABLE\", "
      "\"key_type\":\"idgs.intel.pb.TestKey\", \"value_type\":\"idgs.intel.pb.TestValue\"}]}");

  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName(OP_CREATE_STORE);
  clientActorMsg->setDestActorId(ACTORID_SCHEMA_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  ResultCode code = RC_SUCCESS;
  auto client = getTcpClientPool().getTcpClient(code);
  ASSERT_EQ(RC_SUCCESS, code);

  ClientActorMessagePtr tcpResponse;
  code = client->sendRecv(clientActorMsg, tcpResponse);
  ASSERT_EQ(RC_SUCCESS, code);

  CreateStoreResponse response;
  ASSERT_TRUE(tcpResponse->parsePayload(&response));
  ASSERT_EQ(SRC_SUCCESS, response.result_code());

  LOG(INFO) << "#  2. create store PbTest and Test again, expect error store exists.           #";
  code = client->sendRecv(clientActorMsg, tcpResponse);
  ASSERT_EQ(RC_SUCCESS, code);

  ASSERT_TRUE(tcpResponse->parsePayload(&response));
  ASSERT_EQ(SRC_STORE_EXISTED, response.result_code());

  client->close();
  LOG(INFO) << "################################################################################";
}

TEST(store_schema, drop_store) {
  LOG(INFO) << "#################################### Step 5 ####################################";
  LOG(INFO) << "#  1. drop store PbTest of schema intel.                                       #";
  std::shared_ptr<DropStoreRequest> request = std::make_shared<DropStoreRequest>();
  request->set_schema_name("intel");
  request->set_store_name("PbTest");

  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName(OP_DROP_STORE);
  clientActorMsg->setDestActorId(ACTORID_SCHEMA_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  ResultCode code = RC_SUCCESS;
  auto client = getTcpClientPool().getTcpClient(code);
  ASSERT_EQ(RC_SUCCESS, code);

  ClientActorMessagePtr tcpResponse;
  code = client->sendRecv(clientActorMsg, tcpResponse);
  ASSERT_EQ(RC_SUCCESS, code);

  DropStoreResponse response;
  ASSERT_TRUE(tcpResponse->parsePayload(&response));
  ASSERT_EQ(SRC_SUCCESS, response.result_code());

  LOG(INFO) << "#  2. drop store PbTest again, expect error store not found.                   #";
  code = client->sendRecv(clientActorMsg, tcpResponse);
  ASSERT_EQ(RC_SUCCESS, code);

  ASSERT_TRUE(tcpResponse->parsePayload(&response));
  ASSERT_EQ(SRC_STORE_NOT_FOUND, response.result_code());

  client->close();
  LOG(INFO) << "################################################################################";
}

TEST(store_schema, data) {
  LOG(INFO) << "#################################### Step 6 ####################################";
  LOG(INFO) << "#  1. insert data to store Test(key = 100000, value = abc).                    #";
  protobuf::MessageHelper helper;
  helper.registerDynamicMessageFromString("package idgs.intel.pb;\nmessage TestKey {\n  required int32 key = 1;\n}\nmessage TestValue {\n  optional string value = 1;\n}");

  auto key = helper.createMessage("idgs.intel.pb.TestKey");
  auto value = helper.createMessage("idgs.intel.pb.TestValue");

  key->GetReflection()->SetInt32(key.get(), key->GetDescriptor()->FindFieldByName("key"), 100000);
  value->GetReflection()->SetString(value.get(), value->GetDescriptor()->FindFieldByName("value"), "abc");

  std::shared_ptr<InsertRequest> request = std::make_shared<InsertRequest>();
  request->set_schema_name("intel");
  request->set_store_name("Test");

  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName(OP_INSERT);
  clientActorMsg->setDestActorId(ACTORID_STORE_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  clientActorMsg->setAttachment(STORE_ATTACH_KEY, key);
  clientActorMsg->setAttachment(STORE_ATTACH_VALUE, value);

  ResultCode code = RC_SUCCESS;
  auto client = getTcpClientPool().getTcpClient(code);
  ASSERT_EQ(RC_SUCCESS, code);

  ClientActorMessagePtr tcpResponse;
  code = client->sendRecv(clientActorMsg, tcpResponse);
  ASSERT_EQ(RC_SUCCESS, code);

  InsertResponse response;
  ASSERT_TRUE(tcpResponse->parsePayload(&response));
  ASSERT_EQ(SRC_SUCCESS, response.result_code());

  LOG(INFO) << "#  2. get data from store Test(key = 100000), expect value = abc.              #";
  std::shared_ptr<GetRequest> getRequest = std::make_shared<GetRequest>();
  getRequest->set_schema_name("intel");
  getRequest->set_store_name("Test");

  clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName(OP_GET);
  clientActorMsg->setDestActorId(ACTORID_STORE_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(getRequest);

  clientActorMsg->setAttachment(STORE_ATTACH_KEY, key);

  code = client->sendRecv(clientActorMsg, tcpResponse);
  ASSERT_EQ(RC_SUCCESS, code);

  GetResponse getResponse;
  ASSERT_TRUE(tcpResponse->parsePayload(&getResponse));
  ASSERT_EQ(SRC_SUCCESS, getResponse.result_code());

  value = helper.createMessage("idgs.intel.pb.TestValue");
  ASSERT_TRUE(tcpResponse->parseAttachment(STORE_ATTACH_VALUE, value.get()));

  ASSERT_EQ("abc", value->GetReflection()->GetString(* value, value->GetDescriptor()->FindFieldByName("value")));

  client->close();

  LOG(INFO) << "################################################################################";
}
