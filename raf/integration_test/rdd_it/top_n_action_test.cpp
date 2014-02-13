
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "gtest/gtest.h"
#include "idgs/client/rdd/rdd_client.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"
#include "idgs/store/data_store.h"
#include "protobuf/message_helper.h"

using namespace std;
using namespace idgs;
using namespace idgs::util;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::store;
using namespace idgs::pb;
using namespace idgs::client::rdd;
using namespace protobuf;

TopNActionResult ascTop10Result;

TEST(top_n_action, asc) {
  singleton<DataStore>::getInstance().loadCfgFile("services/store/test/data_store.conf");

  RddClient& client = singleton<RddClient>::getInstance();
  ResultCode code = client.init("integration_test/rdd_it/client.conf");
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "init rdd client error, caused by " << getErrorDescription(code);
  }

  LOG(INFO)<< "create store delegate, store_name: LineItemRDD";
  DelegateRddRequestPtr request(new CreateDelegateRddRequest);
  DelegateRddResponsePtr response(new CreateDelegateRddResponse);
  request->set_store_name("LineItem");
  request->set_rdd_name("LineItemRDD");
  client.createStoreDelegateRDD(request, response);

  ActionRequestPtr actionRequest(new ActionRequest);
  ActionResponsePtr actionResponse(new ActionResponse);
  ActionResultPtr actionResult(new TopNActionResult);

  actionRequest->set_action_id("topNActionTest1");
  actionRequest->set_rdd_name("LineItemRDD");
  actionRequest->set_action_op_name(idgs::rdd::TOP_N_ACTION);

  shared_ptr<TopNActionRequest> actionParam(new TopNActionRequest);
  actionParam->set_top_n(10);
  auto field = actionParam->add_order_field();
  auto fldExpr = field->mutable_expr();
  fldExpr->set_type(FIELD);
  fldExpr->set_value("l_extendedprice");
  field->set_desc(false);

  AttachMessage params;
  params[ACTION_PARAM] = actionParam;
  client.sendAction(actionRequest, actionResponse, actionResult, params);

  EXPECT_EQ(actionResponse->action_id(), "topNActionTest1");
  EXPECT_EQ(actionResponse->result_code(), RRC_SUCCESS);
  TopNActionResult* result = dynamic_cast<TopNActionResult*>(actionResult.get());

  auto& helper = singleton<MessageHelper>::getInstance();
  EXPECT_EQ(result->pair_size(), 10);
  LOG(INFO) << "======= l_extendedprice ======= ";
  for (int32_t i = 0; i < result->pair_size(); ++ i) {
    auto svalue = result->pair(i).value();
    auto value = helper.createMessage("idgs.sample.tpch.pb.LineItem");
    ProtoSerdes<0>::deserialize(svalue, value.get());

    auto ref = value->GetReflection();
    auto fld = value->GetDescriptor()->FindFieldByName("l_extendedprice");
    double price = ref->GetDouble(* value, fld);

    if (i + 1 < result->pair_size()) {
      auto sNextValue = result->pair(i + 1).value();
      auto nextValue = helper.createMessage("idgs.sample.tpch.pb.LineItem");
      ProtoSerdes<0>::deserialize(sNextValue, nextValue.get());
      double nextPrice = ref->GetDouble(* nextValue, fld);

      EXPECT_LE(price, nextPrice);
    }

    LOG(INFO) << "  " << price;
  }
  LOG(INFO) << "=============================== ";

  ascTop10Result.CopyFrom(* result);
}

TEST(top_n_action, desc) {
  RddClient& client = singleton<RddClient>::getInstance();

  ActionRequestPtr actionRequest(new ActionRequest);
  ActionResponsePtr actionResponse(new ActionResponse);
  ActionResultPtr actionResult(new TopNActionResult);

  actionRequest->set_action_id("topNActionTest2");
  actionRequest->set_rdd_name("LineItemRDD");
  actionRequest->set_action_op_name(idgs::rdd::TOP_N_ACTION);

  shared_ptr<TopNActionRequest> actionParam(new TopNActionRequest);
  actionParam->set_top_n(10);
  auto field = actionParam->add_order_field();
  auto fldExpr = field->mutable_expr();
  fldExpr->set_type(FIELD);
  fldExpr->set_value("l_extendedprice");
  field->set_desc(true);

  AttachMessage params;
  params[ACTION_PARAM] = actionParam;
  client.sendAction(actionRequest, actionResponse, actionResult, params);

  EXPECT_EQ(actionResponse->action_id(), "topNActionTest2");
  EXPECT_EQ(actionResponse->result_code(), RRC_SUCCESS);
  TopNActionResult* result = dynamic_cast<TopNActionResult*>(actionResult.get());

  auto& helper = singleton<MessageHelper>::getInstance();

  EXPECT_EQ(result->pair_size(), 10);
  LOG(INFO) << "======= l_extendedprice ======= ";
  for (int32_t i = 0; i < result->pair_size(); ++ i) {
    auto svalue = result->pair(i).value();
    auto value = helper.createMessage("idgs.sample.tpch.pb.LineItem");
    ProtoSerdes<0>::deserialize(svalue, value.get());

    auto ref = value->GetReflection();
    auto fld = value->GetDescriptor()->FindFieldByName("l_extendedprice");
    double price = ref->GetDouble(* value, fld);

    if (i + 1 < result->pair_size()) {
      auto sNextValue = result->pair(i + 1).value();
      auto nextValue = helper.createMessage("idgs.sample.tpch.pb.LineItem");
      ProtoSerdes<0>::deserialize(sNextValue, nextValue.get());
      double nextPrice = ref->GetDouble(* nextValue, fld);

      EXPECT_GE(price, nextPrice);
    }

    LOG(INFO) << "  " << price;
  }
  LOG(INFO) << "=============================== ";
}

