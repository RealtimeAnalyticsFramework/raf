
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include <gtest/gtest.h>
#include "idgs/cancelable_timer.h"
#include "idgs/client/rdd/rdd_client.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_transform.pb.h"
#include "idgs/rdd/pb/rdd_action.pb.h"
#include "idgs/store/data_map.h"

using namespace std;
using namespace idgs::pb;
using namespace idgs::util;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;

using namespace idgs::client::rdd;

namespace {

idgs::pb::ActorId createStoreDelegateRDD(const std::string& storeName) {
  DelegateRddRequestPtr request(new CreateDelegateRddRequest);
  DelegateRddResponsePtr response(new CreateDelegateRddResponse);

  request->set_store_name(storeName);

  singleton<RddClient>::getInstance().createStoreDelegateRDD(request, response);

  if (response->result_code() != RRC_SUCCESS) {
    LOG(ERROR) << "Create store delegate RDD with store " << storeName << " error, caused by code " << response->result_code();
    exit(1);
  }

  return response->rdd_id();
}

std::shared_ptr<Expr> buildTpchQ6FilterExpression() {
  shared_ptr<Expr> expression(new Expr);
  expression->set_type(AND);

  // >= 1994-01-01
  Expr* exp1 = expression->add_expression();
  exp1->set_type(GE);

  Expr* elem1 = exp1->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("l_shipdate");

  Expr* elem2 = exp1->add_expression();
  elem2->set_type(CONST);
  elem2->set_const_type(STRING);
  elem2->set_value("1994-01-01");

  // < 1995-01-01
  Expr* exp2 = expression->add_expression();
  exp2->set_type(LT);

  elem1 = exp2->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("l_shipdate");

  elem2 = exp2->add_expression();
  elem2->set_type(CONST);
  elem2->set_const_type(STRING);
  elem2->set_value("1995-01-01");

  // discount >= 0.05
  Expr* exp3 = expression->add_expression();
  exp3->set_type(GE);

  elem1 = exp3->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("l_discount");

  elem2 = exp3->add_expression();
  elem2->set_type(CONST);
  elem2->set_const_type(DOUBLE);
  elem2->set_value("0.05");

  // discount <= 0.07
  auto exp4 = expression->add_expression();
  exp4->set_type(LE);
  elem1 = exp4->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("l_discount");

  elem2 = exp4->add_expression();
  elem2->set_type(CONST);
  elem2->set_const_type(DOUBLE);
  elem2->set_value("0.07");

  // quantity < 24
  auto emp5 = expression->add_expression();
  emp5->set_type(LT);
  elem1 = emp5->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("l_quantity");

  elem2 = emp5->add_expression();
  elem2->set_type(CONST);
  elem2->set_const_type(DOUBLE);
  elem2->set_value("24");

  return expression;
}

idgs::pb::ActorId createFilter(const string& inRddName) {
  LOG(INFO) << "============================ Create Filter RDD ============================";
  RddRequestPtr request(new CreateRddRequest);
  RddResponsePtr response(new CreateRddResponse);

  request->set_transformer_op_name(FILTER_TRANSFORMER);

  auto in = request->add_in_rdd();
  //in->mutable_rdd_id()->CopyFrom(input_rdd_actor_id);
  in->mutable_rdd_name()->append(inRddName);

  auto out = request->mutable_out_rdd();
  out->set_rdd_name("TPCH_Q6_FILTER_RDD");
  out->set_data_type(ORDERED);
  out->set_key_type_name("idgs.sample.tpch.pb.LineItemKey");
  out->set_value_type_name("idgs.sample.tpch.pb.LineItem");

  auto exp = buildTpchQ6FilterExpression();
  in->mutable_filter_expr()->CopyFrom(* exp);

  singleton<RddClient>::getInstance().createRdd(request, response);

  return response->rdd_id();
}

idgs::pb::ActorId createGroupRDD(const string& inRddName) {
  LOG(INFO) << "============================ Create Group RDD ============================";
  RddRequestPtr request(new CreateRddRequest);
  RddResponsePtr response(new CreateRddResponse);

  request->set_transformer_op_name(GROUP_TRANSFORMER);

  InRddInfo* in = request->add_in_rdd();
  //in->mutable_rdd_id()->CopyFrom(input_rdd_actor_id);
  in->mutable_rdd_name()->append(inRddName);

  FieldNamePair* field = in->add_out_fields();
  field->set_field_alias("order_key");
  auto expr = field->mutable_expr();
  expr->set_type(FIELD);
  expr->set_value("l_orderkey");

  field = in->add_out_fields();
  field->set_field_alias("discount");
  expr = field->mutable_expr();
  expr->set_type(FIELD);
  expr->set_value("l_discount");

  field = in->add_out_fields();
  field->set_field_alias("price");
  expr = field->mutable_expr();
  expr->set_type(FIELD);
  expr->set_value("l_extendedprice");

  auto out = request->mutable_out_rdd();
  out->set_rdd_name("group_tpch_q6");
  out->set_data_type(ORDERED);
  out->set_key_type_name("idgs.sample.tpch.pb.GLineOrderKey");
  out->set_value_type_name("idgs.sample.tpch.pb.GLineOrder");

  auto fld = out->add_key_fields();
  fld->set_field_name("order_key");
  fld->set_field_type(UINT64);

  fld = out->add_value_fields();
  fld->set_field_name("discount");
  fld->set_field_type(DOUBLE);

  fld = out->add_value_fields();
  fld->set_field_name("price");
  fld->set_field_type(DOUBLE);

  singleton<RddClient>::getInstance().createRdd(request, response);

  return response->rdd_id();
}

idgs::pb::ActorId createReduceValueTpchQ6RDD(const string& inRddName) {
  LOG(INFO) << "============================ Create Reduce Value RDD ============================";
  RddRequestPtr request(new CreateRddRequest);
  RddResponsePtr response(new CreateRddResponse);

  request->set_transformer_op_name(REDUCEVALUE_TRANSFORMER);

  InRddInfo* in = request->add_in_rdd();
  //in->mutable_rdd_id()->CopyFrom(input_rdd_actor_id);
  in->mutable_rdd_name()->append(inRddName);

  FieldNamePair* field = in->add_out_fields();
  field->set_field_alias("sum_result");
  auto expr = field->mutable_expr();
  expr->set_type(MULTIPLY);

  Expr* elem1 = expr->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("price");

  Expr* elem2 = expr->add_expression();
  elem2->set_type(FIELD);
  elem2->set_value("discount");

  auto out = request->mutable_out_rdd();
  out->set_rdd_name("reducevalue_tpch_q6");
  out->set_data_type(ORDERED);
  out->set_key_type_name("idgs.sample.tpch.pb.GLineOrderKey");
  out->set_value_type_name("idgs.sample.tpch.pb.SLineOrder");

  auto fld = out->add_value_fields();
  fld->set_field_name("sum_result");
  fld->set_field_type(DOUBLE);

  std::shared_ptr<ReduceByKeyRequest> reduceRequest(new ReduceByKeyRequest);
  auto reduce_field = reduceRequest->add_fields();
  reduce_field->set_distinct(false);
  reduce_field->set_field_alias("sum_result");
  reduce_field->set_type(ReduceType::SUM);

  map<string, shared_ptr<google::protobuf::Message>> params;
  params[TRANSFORMER_PARAM] = reduceRequest;

  singleton<RddClient>::getInstance().createRdd(request, response, params);

  return response->rdd_id();
}

void sum_action(const idgs::pb::ActorId& input_rdd_actor_id, const shared_ptr<Expr>& expr) {
  LOG(INFO) << "============================ Take Sum Action ============================";

  ActionRequestPtr request(new ActionRequest);
  ActionResponsePtr response(new ActionResponse);
  ActionResultPtr result(new SumActionResult);

  request->set_action_id("sum001");
  request->set_action_op_name(idgs::rdd::SUM_ACTION);
  request->mutable_expression()->CopyFrom(*expr);

  singleton<RddClient>::getInstance().sendAction(request, response, result, input_rdd_actor_id);

  auto total = dynamic_cast<SumActionResult*>(result.get())->total();
  EXPECT_DOUBLE_EQ(77949.918600, total);
  LOG(INFO) << "Sum Action Result is : " << std::fixed << total;
}

size_t countAction(const std::string& input_rdd_name, const std::string& action_id, int time_out = 10) {
  std::shared_ptr<ActionRequest> request(new ActionRequest);
  request->set_action_id(action_id);
  request->set_rdd_name(input_rdd_name);
  request->set_action_op_name(idgs::rdd::COUNT_ACTION);
  ActionResponsePtr response(new ActionResponse);
  ActionResultPtr result(new CountActionResult);
  RddClient rdd_client;
  rdd_client.sendAction(request, response, result, AttachMessage(), time_out);
  auto rs = (dynamic_cast<CountActionResult*>(result.get()))->size();
  return rs;
}
}


