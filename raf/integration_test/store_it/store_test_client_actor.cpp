
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "gtest/gtest.h"

#include "idgs/application.h"
#include "idgs/signal_handler.h"
#include "idgs/store/datastore_const.h"
#include "idgs/store/pb/store_service.pb.h"
#include "tpch.pb.h"
#include "idgs/cancelable_timer.h"

using namespace std;
using namespace idgs;
using namespace idgs::pb;
using namespace idgs::sample::tpch::pb;
using namespace idgs::pb;
using namespace idgs::actor;
using namespace idgs::store;

std::vector<std::string> msgs;
string payload;
shared_ptr<Customer> customer = std::make_shared<Customer>();
shared_ptr<Nation> nation = std::make_shared<Nation>();
idgs::store::pb::StoreResultCode resultCode;

namespace {
struct ApplicationSetting {
  ApplicationSetting():clusterConfig("") {}
  std::string clusterConfig;
};
}

void startServer() {
  ApplicationSetting setting;
  setting.clusterConfig = "conf/cluster.conf";

  Application& app = * idgs_application();
  ResultCode rc;

  LOG(INFO) << "Loading configuration.";
  rc = app.init(setting.clusterConfig);
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to initialize server: " << getErrorDescription(rc);
    exit(1);
  }

  LOG(INFO) << "Server is starting.";
  rc = app.start();
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to start server: " << getErrorDescription(rc);
    exit(1);
  }

  SignalHandler sh;
  sh.setup();

  sleep(5);
}

void init() {
  resultCode = idgs::store::pb::SRC_UNKNOWN_ERROR;
  customer->set_c_name("Tom");
  customer->set_c_nationkey(10);
  customer->set_c_address("address");
  customer->set_c_phone("13800138000");
  customer->set_c_acctbal(100.123);
  customer->set_c_comment("customer store test");

  nation->set_n_name("China");
  nation->set_n_regionkey(35000);
  nation->set_n_comment("Intel");

  msgs.resize(static_cast<int32_t>(idgs::store::pb::StoreResultCode_MAX) + 1);
  msgs[idgs::store::pb::SRC_SUCCESS] = "Success";
  msgs[idgs::store::pb::SRC_KEY_EXIST] = "Key is already exists.";
  msgs[idgs::store::pb::SRC_KEY_NOT_EXIST] = "Key is not exists";
  msgs[idgs::store::pb::SRC_VERSION_CONFLICT] = "Version conflict.";
  msgs[idgs::store::pb::SRC_STORE_NOT_FOUND] = "Store not found.";
  msgs[idgs::store::pb::SRC_DATA_NOT_FOUND] = "Data not found.";
  msgs[idgs::store::pb::SRC_PARTITION_NOT_FOUND] = "Partition not found.";
  msgs[idgs::store::pb::SRC_PARTITION_NOT_READY] = "Partition is not ready.";
  msgs[idgs::store::pb::SRC_INVALID_KEY] = "Invalid key.";
  msgs[idgs::store::pb::SRC_INVALID_VALUE] = "Invalid value.";
  msgs[idgs::store::pb::SRC_INVALID_FILTER] = "Invalid filter.";
  msgs[idgs::store::pb::SRC_INVALID_LISTENER_INFO] = "Invalid listener info.";
  msgs[idgs::store::pb::SRC_NOT_LOCAL_STORE] = "Is not local store.";
  msgs[idgs::store::pb::SRC_UNKNOWN_ERROR] = "Unknown error.";
}

namespace idgs {
namespace store {
namespace test {

class DataInsertActor : public StatefulActor {
public:
  void init() {
  }

  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override {
    static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
        {OP_INSERT,                  static_cast<idgs::actor::ActorMessageHandler>(&DataInsertActor::handleInsertRequest)},
        {OP_INSERT_RESPONSE, static_cast<idgs::actor::ActorMessageHandler>(&DataInsertActor::handleInsertResponse)}
    };
    return handlerMap;
  }


  const std::string& getActorName() const override {
    static const std::string actorName("DataInsertActor");
    return actorName;
  }

  const idgs::actor::ActorDescriptorPtr& getDescriptor() const {
    static idgs::actor::ActorDescriptorPtr descriptor;
    if(!descriptor) {
      descriptor = std::make_shared<idgs::actor::ActorDescriptorWrapper>();
      descriptor->setName(getActorName());
      descriptor->setDescription("Test insert data store.");
      descriptor->setType(AT_STATEFUL);

      idgs::actor::ActorOperationDescriporWrapper dataInsertRequest;
      dataInsertRequest.setName(OP_INSERT);
      dataInsertRequest.setDescription("Insert request.");
      dataInsertRequest.setPayloadType("idgs.store.pb.InsertRequest");
      descriptor->setInOperation(dataInsertRequest.getName(), dataInsertRequest);

      idgs::actor::ActorOperationDescriporWrapper dataInsertResponse;
      dataInsertResponse.setName(OP_INSERT_RESPONSE);
      dataInsertResponse.setDescription("Insert response.");
      dataInsertResponse.setPayloadType("idgs.store.pb.InsertResponse");
      descriptor->setInOperation(dataInsertResponse.getName(), dataInsertResponse);
    }
    return descriptor;
  }

