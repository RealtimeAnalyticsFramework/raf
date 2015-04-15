
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
#include "idgs/rdd/pb/rdd_transform.pb.h"
#include "idgs/rdd/pb/rdd_action.pb.h"

using namespace std;
using namespace idgs;
using namespace idgs::pb;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;

namespace idgs {
namespace rdd {
namespace hash_join_test {

RddClient client;

ResultCode init(const std::string& cfgFile) {
  return client.init(cfgFile);
}

idgs::pb::ActorId createStoreDelegateRDD(const std::string& schemaName, const std::string& storeName, const std::string& rddName) {
  DelegateRddRequestPtr request = std::make_shared<CreateDelegateRddRequest>();
  DelegateRddResponsePtr response = std::make_shared<CreateDelegateRddResponse>();

  request->set_store_name(storeName);
  request->set_rdd_name(rddName);

  client.createStoreDelegateRDD(request, response);

  return response->rdd_id();
}

ActorId createLineItemGroupRdd(const std::string& rddName, const string& inRddName) {
  RddRequestPtr request = std::make_shared<CreateRddRequest>();
  RddResponsePtr response = std::make_shared<CreateRddResponse>();

  request->set_transformer_op_name(GROUP_TRANSFORMER);

  InRddInfo* in = request->add_in_rdd();
  //in->mutable_rdd_id()->CopyFrom(rddId);
  in->mutable_rdd_name()->append(inRddName);

  FieldNamePair* field = in->add_out_fields();
  auto expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("l_orderkey");

  field = in->add_out_fields();
  expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("l_extendedprice");

  field = in->add_out_fields();
  expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("l_discount");

  auto out = request->mutable_out_rdd();
  out->set_rdd_name(rddName);
  out->set_key_type_name("idgs.sample.ssb.pb.GLineItemKey");
  out->set_value_type_name("idgs.sample.ssb.pb.GLineItem");

  auto fld = out->add_key_fields();
  fld->set_field_name("l_orderkey");
  fld->set_field_type(INT64);

  fld = out->add_value_fields();
  fld->set_field_name("l_discount");
  fld->set_field_type(DOUBLE);

  fld = out->add_value_fields();
  fld->set_field_name("l_extendedprice");
  fld->set_field_type(DOUBLE);

  client.createRdd(request, response);
  return response->rdd_id();
}

ActorId createJoinRdd(const string& orderRdd, const string& lineItemRDD, const string& rddName, const JoinType& type) {
  RddRequestPtr request = std::make_shared<CreateRddRequest>();
  RddResponsePtr response = std::make_shared<CreateRddResponse>();

  request->set_transformer_op_name(HASH_JOIN_TRANSFORMER);

  InRddInfo* in = request->add_in_rdd();
  //in->mutable_rdd_id()->CopyFrom(orderRdd);
  in->mutable_rdd_name()->append(orderRdd);

  FieldNamePair* field = in->add_out_fields();
  field->set_field_alias("orderkey");
  auto expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("o_orderkey");

  field = in->add_out_fields();
  field->set_field_alias("total");
  expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("o_totalprice");

  in = request->add_in_rdd();
  //in->mutable_rdd_id()->CopyFrom(lineItemRDD);
  in->mutable_rdd_name()->append(lineItemRDD);

  field = in->add_out_fields();
  field->set_field_alias("orderkey");
  expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("l_orderkey");

  field = in->add_out_fields();
  field->set_field_alias("discount");
  expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("l_discount");

  field = in->add_out_fields();
  field->set_field_alias("price");
  expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("l_extendedprice");

  auto out = request->mutable_out_rdd();
  out->set_rdd_name(rddName);
  out->set_key_type_name("idgs.rdd.pb.JoinKey");
  out->set_value_type_name("idgs.rdd.pb.Join");

  auto fld = out->add_key_fields();
  fld->set_field_name("orderkey");
  fld->set_field_type(INT64);

  fld = out->add_value_fields();
  fld->set_field_name("total");
  fld->set_field_type(DOUBLE);

  fld = out->add_value_fields();
  fld->set_field_name("discount");
  fld->set_field_type(DOUBLE);

  fld = out->add_value_fields();
  fld->set_field_name("price");
  fld->set_field_type(DOUBLE);

  shared_ptr<JoinRequest> param = std::make_shared<JoinRequest>();
  param->set_type(type);

  map<string, shared_ptr<google::protobuf::Message>> params;
  params[TRANSFORMER_PARAM] = param;
  client.createRdd(request, response, params);
  return response->rdd_id();
}

uint32_t countAction(const ActorId& rddId) {
  ActionRequestPtr request = std::make_shared<ActionRequest>();
  ActionResponsePtr response = std::make_shared<ActionResponse>();
  ActionResultPtr result = std::make_shared<CountActionResult>();

  request->set_action_id("100000");
  request->set_action_op_name(COUNT_ACTION);

  client.sendAction(request, response, result, rddId);

  auto size = dynamic_cast<CountActionResult*>(result.get())->size();
  return size;
}

} // namespace hash_join_transformer_test
} // namespace rdd
} // namespace idgs

