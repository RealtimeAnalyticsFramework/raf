
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include "gtest/gtest.h"

#include "idgs/client/client_pool.h"
#include "idgs/client/rdd/rdd_client.h"

#include "idgs/expr/expression_helper.h"

#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"

#include "idgs/store/datastore_const.h"

using namespace std;
using namespace idgs::client;
using namespace idgs::pb;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;
using namespace idgs::store;
using namespace protobuf;

namespace idgs {
namespace rdd {
namespace it_rdd_store_listener {

RddClient client;

void init() {
  client.init("conf/client.conf");
}

void buildRdd() {
  DelegateRddRequestPtr delegateRddRequest = make_shared<CreateDelegateRddRequest>();
  DelegateRddResponsePtr delegateRddResponse = make_shared<CreateDelegateRddResponse>();
  delegateRddRequest->set_schema_name("tpch");
  delegateRddRequest->set_store_name("LineItem");
  delegateRddRequest->set_rdd_name("LINE_ITEM_DELEGATE");

  client.createStoreDelegateRDD(delegateRddRequest, delegateRddResponse);

  RddRequestPtr filterRequest = make_shared<CreateRddRequest>();
  RddResponsePtr filterResponse = make_shared<CreateRddResponse>();
  filterRequest->set_transformer_op_name(FILTER_TRANSFORMER);

  auto in = filterRequest->add_in_rdd();
  in->set_rdd_name("LINE_ITEM_DELEGATE");

  auto out = filterRequest->mutable_out_rdd();
  out->set_rdd_name("FILTER_LINE_ITEM");
  out->set_key_type_name("idgs.sample.tpch.pb.LineItemKey");
  out->set_value_type_name("idgs.sample.tpch.pb.LineItem");

  in->set_allocated_filter_expr(AND(GE(FIELD("l_shipdate"), CONST("1994-01-01")),
                                LT(FIELD("l_shipdate"), CONST("1995-01-01")),
                                GE(FIELD("l_discount"), CONST("0.05", DOUBLE)),
                                LE(FIELD("l_discount"), CONST("0.07", DOUBLE)),
                                LT(FIELD("l_quantity"), CONST("24", DOUBLE))
                               ));

  client.createRdd(filterRequest, filterResponse);
}

double action(const std::string& actionId) {
  ActionRequestPtr actionRequest = make_shared<ActionRequest>();
  ActionResponsePtr actionResponse = make_shared<ActionResponse>();
  ActionResultPtr actionResult = make_shared<SumActionResult>();

  actionRequest->set_action_id(actionId);
  actionRequest->set_rdd_name("FILTER_LINE_ITEM");
  actionRequest->set_action_op_name(SUM_ACTION);
  actionRequest->set_allocated_expression(MULTIPLY(FIELD("l_extendedprice"), FIELD("l_discount")));

  client.sendAction(actionRequest, actionResponse, actionResult);
  if (actionResponse->result_code() != RRC_SUCCESS) {
    return -1;
  }

  auto result = dynamic_cast<SumActionResult*>(actionResult.get());
  return result->total();
}

}
}
}

TEST(rdd_store_listener, insert) {
  idgs::rdd::it_rdd_store_listener::init();

  LOG(INFO) << "1. Build RDD";
  idgs::rdd::it_rdd_store_listener::buildRdd();

  LOG(INFO) << "2. Send action and get result";
  double result100000 = idgs::rdd::it_rdd_store_listener::action("100000");
  LOG(INFO) << "First action result is : " << std::fixed << result100000;
  ASSERT_NE(-1, result100000);

  LOG(INFO) << "3. Insert one data to store LineItem";

  MessageHelper helper;
  helper.registerDynamicMessage("conf/tpch.proto");

  std::shared_ptr<idgs::store::pb::InsertRequest> request = std::make_shared<idgs::store::pb::InsertRequest>();
  request->set_store_name("LineItem");

  auto key = helper.createMessage("idgs.sample.tpch.pb.LineItemKey");
  key->GetReflection()->SetInt64(key.get(), key->GetDescriptor()->FindFieldByName("l_orderkey"), 1);
  key->GetReflection()->SetInt64(key.get(), key->GetDescriptor()->FindFieldByName("l_linenumber"), 1);
  auto value = helper.createMessage("idgs.sample.tpch.pb.LineItem");
  value->GetReflection()->SetInt64(value.get(), value->GetDescriptor()->FindFieldByName("l_partkey"), 1);
  value->GetReflection()->SetInt64(value.get(), value->GetDescriptor()->FindFieldByName("l_suppkey"), 1);
  value->GetReflection()->SetDouble(value.get(), value->GetDescriptor()->FindFieldByName("l_quantity"), 20);
  value->GetReflection()->SetDouble(value.get(), value->GetDescriptor()->FindFieldByName("l_extendedprice"), 5000);
  value->GetReflection()->SetDouble(value.get(), value->GetDescriptor()->FindFieldByName("l_discount"), 0.06);
  value->GetReflection()->SetString(value.get(), value->GetDescriptor()->FindFieldByName("l_shipdate"), "1994-05-09");

  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName(OP_INSERT);
  clientActorMsg->setChannel(TC_TCP);
  clientActorMsg->setDestActorId(ACTORID_STORE_SERVCIE);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setPayload(request);

  clientActorMsg->setAttachment(STORE_ATTACH_KEY, key);
  clientActorMsg->setAttachment(STORE_ATTACH_VALUE, value);

  idgs::ResultCode code = idgs::RC_SUCCESS;
  auto client = getTcpClientPool().getTcpClient(code);
  ASSERT_EQ(idgs::RC_SUCCESS, code);

  // response
  ClientActorMessagePtr tcpResponse;
  code = client->sendRecv(clientActorMsg, tcpResponse);
  ASSERT_EQ(idgs::RC_SUCCESS, code);

  idgs::store::pb::InsertResponse response;
  bool res = tcpResponse->parsePayload(&response);
  ASSERT_TRUE(res);

  if (response.result_code() != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR) << "Error in insert data to partition store, cause by error code : " << StoreResultCode_Name(response.result_code());
  }
  ASSERT_EQ(idgs::store::pb::SRC_SUCCESS, response.result_code());

  sleep(3);

  LOG(INFO) << "4. Send action again and get new result";
  double result100001 = idgs::rdd::it_rdd_store_listener::action("100001");
  LOG(INFO) << "Second action result is : " << std::fixed << result100001;
  ASSERT_NE(-1, result100001);

  LOG(INFO) << "Assert the second result is 300 more than the first.";
  ASSERT_DOUBLE_EQ(300, result100001 - result100000);
}
