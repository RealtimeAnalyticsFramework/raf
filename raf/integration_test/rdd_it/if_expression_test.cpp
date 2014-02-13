
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "gtest/gtest.h"
#include "idgs/client/rdd/rdd_client.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"
#include "protobuf/message_helper.h"
#include "idgs/store/data_map.h"

using namespace std;
using namespace idgs;
using namespace idgs::pb;
using namespace idgs::util;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;
using namespace protobuf;

ActionResultPtr ifExprTest() {
  /// create store delegate
  ResultCode code = singleton<RddClient>::getInstance().init("integration_test/rdd_it/client.conf");
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "init rdd client error, caused by " << getErrorDescription(code);
  }

  const std::string store_name = "LineItem", delegateRddName = "LineItemRdd", filterRddName = "LineItemFilterRdd";
  LOG(INFO)<< "create store delegate, store_name: " << store_name;

  DelegateRddRequestPtr delegateReq(new CreateDelegateRddRequest);
  DelegateRddResponsePtr delegateResp(new CreateDelegateRddResponse);
  delegateReq->set_store_name(store_name);
  delegateReq->set_rdd_name(delegateRddName);

  singleton<RddClient>::getInstance().createStoreDelegateRDD(delegateReq, delegateResp);

  RddRequestPtr filterReq(new CreateRddRequest);
  RddResponsePtr filterResp(new CreateRddResponse);

  filterReq->set_transformer_op_name(FILTER_TRANSFORMER);
  auto out = filterReq->mutable_out_rdd();
  out->set_rdd_name(filterRddName);
  out->set_data_type(ORDERED);
  out->set_key_type_name("idgs.sample.tpch.pb.LineItemKey");
  out->set_value_type_name("idgs.rdd.test.pb.BranchLineItem");

  auto outFld = out->add_value_fields();
  outFld->set_field_name("l_quantity");
  outFld->set_field_type(DOUBLE);

  outFld = out->add_value_fields();
  outFld->set_field_name("quantity_desc");
  outFld->set_field_type(STRING);

  auto in = filterReq->add_in_rdd();
  in->set_rdd_name(delegateRddName);

  auto fld = in->add_out_fields();
  auto fldExpr = fld->mutable_expr();
  fldExpr->set_type(FIELD);
  fldExpr->set_value("l_quantity");

  // add IF expression
  fld = in->add_out_fields();
  fld->set_field_alias("quantity_desc");
  fldExpr = fld->mutable_expr();
  fldExpr->set_type(IF);

  // condition 1 : l_quantity > 24
  auto condExpr = fldExpr->add_expression();
  condExpr->set_type(GT);

  auto expr = condExpr->add_expression();
  expr->set_type(FIELD);
  expr->set_value("l_quantity");

  expr = condExpr->add_expression();
  expr->set_type(CONST);
  expr->set_const_type(DOUBLE);
  expr->set_value("24");

  // value 1 : quantity > 24
  auto valueExpr = fldExpr->add_expression();
  valueExpr->set_type(CONST);
  valueExpr->set_const_type(STRING);
  valueExpr->set_value("quantity > 24");

  // condition 2 : 10 < quantity <= 24
  condExpr = fldExpr->add_expression();
  condExpr->set_type(AND);

  // expression for 10 < quantity
  auto cmpExpr = condExpr->add_expression();
  cmpExpr->set_type(GT);

  expr = cmpExpr->add_expression();
  expr->set_type(FIELD);
  expr->set_value("l_quantity");

  expr = cmpExpr->add_expression();
  expr->set_type(CONST);
  expr->set_const_type(DOUBLE);
  expr->set_value("10");

  // expression for quantity <= 24
  cmpExpr = condExpr->add_expression();
  cmpExpr->set_type(LE);

  expr = cmpExpr->add_expression();
  expr->set_type(FIELD);
  expr->set_value("l_quantity");

  expr = cmpExpr->add_expression();
  expr->set_type(CONST);
  expr->set_const_type(DOUBLE);
  expr->set_value("24");

  // value 2 : 10 < quantity <= 24
  valueExpr = fldExpr->add_expression();
  valueExpr->set_type(CONST);
  valueExpr->set_const_type(STRING);
  valueExpr->set_value("10 < quantity <= 24");

  // default value : quantity <= 10
  valueExpr = fldExpr->add_expression();
  valueExpr->set_type(CONST);
  valueExpr->set_const_type(STRING);
  valueExpr->set_value("quantity <= 10");

  singleton<RddClient>::getInstance().createRdd(filterReq, filterResp);

  ActionRequestPtr topNActionReq(new ActionRequest);
  ActionResponsePtr topNActionResp(new ActionResponse);
  ActionResultPtr topNActionResult(new TopNActionResult);

  topNActionReq->set_action_id("if_expression_test");
  topNActionReq->set_action_op_name(TOP_N_ACTION);
  topNActionReq->set_rdd_name(filterRddName);

  shared_ptr<TopNActionRequest> topNActionParam(new TopNActionRequest);
  topNActionParam->set_top_n(10);

  auto orderFld = topNActionParam->add_order_field();
  orderFld->set_desc(false);
  auto orderExpr = orderFld->mutable_expr();
  orderExpr->set_type(FIELD);
  orderExpr->set_value("l_orderkey");

  orderFld = topNActionParam->add_order_field();
  orderFld->set_desc(false);
  orderExpr = orderFld->mutable_expr();
  orderExpr->set_type(FIELD);
  orderExpr->set_value("l_linenumber");

  AttachMessage param;
  param[ACTION_PARAM] = topNActionParam;

  singleton<RddClient>::getInstance().sendAction(topNActionReq, topNActionResp, topNActionResult, param);

  return topNActionResult;
}

