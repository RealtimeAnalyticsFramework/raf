
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
#include "idgs/expr/expression_helper.h"

using namespace std;
using namespace idgs;
using namespace idgs::pb;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;

namespace idgs {
namespace tpc {
namespace ssb_Q1_1_test {

RddClient client;

ActorId createLineOrderFilterRdd(const string& rddId) {
  RddRequestPtr request = std::make_shared<CreateRddRequest>();
  RddResponsePtr response = std::make_shared<CreateRddResponse>();

  request->set_transformer_op_name(FILTER_TRANSFORMER);

  auto out = request->mutable_out_rdd();
  out->set_rdd_name("LINE_ORDER_FILTER");
  out->set_persist_type(idgs::rdd::pb::ORDERED);
  out->set_key_type_name("idgs.sample.ssb.pb.LineOrderKey");
  out->set_value_type_name("idgs.sample.ssb.pb.LineOrder");

  auto in = request->add_in_rdd();
  //in->mutable_rdd_id()->set_actor_id(rddId.actor_id());
  //in->mutable_rdd_id()->set_member_id(rddId.member_id());
  in->mutable_rdd_name()->append(rddId);

  shared_ptr<Expr> expression(AND(GE(FIELD("lo_discount"), CONST("1", DOUBLE)),
                                    LE(FIELD("lo_discount"), CONST("3", DOUBLE)),
                                    LT(FIELD("lo_quantity"), CONST("25", DOUBLE))
                                   ));

  in->mutable_filter_expr()->CopyFrom(* expression);

  auto rc = client.createRdd(request, response);
  EXPECT_EQ(RC_OK, rc);

  return response->rdd_id();
}

ActorId createDateFilterRdd(const string& rddId) {
  RddRequestPtr request = std::make_shared<CreateRddRequest>();
  RddResponsePtr response = std::make_shared<CreateRddResponse>();

  request->set_transformer_op_name(FILTER_TRANSFORMER);

  auto out = request->mutable_out_rdd();
  out->set_rdd_name("DATE_FILTER");
  out->set_persist_type(idgs::rdd::pb::ORDERED);
  out->set_key_type_name("idgs.sample.ssb.pb.DateKey");
  out->set_value_type_name("idgs.sample.ssb.pb.Date");

  auto in = request->add_in_rdd();
  //in->mutable_rdd_id()->set_actor_id(rddId.actor_id());
  //in->mutable_rdd_id()->set_member_id(rddId.member_id());
  in->mutable_rdd_name()->append(rddId);

  shared_ptr<Expr> expression(EQ(FIELD("d_year"), CONST("1992", UINT32)));

  in->mutable_filter_expr()->CopyFrom(* expression);
  auto rc = client.createRdd(request, response);
  EXPECT_EQ(RC_OK, rc);
  return response->rdd_id();
}

ActorId createLineOrderGroupRdd(const string& rddId) {
  RddRequestPtr request = std::make_shared<CreateRddRequest>();
  RddResponsePtr response = std::make_shared<CreateRddResponse>();

  request->set_transformer_op_name(GROUP_TRANSFORMER);

  InRddInfo* in = request->add_in_rdd();
  //in->mutable_rdd_id()->CopyFrom(rddId);
  in->mutable_rdd_name()->append(rddId);

  FieldNamePair* field = in->add_out_fields();
  auto expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("lo_orderdate");

  field = in->add_out_fields();
  expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("lo_discount");

  field = in->add_out_fields();
  expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("lo_extendedprice");

  auto out = request->mutable_out_rdd();
  out->set_rdd_name("LINE_ORDER_GROUP");
  out->set_persist_type(idgs::rdd::pb::ORDERED);
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
  auto rc = client.createRdd(request, response);
  EXPECT_EQ(RC_OK, rc);
  return response->rdd_id();
}

ActorId createJoinRdd(const string& lineOrderRDD, const string& dateRdd) {
  RddRequestPtr request = std::make_shared<CreateRddRequest>();
  RddResponsePtr response = std::make_shared<CreateRddResponse>();

  request->set_transformer_op_name(HASH_JOIN_TRANSFORMER);

  InRddInfo* in = request->add_in_rdd();
  //in->mutable_rdd_id()->CopyFrom(lineOrderRDD);
  in->mutable_rdd_name()->append(lineOrderRDD);

  FieldNamePair* field = in->add_out_fields();
  field->set_field_alias("orderdate");
  auto expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("lo_orderdate");

  field = in->add_out_fields();
  field->set_field_alias("discount");
  expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("lo_discount");

  field = in->add_out_fields();
  field->set_field_alias("price");
  expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("lo_extendedprice");

  in = request->add_in_rdd();
  //in->mutable_rdd_id()->CopyFrom(dateRdd);
  in->mutable_rdd_name()->append(dateRdd);

  auto out = request->mutable_out_rdd();
  out->set_rdd_name("LINE_ORDER_JOIN");
  out->set_persist_type(idgs::rdd::pb::ORDERED);
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

  shared_ptr<JoinRequest> joinParam = std::make_shared<JoinRequest>();
  joinParam->set_type(INNER_JOIN);

  AttachMessage params;
  params[TRANSFORMER_PARAM] = joinParam;

  auto rc = client.createRdd(request, response, params);
  EXPECT_EQ(RC_OK, rc);
  return response->rdd_id();
}

void ssbQ1_1Action(const ActorId& rddId) {
  shared_ptr<Expr> expression(MULTIPLY(FIELD("price"), FIELD("discount")));

  ActionRequestPtr request = std::make_shared<ActionRequest>();
  ActionResponsePtr response = std::make_shared<ActionResponse>();
  ActionResultPtr result = std::make_shared<SumActionResult>();

  request->set_action_id("100000");
  request->set_action_op_name(SUM_ACTION);
  request->mutable_expression()->CopyFrom(*expression);

  auto rc = client.sendAction(request, response, result, rddId);
  EXPECT_EQ(RC_OK, rc);

  LOG(INFO) << "*****************************";
  LOG(INFO) << "    Result is : " << std::fixed << dynamic_cast<SumActionResult*>(result.get())->total();
  LOG(INFO) << "*****************************";
}

}
}
}

