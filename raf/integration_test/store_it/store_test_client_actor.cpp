
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
shared_ptr<Customer> customer(new Customer);
shared_ptr<Nation> nation(new Nation);
idgs::store::pb::StoreResultCode resultCode;

namespace {
  struct ApplicationSetting {
    ApplicationSetting():clusterConfig("") {}
    std::string clusterConfig;
  };
}

void startServer() {
  ApplicationSetting setting;
  setting.clusterConfig = "framework/conf/cluster.conf";

  Application& app = ::idgs::util::singleton<Application>::getInstance();
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

  sleep(3);
}

void init() {
  customer->set_c_name("Tom");
  customer->set_c_nationkey(10);
  customer->set_c_address("address");
  customer->set_c_phone("13800138000");
  customer->set_c_acctbal(100.123);
  customer->set_c_comment("customer store test");

  nation->set_n_name("China");
  nation->set_n_regionkey(35000);
  nation->set_n_comment("Intel");

  msgs.resize(13);
  msgs[idgs::store::pb::SRC_SUCCESS] = "Success";
  msgs[idgs::store::pb::SRC_KEY_EXIST] = "Key is already exists.";
  msgs[idgs::store::pb::SRC_KEY_NOT_EXIST] = "Key is not exists";
  msgs[idgs::store::pb::SRC_VERSION_CONFLICT] = "Version conflict.";
  msgs[idgs::store::pb::SRC_TABLE_NOT_EXIST] = "Table not found.";
  msgs[idgs::store::pb::SRC_DATA_NOT_FOUND] = "Data not found.";
  msgs[idgs::store::pb::SRC_PARTITION_NOT_FOUND] = "Partition not found.";
  msgs[idgs::store::pb::SRC_PARTITION_NOT_READY] = "Partition is not ready.";
  msgs[idgs::store::pb::SRC_INVALID_KEY] = "Invalid key.";
  msgs[idgs::store::pb::SRC_INVALID_VALUE] = "Invalid value.";
  msgs[idgs::store::pb::SRC_INVALID_FILTER] = "Invalid filter.";
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
                {OP_INSERT,                         static_cast<idgs::actor::ActorMessageHandler>(&DataInsertActor::handleInsertRequest)},
                {DATA_STORE_INSERT_RESPONSE,                         static_cast<idgs::actor::ActorMessageHandler>(&DataInsertActor::handleInsertResponse)},
            };
            return handlerMap;
          }


          const std::string& getActorName() const override {
            return "DataInsertActor";
          }

          const idgs::actor::ActorDescriptorPtr& getDescriptor() const {
            static idgs::actor::ActorDescriptorPtr descriptor;
            if(!descriptor) {
              descriptor.reset(new idgs::actor::ActorDescriptorWrapper);
              descriptor->setName(getActorName());
              descriptor->setDescription("Test insert data store.");
              descriptor->setType(AT_STATEFUL);

              idgs::actor::ActorOperationDescriporWrapper dataInsertRequest;
              dataInsertRequest.setName(OP_INSERT);
              dataInsertRequest.setDescription("Insert request.");
              dataInsertRequest.setPayloadType("idgs.store.pb.InsertRequest");
              descriptor->setInOperation(dataInsertRequest.getName(), dataInsertRequest);

              idgs::actor::ActorOperationDescriporWrapper dataInsertResponse;
              dataInsertResponse.setName(DATA_STORE_INSERT_RESPONSE);
              dataInsertResponse.setDescription("Insert response.");
              dataInsertResponse.setPayloadType("idgs.store.pb.InsertResponse");
              descriptor->setInOperation(dataInsertResponse.getName(), dataInsertResponse);
            }
            return descriptor;
          }

          void handleInsertRequest(const ActorMessagePtr& msg) {
            msg->setOperationName(OP_INSERT);
            msg->setChannel(TC_AUTO);
            msg->setSourceActorId(getActorId());
            msg->setSourceMemberId(1);
            msg->setDestActorId(ACTORID_STORE_SERVCIE);
            msg->setDestMemberId(0);

            idgs::actor::sendMessage(const_cast<ActorMessagePtr&>(msg));
          }

          void handleInsertResponse(const ActorMessagePtr& msg) {
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
                {OP_GET,                         static_cast<idgs::actor::ActorMessageHandler>(&DataGetActor::handleGetRequest)},
                {DATA_STORE_GET_RESPONSE,                         static_cast<idgs::actor::ActorMessageHandler>(&DataGetActor::handleGetResponse)},
            };
            return handlerMap;
          }

          const std::string& getActorName() const override{
            static const std::string actorName("DataGetActor");
            return actorName;
          }


          const idgs::actor::ActorDescriptorPtr& getDescriptor() const {
            static idgs::actor::ActorDescriptorPtr descriptor(new idgs::actor::ActorDescriptorWrapper);
            descriptor->setName(getActorName());
            descriptor->setDescription("Test insert data store.");
            descriptor->setType(AT_STATEFUL);

            idgs::actor::ActorOperationDescriporWrapper dataGetRequest;
            dataGetRequest.setName(OP_GET);
            dataGetRequest.setDescription("Get request.");
            dataGetRequest.setPayloadType("idgs.store.pb.GetRequest");
            descriptor->setInOperation(dataGetRequest.getName(), dataGetRequest);

            idgs::actor::ActorOperationDescriporWrapper dataGetResponse;
            dataGetResponse.setName(DATA_STORE_GET_RESPONSE);
            dataGetResponse.setDescription("Get response.");
            dataGetResponse.setPayloadType("idgs.store.pb.GetResponse");
            descriptor->setInOperation(dataGetResponse.getName(), dataGetResponse);

            return descriptor;
          }


          void handleGetRequest(const ActorMessagePtr& msg) {
            msg->setOperationName(OP_GET);
            msg->setChannel(TC_AUTO);
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
  TEST_TIMEOUT(10);
  init();
  startServer();

  shared_ptr<idgs::store::pb::InsertRequest> request(new idgs::store::pb::InsertRequest);
  request->set_store_name("Customer");

  shared_ptr<CustomerKey> key(new CustomerKey);
  key->set_c_custkey(10000);

  ActorMessagePtr msg(new ActorMessage);
  msg->setOperationName(OP_INSERT);
  msg->setPayload(request);
  msg->setAttachment(STORE_ATTACH_KEY, key);
  msg->setAttachment(STORE_ATTACH_VALUE, customer);

  idgs::store::test::DataInsertActor* actor = new idgs::store::test::DataInsertActor;
  actor->init();
  ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework()->Register(actor->getActorId(), actor);
  actor->process(msg);

  sleep(5);

  ASSERT_EQ(idgs::store::pb::SRC_SUCCESS, resultCode);
  if (resultCode != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR) << "Error in insert data, cause by " << msgs[resultCode];
    return;
  }
}