TEST(count_action_test, count) {
  LOG(INFO) << "select l_orderkey,";
  LOG(INFO) << "       l_linenumber,";
  LOG(INFO) << "       l_quantity, ";
  LOG(INFO) << "       case when l_quantity > 24 then 'quantity > 24'";
  LOG(INFO) << "            when 10 < l_quantity <= 24 then '10 < quantity <= 24'";
  LOG(INFO) << "            else 'quantity <= 10'";
  LOG(INFO) << "        end quantity_desc";
  LOG(INFO) << "  from lineitem";

  ActionResultPtr topNActionResult = ifExprTest();

  TopNActionResult* result = dynamic_cast<TopNActionResult*>(topNActionResult.get());

  auto& helper = singleton<MessageHelper>::getInstance();
  helper.registerDynamicMessage("integration_test/rdd_it/rdd_it.proto");

  for (int32_t i = 0; i < result->pair_size(); ++ i) {
    string key_str = result->pair(i).key();
    string value_str = result->pair(i).value();
    auto key = helper.createMessage("idgs.sample.tpch.pb.LineItemKey");
    ProtoSerdes<DEFAULT_PB_SERDES>::deserialize(key_str, key.get());
    auto value = helper.createMessage("idgs.rdd.test.pb.BranchLineItem");
    ProtoSerdes<DEFAULT_PB_SERDES>::deserialize(value_str, value.get());

    auto order_key = key->GetReflection()->GetInt64(* key, key->GetDescriptor()->FindFieldByName("l_orderkey"));
    auto line_number = key->GetReflection()->GetInt64(* key, key->GetDescriptor()->FindFieldByName("l_linenumber"));
    auto quantity = value->GetReflection()->GetDouble(* value, value->GetDescriptor()->FindFieldByName("l_quantity"));
    auto quantity_desc = value->GetReflection()->GetString(* value, value->GetDescriptor()->FindFieldByName("quantity_desc"));

    if (quantity > 24) {
      EXPECT_EQ("quantity > 24", quantity_desc);
    } else if (quantity > 10 && quantity <=24) {
      EXPECT_EQ("10 < quantity <= 24", quantity_desc);
    } else {
      EXPECT_EQ("quantity <= 10", quantity_desc);
    }

    string quantity_str;
    if (quantity < 10) {
      quantity_str = " "+ to_string((int32_t)quantity);
    } else {
      quantity_str = to_string((int32_t)quantity);
    }

    LOG(INFO) << order_key << " | " << line_number << " | " << quantity_str << " | " << quantity_desc;
  }
}

