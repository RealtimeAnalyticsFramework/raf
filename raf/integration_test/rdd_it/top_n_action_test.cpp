
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "gtest/gtest.h"


#include "idgs/client/rdd/rdd_client.h"

#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"


using namespace std;
using namespace idgs;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;
using namespace protobuf;

namespace idgs {
namespace rdd {
namespace top_n_test {

RddClient client;
TopNActionResult ascTop10Result;

}
}
}


TEST(top_n_action, asc) {
  ResultCode code = idgs::rdd::top_n_test::client.init("conf/client.conf");
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "init rdd client error, caused by " << getErrorDescription(code);
  }

  LOG(INFO)<< "create store delegate, store_name: LineItemRDD";
  DelegateRddRequestPtr request = std::make_shared<CreateDelegateRddRequest>();
  DelegateRddResponsePtr response = std::make_shared<CreateDelegateRddResponse>();
  request->set_schema_name("tpch");
  request->set_store_name("LineItem");
  request->set_rdd_name("LineItemRDD");
  idgs::rdd::top_n_test::client.createStoreDelegateRDD(request, response);

  ActionRequestPtr actionRequest = std::make_shared<ActionRequest>();
  ActionResponsePtr actionResponse = std::make_shared<ActionResponse>();
  ActionResultPtr actionResult = std::make_shared<TopNActionResult>();

  actionRequest->set_action_id("topNActionTest1");
  actionRequest->set_rdd_name("LineItemRDD");
  actionRequest->set_action_op_name(idgs::rdd::TOP_N_ACTION);

  shared_ptr<TopNActionRequest> actionParam = std::make_shared<TopNActionRequest>();
  actionParam->set_top_n(10);
  auto field = actionParam->add_order_field();
  auto fldExpr = field->mutable_expr();
  fldExpr->set_name("FIELD");
  fldExpr->set_value("l_extendedprice");
  field->set_desc(false);

  AttachMessage params;
  params[ACTION_PARAM] = actionParam;
  idgs::rdd::top_n_test::client.sendAction(actionRequest, actionResponse, actionResult, params);

  EXPECT_EQ(actionResponse->action_id(), "topNActionTest1");
  EXPECT_EQ(actionResponse->result_code(), RRC_SUCCESS);
  TopNActionResult* result = dynamic_cast<TopNActionResult*>(actionResult.get());

  auto& storeConfigWrapper = idgs::rdd::top_n_test::client.getStoreConfigWrapper("LineItem");

  EXPECT_EQ(result->pair_size(), 10);
  LOG(INFO) << "======= l_extendedprice ======= ";
  for (int32_t i = 0; i < result->pair_size(); ++ i) {
    auto svalue = result->pair(i).value();
    auto value = storeConfigWrapper->newValue();
    ProtoSerdes<0>::deserialize(svalue, value.get());

    auto ref = value->GetReflection();
    auto fld = value->GetDescriptor()->FindFieldByName("l_extendedprice");
    double price = ref->GetDouble(* value, fld);

    if (i + 1 < result->pair_size()) {
      auto sNextValue = result->pair(i + 1).value();
      auto nextValue = storeConfigWrapper->newValue();;
      ProtoSerdes<0>::deserialize(sNextValue, nextValue.get());
      double nextPrice = ref->GetDouble(* nextValue, fld);

      EXPECT_LE(price, nextPrice);
    }

    LOG(INFO) << "  " << price;
  }
  LOG(INFO) << "=============================== ";

  idgs::rdd::top_n_test::ascTop10Result.CopyFrom(* result);
}

TEST(top_n_action, desc) {
  ActionRequestPtr actionRequest = std::make_shared<ActionRequest>();
  ActionResponsePtr actionResponse = std::make_shared<ActionResponse>();
  ActionResultPtr actionResult = std::make_shared<TopNActionResult>();

  actionRequest->set_action_id("topNActionTest2");
  actionRequest->set_rdd_name("LineItemRDD");
  actionRequest->set_action_op_name(idgs::rdd::TOP_N_ACTION);

  shared_ptr<TopNActionRequest> actionParam = std::make_shared<TopNActionRequest>();
  actionParam->set_top_n(10);
  auto field = actionParam->add_order_field();
  auto fldExpr = field->mutable_expr();
  fldExpr->set_name("FIELD");
  fldExpr->set_value("l_extendedprice");
  field->set_desc(true);

  AttachMessage params;
  params[ACTION_PARAM] = actionParam;
  idgs::rdd::top_n_test::client.sendAction(actionRequest, actionResponse, actionResult, params);

  EXPECT_EQ(actionResponse->action_id(), "topNActionTest2");
  EXPECT_EQ(actionResponse->result_code(), RRC_SUCCESS);
  TopNActionResult* result = dynamic_cast<TopNActionResult*>(actionResult.get());

  auto& storeConfigWrapper = idgs::rdd::top_n_test::client.getStoreConfigWrapper("LineItem");

  EXPECT_EQ(result->pair_size(), 10);
  LOG(INFO) << "======= l_extendedprice ======= ";
  for (int32_t i = 0; i < result->pair_size(); ++ i) {
    auto svalue = result->pair(i).value();
    auto value = storeConfigWrapper->newValue();
    ProtoSerdes<0>::deserialize(svalue, value.get());

    auto ref = value->GetReflection();
    auto fld = value->GetDescriptor()->FindFieldByName("l_extendedprice");
    double price = ref->GetDouble(* value, fld);

    if (i + 1 < result->pair_size()) {
      auto sNextValue = result->pair(i + 1).value();
      auto nextValue = storeConfigWrapper->newValue();
      ProtoSerdes<0>::deserialize(sNextValue, nextValue.get());
      double nextPrice = ref->GetDouble(* nextValue, fld);

      EXPECT_GE(price, nextPrice);
    }

    LOG(INFO) << "  " << price;
  }
  LOG(INFO) << "=============================== ";
}

