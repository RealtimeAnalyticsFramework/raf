
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
////#if defined(__GNUC__) || defined(__clang__)
////#include "idgs_gch.h"
////#endif // GNUC_ $
#include "gtest/gtest.h"

#include "idgs/client/rdd/rdd_client.h"

#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"
#include "idgs/rdd/pb/rdd_transform.pb.h"
#include "idgs/store/data_map.h"


using namespace std;
using namespace idgs;
using namespace idgs::pb;
using namespace idgs::util;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;

ActorId createLineOrderFilterRdd(const string& rddId) {
  RddRequestPtr request(new CreateRddRequest);
  RddResponsePtr response(new CreateRddResponse);

  request->set_transformer_op_name(FILTER_TRANSFORMER);

  auto out = request->mutable_out_rdd();
  out->set_rdd_name("LINE_ORDER_FILTER");
  out->set_data_type(idgs::rdd::pb::ORDERED);
  out->set_key_type_name("idgs.sample.ssb.pb.LineOrderKey");
  out->set_value_type_name("idgs.sample.ssb.pb.LineOrder");

  auto in = request->add_in_rdd();
  //in->mutable_rdd_id()->set_actor_id(rddId.actor_id());
  //in->mutable_rdd_id()->set_member_id(rddId.member_id());
  in->mutable_rdd_name()->append(rddId);

  shared_ptr<Expr> expression(new Expr);
  expression->set_type(AND);

  // >= 1994-01-01
  Expr* exp1 = expression->add_expression();
  exp1->set_type(GE);

  Expr* elem1 = exp1->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("lo_discount");

  Expr* elem2 = exp1->add_expression();
  elem2->set_type(CONST);
  elem2->set_const_type(DOUBLE);
  elem2->set_value("1");

  // discount <= 0.07
  auto exp4 = expression->add_expression();
  exp4->set_type(LE);
  elem1 = exp4->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("lo_discount");

  elem2 = exp4->add_expression();
  elem2->set_type(CONST);
  elem2->set_const_type(DOUBLE);
  elem2->set_value("3");

  // quantity < 25
  auto emp5 = expression->add_expression();
  emp5->set_type(LT);
  elem1 = emp5->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("lo_quantity");

  elem2 = emp5->add_expression();
  elem2->set_type(CONST);
  elem2->set_const_type(DOUBLE);
  elem2->set_value("25");

  in->mutable_filter_expr()->CopyFrom(* expression);

  singleton<RddClient>::getInstance().createRdd(request, response);

  return response->rdd_id();
}

ActorId createDateFilterRdd(const string& rddId) {
  RddRequestPtr request(new CreateRddRequest);
  RddResponsePtr response(new CreateRddResponse);

  request->set_transformer_op_name(FILTER_TRANSFORMER);

  auto out = request->mutable_out_rdd();
  out->set_rdd_name("DATE_FILTER");
  out->set_data_type(idgs::rdd::pb::ORDERED);
  out->set_key_type_name("idgs.sample.ssb.pb.DateKey");
  out->set_value_type_name("idgs.sample.ssb.pb.Date");

  auto in = request->add_in_rdd();
  //in->mutable_rdd_id()->set_actor_id(rddId.actor_id());
  //in->mutable_rdd_id()->set_member_id(rddId.member_id());
  in->mutable_rdd_name()->append(rddId);

  shared_ptr<Expr> expression(new Expr);
  expression->set_type(EQ);

  Expr* elem1 = expression->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("d_year");

  Expr* elem2 = expression->add_expression();
  elem2->set_type(CONST);
  elem2->set_const_type(UINT32);
  elem2->set_value("1992");

  in->mutable_filter_expr()->CopyFrom(* expression);
  singleton<RddClient>::getInstance().createRdd(request, response);

  return response->rdd_id();
}

ActorId createLineOrderGroupRdd(const string& rddId) {
  RddRequestPtr request(new CreateRddRequest);
  RddResponsePtr response(new CreateRddResponse);

  request->set_transformer_op_name(GROUP_TRANSFORMER);

  InRddInfo* in = request->add_in_rdd();
  //in->mutable_rdd_id()->CopyFrom(rddId);
  in->mutable_rdd_name()->append(rddId);

  FieldNamePair* field = in->add_out_fields();
  auto expr = field->mutable_expr();
  expr->set_type(FIELD);
  expr->set_value("lo_orderdate");

  field = in->add_out_fields();
  expr = field->mutable_expr();
  expr->set_type(FIELD);
  expr->set_value("lo_discount");

  field = in->add_out_fields();
  expr = field->mutable_expr();
  expr->set_type(FIELD);
  expr->set_value("lo_extendedprice");

  auto out = request->mutable_out_rdd();
  out->set_rdd_name("LINE_ORDER_GROUP");
  out->set_data_type(idgs::rdd::pb::ORDERED);
  out->set_key_type_name("idgs.sample.ssb.pb.GLineOrderKey");
  out->set_value_type_name("idgs.sample.ssb.pb.GLineOrder");

  auto fld = out->add_key_fields();
  fld->set_field_name("lo_orderdate");
  fld->set_field_type(UINT64);

  fld = out->add_value_fields();
  fld->set_field_name("lo_discount");
  fld->set_field_type(DOUBLE);

  fld = out->add_value_fields();
  fld->set_field_name("lo_extendedprice");
  fld->set_field_type(DOUBLE);

  in->mutable_filter_expr()->CopyFrom(* expr);
  singleton<RddClient>::getInstance().createRdd(request, response);

  return response->rdd_id();
}