TEST(store, partition_store_get) {
  TEST_TIMEOUT(10);

  shared_ptr<idgs::store::pb::GetRequest> request(new idgs::store::pb::GetRequest);
  request->set_store_name("Customer");

  shared_ptr<CustomerKey> key(new CustomerKey);
  key->set_c_custkey(10000);

  ActorMessagePtr msg(new ActorMessage);
  msg->setOperationName(OP_GET);
  msg->setPayload(request);
  msg->setAttachment(STORE_ATTACH_KEY, key);

  idgs::store::test::DataGetActor* actor = new idgs::store::test::DataGetActor;
  actor->init();
  ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework()->Register(actor->getActorId(), actor);
  actor->process(msg);

  ASSERT_EQ(idgs::store::pb::SRC_SUCCESS, resultCode);
  if (resultCode != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR) << "Error in get data, cause by " << msgs[resultCode];
    return;
  }

  sleep(5);

  shared_ptr<Customer> result(new Customer);
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
}

TEST(store, replicated_store_insert) {
  TEST_TIMEOUT(10);

  shared_ptr<idgs::store::pb::InsertRequest> request(new idgs::store::pb::InsertRequest);
  request->set_store_name("Nation");

  shared_ptr<NationKey> key(new NationKey);
  key->set_n_nationkey(50000);

  ActorMessagePtr msg(new ActorMessage);
  msg->setOperationName(OP_INSERT);
  msg->setPayload(request);
  msg->setAttachment(STORE_ATTACH_KEY, key);
  msg->setAttachment(STORE_ATTACH_VALUE, nation);

  idgs::store::test::DataInsertActor* actor = new idgs::store::test::DataInsertActor;
  actor->init();
  LOG(INFO) << "insert actor id : " << actor->getActorId();
  ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework()->Register(actor->getActorId(), actor);
  actor->process(msg);

  sleep(5);

  ASSERT_EQ(idgs::store::pb::SRC_SUCCESS, resultCode);
  if (resultCode != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR) << "Error in insert data, cause by " << msgs[resultCode];
    return;
  }

}

TEST(store, replicated_store_get) {
  TEST_TIMEOUT(10);

  shared_ptr<idgs::store::pb::GetRequest> request(new idgs::store::pb::GetRequest);
  request->set_store_name("Nation");

  shared_ptr<NationKey> key(new NationKey);
  key->set_n_nationkey(50000);

  ActorMessagePtr msg(new ActorMessage);
  msg->setOperationName(OP_GET);
  msg->setPayload(request);
  msg->setAttachment(STORE_ATTACH_KEY, key);

  idgs::store::test::DataGetActor* actor = new idgs::store::test::DataGetActor;
  actor->init();
  ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework()->Register(actor->getActorId(), actor);
  actor->process(msg);

  ASSERT_EQ(idgs::store::pb::SRC_SUCCESS, resultCode);
  if (resultCode != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR) << "Error in get data, cause by " << msgs[resultCode];
    return;
  }

  sleep(5);

  shared_ptr<Nation> result(new Nation);
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
}

TEST(store, stop) {
  idgs::util::singleton<Application>::getInstance().shutdown();
  ResultCode rc = ::idgs::util::singleton<Application>::getInstance().stop();
  ASSERT_EQ(RC_SUCCESS, rc);

  exit(0);
}