TEST(top_n_action, expression) {
  ActionRequestPtr actionRequest = std::make_shared<ActionRequest>();
  ActionResponsePtr actionResponse = std::make_shared<ActionResponse>();
  ActionResultPtr actionResult = std::make_shared<TopNActionResult>();

  actionRequest->set_action_id("topNActionTest3");
  actionRequest->set_rdd_name("LineItemRDD");
  actionRequest->set_action_op_name(idgs::rdd::TOP_N_ACTION);

  shared_ptr<TopNActionRequest> actionParam = std::make_shared<TopNActionRequest>();
  actionParam->set_top_n(10);
  auto field = actionParam->add_order_field();
  auto fldExpr = field->mutable_expr();
  fldExpr->set_name("MULTIPLY");
  field->set_desc(true);

  auto fld = fldExpr->add_expression();
  fld->set_name("FIELD");
  fld->set_value("l_extendedprice");

  fld = fldExpr->add_expression();
  fld->set_name("FIELD");
  fld->set_value("l_discount");

  AttachMessage params;
  params[ACTION_PARAM] = actionParam;
  idgs::rdd::top_n_test::client.sendAction(actionRequest, actionResponse, actionResult, params);

  EXPECT_EQ(actionResponse->action_id(), "topNActionTest3");
  EXPECT_EQ(actionResponse->result_code(), RRC_SUCCESS);
  TopNActionResult* result = dynamic_cast<TopNActionResult*>(actionResult.get());

  auto& storeConfigWrapper = idgs::rdd::top_n_test::client.getStoreConfigWrapper("LineItem");

  EXPECT_EQ(result->pair_size(), 10);
  LOG(INFO) << "======= l_extendedprice * l_discount =======";
  for (int32_t i = 0; i < result->pair_size(); ++ i) {
    auto svalue = result->pair(i).value();
    auto value = storeConfigWrapper->newValue();
    ProtoSerdes<0>::deserialize(svalue, value.get());

    auto ref = value->GetReflection();
    auto fldPrice = value->GetDescriptor()->FindFieldByName("l_extendedprice");
    double price = ref->GetDouble(* value, fldPrice);
    auto fldDiscount = value->GetDescriptor()->FindFieldByName("l_discount");
    double discount = ref->GetDouble(* value, fldDiscount);

    if (i + 1 < result->pair_size()) {
      auto sNextValue = result->pair(i + 1).value();
      auto nextValue = storeConfigWrapper->newValue();
      ProtoSerdes<0>::deserialize(sNextValue, nextValue.get());
      double nextPrice = ref->GetDouble(* nextValue, fldPrice);
      double nextDiscount = ref->GetDouble(* nextValue, fldDiscount);

      EXPECT_GE(price * discount, nextPrice * nextDiscount);
    }

    LOG(INFO) << "  " << price * discount;
  }
  LOG(INFO) << "============================================";
}

TEST(top_n_action, limit) {
  ActionRequestPtr actionRequest = std::make_shared<ActionRequest>();
  ActionResponsePtr actionResponse = std::make_shared<ActionResponse>();
  ActionResultPtr actionResult = std::make_shared<TopNActionResult>();

  actionRequest->set_action_id("topNActionTest4");
  actionRequest->set_rdd_name("LineItemRDD");
  actionRequest->set_action_op_name(idgs::rdd::TOP_N_ACTION);

  shared_ptr<TopNActionRequest> actionParam = std::make_shared<TopNActionRequest>();
  actionParam->set_top_n(5);
  actionParam->set_start(5);
  auto field = actionParam->add_order_field();
  auto fldExpr = field->mutable_expr();
  fldExpr->set_name("FIELD");
  fldExpr->set_value("l_extendedprice");
  field->set_desc(false);

  AttachMessage params;
  params[ACTION_PARAM] = actionParam;
  idgs::rdd::top_n_test::client.sendAction(actionRequest, actionResponse, actionResult, params);

  EXPECT_EQ(actionResponse->action_id(), "topNActionTest4");
  EXPECT_EQ(actionResponse->result_code(), RRC_SUCCESS);
  TopNActionResult* result = dynamic_cast<TopNActionResult*>(actionResult.get());

  auto& storeConfigWrapper = idgs::rdd::top_n_test::client.getStoreConfigWrapper("LineItem");

  EXPECT_EQ(result->pair_size(), 5);

  LOG(INFO) << "========= top 5 start with index 5 =========";
  for (int32_t i = 0, j = 5 - 1; i < result->pair_size(); ++ i, ++ j) {
    auto svalue = result->pair(i).value();
    auto value = storeConfigWrapper->newValue();
    ProtoSerdes<0>::deserialize(svalue, value.get());

    auto sTop10Value = idgs::rdd::top_n_test::ascTop10Result.pair(j).value();
    auto top10Value = storeConfigWrapper->newValue();;
    ProtoSerdes<0>::deserialize(sTop10Value, top10Value.get());

    auto ref = value->GetReflection();
    auto fld = value->GetDescriptor()->FindFieldByName("l_extendedprice");
    double price = ref->GetDouble(* value, fld);
    double top10Price = ref->GetDouble(* top10Value, fld);

    VLOG(0) << "  " << price;
    EXPECT_EQ(price, top10Price);
  }

  LOG(INFO) << "============================================";
}