TEST(reducevalue_transformer_test, reducevalue) {
  TEST_TIMEOUT(30);

  singleton<RddClient>::getInstance().init("integration_test/rdd_it/client.conf");

  DelegateRddRequestPtr request(new CreateDelegateRddRequest);
  DelegateRddResponsePtr response(new CreateDelegateRddResponse);
  request->set_store_name("LineItem");
  request->set_rdd_name("LineItem");

  singleton<RddClient>::getInstance().createStoreDelegateRDD(request, response);
  auto rdd_actor_id = response->rdd_id();
  sleep(2);

  /// filter begin
  rdd_actor_id = createFilter("LineItem");
  sleep(2);
  shared_ptr<Expr> expr(new Expr);
  expr->set_type(MULTIPLY);
  auto elem1 = expr->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("l_extendedprice");
  auto elem2 = expr->add_expression();
  elem2->set_type(FIELD);
  elem2->set_value("l_discount");
  sum_action(rdd_actor_id, expr);

  LOG(INFO) << "after filter, count action result is : " << countAction("TPCH_Q6_FILTER_RDD", "filter_count_action");

  /// gourp
  rdd_actor_id = createGroupRDD("TPCH_Q6_FILTER_RDD");
  sleep(2);

  expr = shared_ptr<Expr>(new Expr);
  expr->set_type(MULTIPLY);
  elem1 = expr->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("price");
  elem2 = expr->add_expression();
  elem2->set_type(FIELD);
  elem2->set_value("discount");
  LOG(INFO) << "after group, count action result is : " << countAction("group_tpch_q6", "group_count_action");
  sum_action(rdd_actor_id, expr);

  /// reducevalue
  rdd_actor_id = createReduceValueTpchQ6RDD("group_tpch_q6");
  sleep(2);

  expr = shared_ptr<Expr>(new Expr);
  expr->set_type(FIELD);
  expr->set_value("sum_result");

  sum_action(rdd_actor_id, expr);
}