TEST(top_n_action, expression) {
  RddClient& client = singleton<RddClient>::getInstance();

  ActionRequestPtr actionRequest(new ActionRequest);
  ActionResponsePtr actionResponse(new ActionResponse);
  ActionResultPtr actionResult(new TopNActionResult);

  actionRequest->set_action_id("topNActionTest3");
  actionRequest->set_rdd_name("LineItemRDD");
  actionRequest->set_action_op_name(idgs::rdd::TOP_N_ACTION);

  shared_ptr<TopNActionRequest> actionParam(new TopNActionRequest);
  actionParam->set_top_n(10);
  auto field = actionParam->add_order_field();
  auto fldExpr = field->mutable_expr();
  fldExpr->set_type(MULTIPLY);
  field->set_desc(true);

  auto fld = fldExpr->add_expression();
  fld->set_type(FIELD);
  fld->set_value("l_extendedprice");

  fld = fldExpr->add_expression();
  fld->set_type(FIELD);
  fld->set_value("l_discount");

  AttachMessage params;
  params[ACTION_PARAM] = actionParam;
  client.sendAction(actionRequest, actionResponse, actionResult, params);

  EXPECT_EQ(actionResponse->action_id(), "topNActionTest3");
  EXPECT_EQ(actionResponse->result_code(), RRC_SUCCESS);
  TopNActionResult* result = dynamic_cast<TopNActionResult*>(actionResult.get());

  auto& helper = singleton<MessageHelper>::getInstance();
  EXPECT_EQ(result->pair_size(), 10);
  LOG(INFO) << "======= l_extendedprice * l_discount =======";
  for (int32_t i = 0; i < result->pair_size(); ++ i) {
    auto svalue = result->pair(i).value();
    auto value = helper.createMessage("idgs.sample.tpch.pb.LineItem");
    ProtoSerdes<0>::deserialize(svalue, value.get());

    auto ref = value->GetReflection();
    auto fldPrice = value->GetDescriptor()->FindFieldByName("l_extendedprice");
    double price = ref->GetDouble(* value, fldPrice);
    auto fldDiscount = value->GetDescriptor()->FindFieldByName("l_discount");
    double discount = ref->GetDouble(* value, fldDiscount);

    if (i + 1 < result->pair_size()) {
      auto sNextValue = result->pair(i + 1).value();
      auto nextValue = helper.createMessage("idgs.sample.tpch.pb.LineItem");
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
  RddClient& client = singleton<RddClient>::getInstance();

  ActionRequestPtr actionRequest(new ActionRequest);
  ActionResponsePtr actionResponse(new ActionResponse);
  ActionResultPtr actionResult(new TopNActionResult);

  actionRequest->set_action_id("topNActionTest4");
  actionRequest->set_rdd_name("LineItemRDD");
  actionRequest->set_action_op_name(idgs::rdd::TOP_N_ACTION);

  shared_ptr<TopNActionRequest> actionParam(new TopNActionRequest);
  actionParam->set_top_n(5);
  actionParam->set_start(5);
  auto field = actionParam->add_order_field();
  auto fldExpr = field->mutable_expr();
  fldExpr->set_type(FIELD);
  fldExpr->set_value("l_extendedprice");
  field->set_desc(false);

  AttachMessage params;
  params[ACTION_PARAM] = actionParam;
  client.sendAction(actionRequest, actionResponse, actionResult, params);

  EXPECT_EQ(actionResponse->action_id(), "topNActionTest4");
  EXPECT_EQ(actionResponse->result_code(), RRC_SUCCESS);
  TopNActionResult* result = dynamic_cast<TopNActionResult*>(actionResult.get());

  auto& helper = singleton<MessageHelper>::getInstance();

  EXPECT_EQ(result->pair_size(), 5);

  LOG(INFO) << "========= top 5 start with index 5 =========";
  for (int32_t i = 0, j = 5 - 1; i < result->pair_size(); ++ i, ++ j) {
    auto svalue = result->pair(i).value();
    auto value = helper.createMessage("idgs.sample.tpch.pb.LineItem");
    ProtoSerdes<0>::deserialize(svalue, value.get());

    auto sTop10Value = ascTop10Result.pair(j).value();
    auto top10Value = helper.createMessage("idgs.sample.tpch.pb.LineItem");
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
