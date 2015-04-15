
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
#include "idgs/expr/expression_helper.h"

using namespace std;
using namespace idgs::pb;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;
using namespace protobuf;

namespace idgs {
namespace rdd {
namespace if_expr_test {

RddClient client;

ActionResultPtr ifExprTest() {
  /// create store delegate
  auto code = client.init("conf/client.conf");
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "init rdd client error, caused by " << getErrorDescription(code);
  }

  const std::string store_name = "LineItem", delegateRddName = "LineItemRdd", filterRddName = "LineItemFilterRdd";
  LOG(INFO)<< "create store delegate, store_name: " << store_name;

  DelegateRddRequestPtr delegateReq = std::make_shared<CreateDelegateRddRequest>();
  DelegateRddResponsePtr delegateResp = std::make_shared<CreateDelegateRddResponse>();
  delegateReq->set_schema_name("tpch");
  delegateReq->set_store_name(store_name);
  delegateReq->set_rdd_name(delegateRddName);

  client.createStoreDelegateRDD(delegateReq, delegateResp);

  RddRequestPtr filterReq = std::make_shared<CreateRddRequest>();
  RddResponsePtr filterResp = std::make_shared<CreateRddResponse>();

  filterReq->set_transformer_op_name(FILTER_TRANSFORMER);
  auto out = filterReq->mutable_out_rdd();
  out->set_rdd_name(filterRddName);
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
  fldExpr->set_name("FIELD");
  fldExpr->set_value("l_quantity");

  // add IF expression
  fld = in->add_out_fields();
  fld->set_field_alias("quantity_desc");
  MOVE_EXPR(fld->mutable_expr(), IF(GT(FIELD("l_quantity"), CONST("24", DOUBLE)),
                                     CONST("quantity > 24"),
                                     AND(GT(FIELD("l_quantity"), CONST("10", DOUBLE)),
                                           LE(FIELD("l_quantity"), CONST("24", DOUBLE))),
                                     CONST("10 < quantity <= 24"),
                                     CONST("quantity <= 10")
                                    ));

  client.createRdd(filterReq, filterResp);

  ActionRequestPtr topNActionReq = std::make_shared<ActionRequest>();
  ActionResponsePtr topNActionResp = std::make_shared<ActionResponse>();
  ActionResultPtr topNActionResult = std::make_shared<TopNActionResult>();

  topNActionReq->set_action_id("if_expression_test");
  topNActionReq->set_action_op_name(TOP_N_ACTION);
  topNActionReq->set_rdd_name(filterRddName);

  shared_ptr<TopNActionRequest> topNActionParam = std::make_shared<TopNActionRequest>();
  topNActionParam->set_top_n(10);

  auto orderFld = topNActionParam->add_order_field();
  orderFld->set_desc(false);
  auto orderExpr = orderFld->mutable_expr();
  orderExpr->set_name("FIELD");
  orderExpr->set_value("l_orderkey");

  orderFld = topNActionParam->add_order_field();
  orderFld->set_desc(false);
  orderExpr = orderFld->mutable_expr();
  orderExpr->set_name("FIELD");
  orderExpr->set_value("l_linenumber");

  AttachMessage param;
  param[ACTION_PARAM] = topNActionParam;

  client.sendAction(topNActionReq, topNActionResp, topNActionResult, param);

  return topNActionResult;
}

}  // namespace if_expr_test
}  // namespace rdd
}  // namespace idgs

TEST(count_action_test, count) {
  LOG(INFO) << "select l_orderkey,";
  LOG(INFO) << "       l_linenumber,";
  LOG(INFO) << "       l_quantity, ";
  LOG(INFO) << "       case when l_quantity > 24 then 'quantity > 24'";
  LOG(INFO) << "            when 10 < l_quantity <= 24 then '10 < quantity <= 24'";
  LOG(INFO) << "            else 'quantity <= 10'";
  LOG(INFO) << "        end quantity_desc";
  LOG(INFO) << "  from lineitem";

  ActionResultPtr topNActionResult = idgs::rdd::if_expr_test::ifExprTest();

  TopNActionResult* result = dynamic_cast<TopNActionResult*>(topNActionResult.get());

  MessageHelper helper;
  helper.registerDynamicMessage("integration_test/rdd_it/rdd_it.proto");

  auto& storeConfigWrapper = idgs::rdd::if_expr_test::client.getStoreConfigWrapper("LineItem");

  for (int32_t i = 0; i < result->pair_size(); ++ i) {
    string key_str = result->pair(i).key();
    string value_str = result->pair(i).value();
    auto key = storeConfigWrapper->newKey();
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