TEST(hash_join, join) {
  ResultCode code = idgs::rdd::hash_join_test::init("conf/client.conf");
  if (code != RC_SUCCESS) {
    exit(1);
  }

  auto orderRDD = idgs::rdd::hash_join_test::createStoreDelegateRDD("tpch", "Orders", "ORDERS");
  LOG(INFO) << "create delegate RDD ORDERS(" << orderRDD.member_id() << ", " << orderRDD.actor_id() << ")";

  auto lineitemRDD = idgs::rdd::hash_join_test::createStoreDelegateRDD("tpch", "LineItem", "LINEITEM");
  LOG(INFO) << "create delegate RDD LINEITEM(" << lineitemRDD.member_id() << ", " << lineitemRDD.actor_id() << ")";
  sleep(2);

  auto lineitemGroupRDD = idgs::rdd::hash_join_test::createLineItemGroupRdd("LINE_ITEM_GROUP", "LINEITEM");
  LOG(INFO) << "create RDD LINE_ITEM_GROUP(" << lineitemGroupRDD.member_id() << ", " << lineitemGroupRDD.actor_id() << ")";
  sleep(2);

  auto innerJoinRDD = idgs::rdd::hash_join_test::createJoinRdd("ORDERS", "LINE_ITEM_GROUP", "ORDERS_ITEM_INNER_JOIN", INNER_JOIN);
  LOG(INFO) << "create RDD ORDERS_ITEM_INNER_JOIN(" << innerJoinRDD.member_id() << ", " << innerJoinRDD.actor_id() << ")";
  sleep(2);

  auto leftJoinRDD = idgs::rdd::hash_join_test::createJoinRdd("ORDERS", "LINE_ITEM_GROUP", "ORDERS_ITEM_LEFT_JOIN", LEFT_JOIN);
  LOG(INFO) << "create RDD ORDERS_ITEM_LEFT_JOIN(" << leftJoinRDD.member_id() << ", " << leftJoinRDD.actor_id() << ")";
  sleep(2);

  auto outerJoinRDD = idgs::rdd::hash_join_test::createJoinRdd("ORDERS", "LINE_ITEM_GROUP", "ORDERS_ITEM_OUTER_JOIN", OUTER_JOIN);
  LOG(INFO) << "create RDD ORDERS_ITEM_OUTER_JOIN(" << outerJoinRDD.member_id() << ", " << outerJoinRDD.actor_id() << ")";
  sleep(2);

  LOG(INFO) << "=================================";
  LOG(INFO) << "testing inner join";
  auto cnt = idgs::rdd::hash_join_test::countAction(innerJoinRDD);
  ASSERT_EQ(3, cnt);
  LOG_IF(INFO, cnt == 3) << "success";
  LOG_IF(INFO, cnt != 3) << "failed";

  sleep(2);

  LOG(INFO) << "=================================";
  LOG(INFO) << "testing left join";
  cnt = idgs::rdd::hash_join_test::countAction(leftJoinRDD);
  ASSERT_EQ(4, cnt);
  LOG_IF(INFO, cnt == 4) << "success";
  LOG_IF(INFO, cnt != 4) << "failed";

  sleep(2);

  LOG(INFO) << "=================================";
  LOG(INFO) << "testing outer join";
  cnt = idgs::rdd::hash_join_test::countAction(outerJoinRDD);
  ASSERT_EQ(5, cnt);
  LOG_IF(INFO, cnt == 5) << "success";
  LOG_IF(INFO, cnt != 5) << "failed";
  LOG(INFO) << "=================================";
}