ActorId createJoinRdd(const string& lineOrderRDD, const string& dateRdd) {
  RddRequestPtr request(new CreateRddRequest);
  RddResponsePtr response(new CreateRddResponse);

  request->set_transformer_op_name(HASH_JOIN_TRANSFORMER);

  InRddInfo* in = request->add_in_rdd();
  //in->mutable_rdd_id()->CopyFrom(lineOrderRDD);
  in->mutable_rdd_name()->append(lineOrderRDD);

  FieldNamePair* field = in->add_out_fields();
  field->set_field_alias("orderdate");
  auto expr = field->mutable_expr();
  expr->set_type(FIELD);
  expr->set_value("lo_orderdate");

  field = in->add_out_fields();
  field->set_field_alias("discount");
  expr = field->mutable_expr();
  expr->set_type(FIELD);
  expr->set_value("lo_discount");

  field = in->add_out_fields();
  field->set_field_alias("price");
  expr = field->mutable_expr();
  expr->set_type(FIELD);
  expr->set_value("lo_extendedprice");

  in = request->add_in_rdd();
  //in->mutable_rdd_id()->CopyFrom(dateRdd);
  in->mutable_rdd_name()->append(dateRdd);

  auto out = request->mutable_out_rdd();
  out->set_rdd_name("LINE_ORDER_JOIN");
  out->set_data_type(idgs::rdd::pb::ORDERED);
  out->set_key_type_name("idgs.sample.ssb.pb.JoinKey");
  out->set_value_type_name("idgs.sample.ssb.pb.Join");

  auto fld = out->add_key_fields();
  fld->set_field_name("orderdate");
  fld->set_field_type(UINT64);

  fld = out->add_value_fields();
  fld->set_field_name("discount");
  fld->set_field_type(DOUBLE);

  fld = out->add_value_fields();
  fld->set_field_name("price");
  fld->set_field_type(DOUBLE);

  shared_ptr<JoinRequest> joinParam(new JoinRequest);
  joinParam->set_type(INNER_JOIN);

  auto joinfld = joinParam->add_join_field();
  joinfld->set_l_join_field("lo_orderdate");
  joinfld->set_r_join_field("d_datekey");

  AttachMessage params;
  params[JOIN_PARAM] = joinParam;

  singleton<RddClient>::getInstance().createRdd(request, response, params);

  return response->rdd_id();
}

void ssbQ1_1Action(const ActorId& rddId) {
  shared_ptr<Expr> expression(new Expr);
  expression->set_type(MULTIPLY);
  auto elem1 = expression->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("price");

  auto elem2 = expression->add_expression();
  elem2->set_type(FIELD);
  elem2->set_value("discount");

  ActionRequestPtr request(new ActionRequest);
  ActionResponsePtr response(new ActionResponse);
  ActionResultPtr result(new SumActionResult);

  request->set_action_id("100000");
  request->set_action_op_name(SUM_ACTION);
  request->mutable_expression()->CopyFrom(*expression);

  singleton<RddClient>::getInstance().sendAction(request, response, result, rddId);

  LOG(INFO) << "*****************************";
  LOG(INFO) << "    Result is : " << std::fixed << dynamic_cast<SumActionResult*>(result.get())->total();
  LOG(INFO) << "*****************************";
}

TEST(hash_join, ssb_Q1_1) {
  ResultCode code = singleton<RddClient>::getInstance().init("samples/tpc-svc/conf/client.conf");

  LOG(INFO) << "Create store delegate RDD for lineorder.";
  DelegateRddRequestPtr request(new CreateDelegateRddRequest);
  DelegateRddResponsePtr response(new CreateDelegateRddResponse);
  request->set_store_name("ssb_lineorder");
  request->set_rdd_name("ssb_lineorder");

  singleton<RddClient>::getInstance().createStoreDelegateRDD(request, response);
  auto orderDelegateRDD = response->rdd_id();

  sleep(2);

  LOG(INFO) << "Create filter RDD for lineorder.";
  auto orderFilterRDD = createLineOrderFilterRdd("ssb_lineorder");

  sleep(2);

  LOG(INFO) << "Create group RDD for lineorder.";
  auto orderGroupRDD = createLineOrderGroupRdd("LINE_ORDER_FILTER");

  sleep(2);

  LOG(INFO) << "Create store delegate RDD for date.";
  request->set_store_name("ssb_date");
  request->set_rdd_name("ssb_date");

  singleton<RddClient>::getInstance().createStoreDelegateRDD(request, response);
  auto dateDelegateRDD = response->rdd_id();

  sleep(2);

  LOG(INFO) << "Create filter RDD for date.";
  auto dateFilterRDD = createDateFilterRdd("ssb_date");

  sleep(2);

  LOG(INFO) << "Create inner join RDD for date.";
  auto joinRDD = createJoinRdd("LINE_ORDER_GROUP", "DATE_FILTER");

  sleep(2);

  LOG(INFO) << "Handle sum action.";
  ssbQ1_1Action(joinRDD);
}