  void handleInsertRequest(const ActorMessagePtr& msg) {
    msg->setOperationName(OP_INSERT);
    msg->setSourceActorId(getActorId());
    msg->setSourceMemberId(1);
    msg->setDestActorId(ACTORID_STORE_SERVCIE);
    msg->setDestMemberId(0);

    VLOG(0) << "handle OP_INSERT";
    idgs::actor::sendMessage(const_cast<ActorMessagePtr&>(msg));
  }

  void handleInsertResponse(const ActorMessagePtr& msg) {
    VLOG(0) << "handle insert response";
    idgs::store::pb::InsertResponse* response = dynamic_cast<idgs::store::pb::InsertResponse*>(msg->getPayload().get());;

    LOG(INFO) << "Result is " << msgs[response->result_code()];
    resultCode = response->result_code();
  }
};

class DataGetActor : public StatefulActor {
public:
  void init() {
  }
  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override {
    static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
        {OP_GET,                  static_cast<idgs::actor::ActorMessageHandler>(&DataGetActor::handleGetRequest)},
        {OP_GET_RESPONSE, static_cast<idgs::actor::ActorMessageHandler>(&DataGetActor::handleGetResponse)},
    };
    return handlerMap;
  }

  const std::string& getActorName() const override{
    static const std::string actorName("DataGetActor");
    return actorName;
  }


  const idgs::actor::ActorDescriptorPtr& getDescriptor() const {
    static idgs::actor::ActorDescriptorPtr descriptor;
    if (!descriptor) {
      descriptor = std::make_shared<idgs::actor::ActorDescriptorWrapper>();
      descriptor->setName(getActorName());
      descriptor->setDescription("Test insert data store.");
      descriptor->setType(AT_STATEFUL);

      idgs::actor::ActorOperationDescriporWrapper dataGetRequest;
      dataGetRequest.setName(OP_GET);
      dataGetRequest.setDescription("Get request.");
      dataGetRequest.setPayloadType("idgs.store.pb.GetRequest");
      descriptor->setInOperation(dataGetRequest.getName(), dataGetRequest);

      idgs::actor::ActorOperationDescriporWrapper dataGetResponse;
      dataGetResponse.setName(OP_GET_RESPONSE);
      dataGetResponse.setDescription("Get response.");
      dataGetResponse.setPayloadType("idgs.store.pb.GetResponse");
      descriptor->setInOperation(dataGetResponse.getName(), dataGetResponse);
    }

    return descriptor;
  }


  void handleGetRequest(const ActorMessagePtr& msg) {
    msg->setOperationName(OP_GET);
    msg->setSourceActorId(getActorId());
    msg->setSourceMemberId(1);
    msg->setDestActorId(ACTORID_STORE_SERVCIE);
    msg->setDestMemberId(0);

    idgs::actor::sendMessage(const_cast<ActorMessagePtr&>(msg));
  }

  void handleGetResponse(const ActorMessagePtr& msg) {
    idgs::store::pb::GetResponse* response = dynamic_cast<idgs::store::pb::GetResponse*>(msg->getPayload().get());

    LOG(INFO) << "Result is " << msgs[response->result_code()];
    resultCode = response->result_code();
    if (response->result_code() == idgs::store::pb::SRC_SUCCESS) {
      payload = msg->getRawAttachments().find(STORE_ATTACH_VALUE)->second;
    }
  }
};
}
}
}

TEST(store, partition_store_insert) {
  TEST_TIMEOUT(200);
  init();
  startServer();

  shared_ptr<idgs::store::pb::InsertRequest> request = std::make_shared<idgs::store::pb::InsertRequest>();
  request->set_schema_name("tpch");
  request->set_store_name("Customer");

  shared_ptr<CustomerKey> key = std::make_shared<CustomerKey>();
  key->set_c_custkey(10000);

  ActorMessagePtr msg = std::make_shared<ActorMessage>();
  msg->setOperationName(OP_INSERT);
  msg->setPayload(request);
  msg->setAttachment(STORE_ATTACH_KEY, key);
  msg->setAttachment(STORE_ATTACH_VALUE, customer);

  idgs::store::test::DataInsertActor* actor = new idgs::store::test::DataInsertActor;
  actor->init();
  idgs_application()->getRpcFramework()->getActorManager()->registerSessionActor(actor->getActorId(), actor);
  actor->process(msg);


  /// add by LT
  std::this_thread::sleep_for(std::chrono::seconds(2));

  if (resultCode != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR) << "Error in insert data, cause by " << msgs[resultCode];
    return;
  }
  ASSERT_EQ(idgs::store::pb::SRC_SUCCESS, resultCode);

  /// add by LT
  actor->terminate();
}