TEST(hash_join, ssb_Q1_1) {
  ResultCode code = idgs::tpc::ssb_Q1_1_test::client.init("conf/client.conf");
  LOG(INFO) << code;

  LOG(INFO) << "Create store delegate RDD for lineorder.";
  DelegateRddRequestPtr request = std::make_shared<CreateDelegateRddRequest>();
  DelegateRddResponsePtr response = std::make_shared<CreateDelegateRddResponse>();
  request->set_schema_name("ssb");
  request->set_store_name("ssb_lineorder");
  request->set_rdd_name("ssb_lineorder");

  auto rc = idgs::tpc::ssb_Q1_1_test::client.createStoreDelegateRDD(request, response);
  ASSERT_EQ(RC_OK, rc);

  auto orderDelegateRDD = response->rdd_id();

  sleep(2);

  LOG(INFO) << "Create filter RDD for lineorder.";
  auto orderFilterRDD = idgs::tpc::ssb_Q1_1_test::createLineOrderFilterRdd("ssb_lineorder");

  sleep(2);

  LOG(INFO) << "Create group RDD for lineorder.";
  auto orderGroupRDD = idgs::tpc::ssb_Q1_1_test::createLineOrderGroupRdd("LINE_ORDER_FILTER");

  sleep(2);

  LOG(INFO) << "Create store delegate RDD for date.";
  request->set_schema_name("ssb");
  request->set_store_name("ssb_date");
  request->set_rdd_name("ssb_date");

  rc = idgs::tpc::ssb_Q1_1_test::client.createStoreDelegateRDD(request, response);
  assert(RC_OK == rc);

  auto dateDelegateRDD = response->rdd_id();

  sleep(2);

  LOG(INFO) << "Create filter RDD for date.";
  auto dateFilterRDD = idgs::tpc::ssb_Q1_1_test::createDateFilterRdd("ssb_date");

  sleep(2);

  LOG(INFO) << "Create inner join RDD for date.";
  auto joinRDD = idgs::tpc::ssb_Q1_1_test::createJoinRdd("LINE_ORDER_GROUP", "DATE_FILTER");

  sleep(2);

  LOG(INFO) << "Handle sum action.";
  idgs::tpc::ssb_Q1_1_test::ssbQ1_1Action(joinRDD);
}