TEST(store, partition_store_get) {
  TEST_TIMEOUT(20);

  shared_ptr<idgs::store::pb::GetRequest> request = std::make_shared<idgs::store::pb::GetRequest>();
  request->set_schema_name("tpch");
  request->set_store_name("Customer");

  shared_ptr<CustomerKey> key = std::make_shared<CustomerKey>();
  key->set_c_custkey(10000);

  ActorMessagePtr msg = std::make_shared<ActorMessage>();
  msg->setOperationName(OP_GET);
  msg->setPayload(request);
  msg->setAttachment(STORE_ATTACH_KEY, key);

  idgs::store::test::DataGetActor* actor = new idgs::store::test::DataGetActor;
  actor->init();
  idgs_application()->getRpcFramework()->getActorManager()->registerSessionActor(actor->getActorId(), actor);
  actor->process(msg);

  /// add by LT
  std::this_thread::sleep_for(std::chrono::seconds(2));

  if (resultCode != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR) << "Error in get data, cause by " << msgs[resultCode];
    return;
  }
  ASSERT_EQ(idgs::store::pb::SRC_SUCCESS, resultCode);

  shared_ptr<Customer> result = std::make_shared<Customer>();
  bool res = protobuf::ProtoSerdesHelper::deserialize(protobuf::PB_BINARY, payload, result.get());
  ASSERT_TRUE(res);
  if (!res) {
    LOG(ERROR) << "Response value for get data cannot be parsed.";
    return;
  }

  LOG(INFO) << "Result value is " << result->DebugString();

  ASSERT_EQ(result->c_name(), customer->c_name());
  ASSERT_EQ(result->c_address(), customer->c_address());
  ASSERT_EQ(result->c_nationkey(), customer->c_nationkey());
  ASSERT_EQ(result->c_phone(), customer->c_phone());
  ASSERT_EQ(result->c_acctbal(), customer->c_acctbal());
  ASSERT_EQ(result->c_comment(), customer->c_comment());

  /// add by LT
  actor->terminate();
}

TEST(store, replicated_store_insert) {
  TEST_TIMEOUT(20);

  shared_ptr<idgs::store::pb::InsertRequest> request = std::make_shared<idgs::store::pb::InsertRequest>();
  request->set_schema_name("tpch");
  request->set_store_name("Nation");

  shared_ptr<NationKey> key = std::make_shared<NationKey>();
  key->set_n_nationkey(50000);

  ActorMessagePtr msg = std::make_shared<ActorMessage>();
  msg->setOperationName(OP_INSERT);
  msg->setPayload(request);
  msg->setAttachment(STORE_ATTACH_KEY, key);
  msg->setAttachment(STORE_ATTACH_VALUE, nation);

  idgs::store::test::DataInsertActor* actor = new idgs::store::test::DataInsertActor;
  actor->init();
  LOG(INFO) << "insert actor id : " << actor->getActorId();
  idgs_application()->getRpcFramework()->getActorManager()->registerSessionActor(actor->getActorId(), actor);
  actor->process(msg);

  /// add by LT
  std::this_thread::sleep_for(std::chrono::seconds(2));

  if (resultCode != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR) << "Error in insert data, cause by " << msgs[resultCode];
    return;
  }
  ASSERT_EQ(idgs::store::pb::SRC_SUCCESS, resultCode);

  /// add by LT
  actor->terminate();
}

TEST(store, replicated_store_get) {
  TEST_TIMEOUT(20);

  shared_ptr<idgs::store::pb::GetRequest> request = std::make_shared<idgs::store::pb::GetRequest>();
  request->set_schema_name("tpch");
  request->set_store_name("Nation");

  shared_ptr<NationKey> key = std::make_shared<NationKey>();
  key->set_n_nationkey(50000);

  ActorMessagePtr msg = std::make_shared<ActorMessage>();
  msg->setOperationName(OP_GET);
  msg->setPayload(request);
  msg->setAttachment(STORE_ATTACH_KEY, key);

  idgs::store::test::DataGetActor* actor = new idgs::store::test::DataGetActor;
  actor->init();
  idgs_application()->getRpcFramework()->getActorManager()->registerSessionActor(actor->getActorId(), actor);
  actor->process(msg);

  /// add by LT
  std::this_thread::sleep_for(std::chrono::seconds(2));

  ASSERT_EQ(idgs::store::pb::SRC_SUCCESS, resultCode);
  if (resultCode != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR) << "Error in get data, cause by " << msgs[resultCode];
    return;
  }

  shared_ptr<Nation> result = std::make_shared<Nation>();
  bool res = protobuf::ProtoSerdesHelper::deserialize(protobuf::PB_BINARY, payload, result.get());
  ASSERT_TRUE(res);
  if (!res) {
    LOG(ERROR) << "Response value for get data cannot be parsed.";
    return;
  }

  LOG(INFO) << "Result value is " << result->DebugString();

  ASSERT_EQ(result->n_name(), nation->n_name());
  ASSERT_EQ(result->n_regionkey(), nation->n_regionkey());
  ASSERT_EQ(result->n_comment(), nation->n_comment());

  /// add by LT
  actor->terminate();
}

TEST(store, stop) {
  idgs_application()->shutdown();
  ResultCode rc = idgs_application()->stop();
  ASSERT_EQ(RC_SUCCESS, rc);

  exit(0);
}
