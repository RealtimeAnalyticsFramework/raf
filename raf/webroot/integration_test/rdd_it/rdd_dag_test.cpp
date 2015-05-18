/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "gtest/gtest.h"

#include "idgs/client/rdd/rdd_client.h"

#include "idgs/expr/expression_helper.h"

#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"
#include "idgs/rdd/pb/rdd_transform.pb.h"

#include "protobuf/message_helper.h"

using namespace std;
using namespace idgs::client::rdd;
using namespace idgs::pb;
using namespace idgs::rdd::pb;
using namespace protobuf;

namespace idgs {
namespace rdd {
namespace test {

const char LINEITEM_RDD[] = "LINE_ITEM_RDD";
const char ORDERS_RDD[] = "ORDERS_RDD";
const char REDUCE_LINEITEM_RDD[] = "REDUCE_LINEITEM_RDD";
const char GROUP_LINEITEM_RDD[] = "GROUP_LINEITEM_RDD";
const char REDUCE_BY_KEY_LINEITEM_RDD[] = "REDUCE_BY_KEY_LINEITEM_RDD";
const char JOIN_ORDERS_LINEITEM_RDD[] = "JOIN_ORDERS_LINEITEM_RDD";
const char UNION_ORDERS_RDD[] = "UNION_ORDERS_RDD";

RddClient client;
MessageHelper helper;

void init() {
  client.init("conf/client.conf");
  helper.registerDynamicMessage("integration_test/rdd_it/rdd_dag_test.proto");
}

void buildComplexRDD() {
  LOG(INFO) << "create lineitem delegate RDD";

  DelegateRddRequestPtr lineitemRddRequest = make_shared<CreateDelegateRddRequest>();
  DelegateRddResponsePtr lineitemRddResponse = make_shared<CreateDelegateRddResponse>();
  lineitemRddRequest->set_schema_name("tpch");
  lineitemRddRequest->set_store_name("LineItem");
  lineitemRddRequest->set_rdd_name(LINEITEM_RDD);

  client.createStoreDelegateRDD(lineitemRddRequest, lineitemRddResponse);

  LOG(INFO) << "create orders delegate RDD";

  DelegateRddRequestPtr ordersRddRequest = make_shared<CreateDelegateRddRequest>();
  DelegateRddResponsePtr ordersRddResponse = make_shared<CreateDelegateRddResponse>();
  ordersRddRequest->set_schema_name("tpch");
  ordersRddRequest->set_store_name("Orders");
  ordersRddRequest->set_rdd_name(ORDERS_RDD);

  client.createStoreDelegateRDD(ordersRddRequest, ordersRddResponse);

  LOG(INFO) << "create reduce lineitem RDD";

  RddRequestPtr reduceRddRequest = make_shared<CreateRddRequest>();
  RddResponsePtr reduceRddResponse = make_shared<CreateRddResponse>();

  reduceRddRequest->set_transformer_op_name(REDUCE_TRANSFORMER);

  auto in = reduceRddRequest->add_in_rdd();
  in->set_rdd_name(LINEITEM_RDD);
  in->set_allocated_filter_expr(AND(GE(FIELD("l_shipdate"), CONST("1994-01-01")),
                                    LT(FIELD("l_shipdate"), CONST("1995-01-01")),
                                    GE(FIELD("l_discount"), CONST("0.05", DOUBLE)),
                                    LE(FIELD("l_discount"), CONST("0.07", DOUBLE)),
                                    LT(FIELD("l_quantity"), CONST("24", DOUBLE))
                                   )
                                );
  auto field = in->add_out_fields();
  field->set_allocated_expr(CONST("1", INT32));
  field->set_field_alias("l_totalsize");
  field->set_field_type(UINT32);

  field = in->add_out_fields();
  field->set_allocated_expr(MULTIPLY(FIELD("l_extendedprice"), FIELD("l_discount")));
  field->set_field_alias("l_totalprice");
  field->set_field_type(DOUBLE);

  auto out = reduceRddRequest->mutable_out_rdd();
  out->set_rdd_name(REDUCE_LINEITEM_RDD);
  out->set_persist_type(ORDERED);
  out->set_key_type_name("idgs.pb.Integer");
  out->set_value_type_name("idgs.rdd.pb.ReduceLineitem");
  auto out_field = out->add_value_fields();
  out_field->set_field_name("l_totalsize");
  out_field->set_field_type(UINT32);
  out_field = out->add_value_fields();
  out_field->set_field_name("l_totalprice");
  out_field->set_field_type(DOUBLE);

  shared_ptr<ReduceByKeyRequest> reduceParam = make_shared<ReduceByKeyRequest>();
  auto param_field = reduceParam->add_fields();
  param_field->set_field_alias("l_totalsize");
  param_field->set_type("COUNT");
  param_field->set_distinct(false);
  param_field = reduceParam->add_fields();
  param_field->set_field_alias("l_totalprice");
  param_field->set_type("SUM");
  param_field->set_distinct(false);

  client.createRdd(reduceRddRequest, reduceRddResponse, {{TRANSFORMER_PARAM, reduceParam}});

  VLOG(0) << "create group lineitem RDD";

  RddRequestPtr groupRddRequest = make_shared<CreateRddRequest>();
  RddResponsePtr groupRddResponse = make_shared<CreateRddResponse>();

  groupRddRequest->set_transformer_op_name(GROUP_TRANSFORMER);

  in = groupRddRequest->add_in_rdd();
  in->set_rdd_name(LINEITEM_RDD);
  field = in->add_out_fields();
  field->set_allocated_expr(FIELD("l_orderkey"));
  field->set_field_alias("l_orderkey");
  field->set_field_type(INT64);

  out = groupRddRequest->mutable_out_rdd();
  out->set_rdd_name(GROUP_LINEITEM_RDD);
  out->set_key_type_name("idgs.rdd.pb.GroupLineitem");
  out->set_value_type_name("idgs.sample.tpch.pb.LineItem");
  out_field = out->add_key_fields();
  out_field->set_field_name("l_orderkey");
  out_field->set_field_type(INT64);

  client.createRdd(groupRddRequest, groupRddResponse);

  VLOG(0) << "create reduce by key lineitem RDD";

  RddRequestPtr reduceByKeyRddRequest = make_shared<CreateRddRequest>();
  RddResponsePtr reduceByKeyRddResponse = make_shared<CreateRddResponse>();

  reduceByKeyRddRequest->set_transformer_op_name(REDUCE_BY_KEY_TRANSFORMER);

  in = reduceByKeyRddRequest->add_in_rdd();
  in->set_rdd_name(GROUP_LINEITEM_RDD);
  in->set_allocated_filter_expr(LT(FIELD("l_orderkey"), CONST("50", INT64)));
  field = in->add_out_fields();
  field->set_allocated_expr(CONST("1", INT32));
  field->set_field_alias("l_totalsize");
  field->set_field_type(INT64);

  field = in->add_out_fields();
  field->set_allocated_expr(MULTIPLY(FIELD("l_extendedprice"), FIELD("l_discount")));
  field->set_field_alias("l_totalprice");
  field->set_field_type(INT64);

  out = reduceByKeyRddRequest->mutable_out_rdd();
  out->set_rdd_name(REDUCE_BY_KEY_LINEITEM_RDD);
  out->set_persist_type(ORDERED);
  out->set_key_type_name("idgs.rdd.pb.GroupLineitem");
  out->set_value_type_name("idgs.rdd.pb.ReduceLineitem");
  out_field = out->add_value_fields();
  out_field->set_field_name("l_totalsize");
  out_field->set_field_type(UINT32);
  out_field = out->add_value_fields();
  out_field->set_field_name("l_totalprice");
  out_field->set_field_type(DOUBLE);

  client.createRdd(reduceByKeyRddRequest, reduceByKeyRddResponse, {{TRANSFORMER_PARAM, reduceParam}});

  VLOG(0) << "create join orders lineitem RDD";
  RddRequestPtr joinRddRequest = make_shared<CreateRddRequest>();
  RddResponsePtr joinRddResponse = make_shared<CreateRddResponse>();

  joinRddRequest->set_transformer_op_name(HASH_JOIN_TRANSFORMER);

  in = joinRddRequest->add_in_rdd();
  in->set_rdd_name(ORDERS_RDD);
  in->set_allocated_filter_expr(LT(FIELD("o_orderkey"), CONST("50", INT64)));
  field = in->add_out_fields();
  field->set_allocated_expr(FIELD("o_orderkey"));
  field->set_field_alias("j_orderkey");
  field->set_field_type(INT64);

  field = in->add_out_fields();
  field->set_allocated_expr(FIELD("o_orderdate"));
  field->set_field_alias("j_orderdate");
  field->set_field_type(STRING);

  in = joinRddRequest->add_in_rdd();
  in->set_rdd_name(GROUP_LINEITEM_RDD);
  in->set_allocated_filter_expr(LT(FIELD("l_discount"), CONST("0.05", DOUBLE)));
  field = in->add_out_fields();
  field->set_allocated_expr(FIELD("l_orderkey"));
  field->set_field_alias("j_orderkey");
  field->set_field_type(INT64);

  field = in->add_out_fields();
  field->set_allocated_expr(FIELD("l_extendedprice"));
  field->set_field_alias("j_extendedprice");
  field->set_field_type(DOUBLE);

  field = in->add_out_fields();
  field->set_allocated_expr(FIELD("l_discount"));
  field->set_field_alias("j_discount");
  field->set_field_type(DOUBLE);

  out = joinRddRequest->mutable_out_rdd();
  out->set_rdd_name(JOIN_ORDERS_LINEITEM_RDD);
  out->set_key_type_name("idgs.rdd.pb.JoinOrdersLineitemKey");
  out->set_value_type_name("idgs.rdd.pb.JoinOrdersLineitem");
  out_field = out->add_key_fields();
  out_field->set_field_name("j_orderkey");
  out_field->set_field_type(INT64);
  out_field = out->add_value_fields();
  out_field->set_field_name("j_orderdate");
  out_field->set_field_type(STRING);
  out_field = out->add_value_fields();
  out_field->set_field_name("j_extendedprice");
  out_field->set_field_type(DOUBLE);
  out_field = out->add_value_fields();
  out_field->set_field_name("j_discount");
  out_field->set_field_type(DOUBLE);

  shared_ptr<JoinRequest> joinParam = make_shared<JoinRequest>();
  joinParam->set_type(INNER_JOIN);

  client.createRdd(joinRddRequest, joinRddResponse, {{TRANSFORMER_PARAM, joinParam}});

  VLOG(0) << "create union orders RDD";
  RddRequestPtr unionRddRequest = make_shared<CreateRddRequest>();
  RddResponsePtr unionRddResponse = make_shared<CreateRddResponse>();

  unionRddRequest->set_transformer_op_name(UNION_TRANSFORMER);

  in = unionRddRequest->add_in_rdd();
  in->set_rdd_name(ORDERS_RDD);
  in->set_allocated_filter_expr(AND(LT(FIELD("o_orderkey"), CONST("50", INT64)), EQ(FIELD("o_orderstatus"), CONST("O"))));
  field = in->add_out_fields();
  field->set_allocated_expr(FIELD("o_orderdate"));
  field->set_field_alias("u_orderdate");
  field->set_field_type(STRING);

  field = in->add_out_fields();
  field->set_allocated_expr(FIELD("o_orderstatus"));
  field->set_field_alias("u_orderstatus");
  field->set_field_type(STRING);

  field = in->add_out_fields();
  field->set_allocated_expr(FIELD("o_totalprice"));
  field->set_field_alias("u_ordertotalprice");
  field->set_field_type(DOUBLE);

  in = unionRddRequest->add_in_rdd();
  in->set_rdd_name(ORDERS_RDD);
  in->set_allocated_filter_expr(AND(LT(FIELD("o_orderkey"), CONST("50", INT64)), EQ(FIELD("o_orderstatus"), CONST("F"))));
  field = in->add_out_fields();
  field->set_allocated_expr(FIELD("o_orderdate"));
  field->set_field_alias("u_orderdate");
  field->set_field_type(STRING);

  field = in->add_out_fields();
  field->set_allocated_expr(FIELD("o_orderstatus"));
  field->set_field_alias("u_orderstatus");
  field->set_field_type(STRING);

  field = in->add_out_fields();
  field->set_allocated_expr(FIELD("o_totalprice"));
  field->set_field_alias("u_ordertotalprice");
  field->set_field_type(DOUBLE);

  out = unionRddRequest->mutable_out_rdd();
  out->set_rdd_name(UNION_ORDERS_RDD);
  out->set_key_type_name("idgs.rdd.pb.OrdersKey");
  out->set_value_type_name("idgs.rdd.pb.UnionOrders");
  out_field = out->add_value_fields();
  out_field->set_field_name("u_orderdate");
  out_field->set_field_type(STRING);
  out_field = out->add_value_fields();
  out_field->set_field_name("u_orderstatus");
  out_field->set_field_type(STRING);
  out_field = out->add_value_fields();
  out_field->set_field_name("u_ordertotalprice");
  out_field->set_field_type(DOUBLE);

  client.createRdd(unionRddRequest, unionRddResponse);
}

ActionResultPtr collectAction(const string& action_id, const string& rdd_name) {
  ActionRequestPtr actionRequest = make_shared<ActionRequest>();
  ActionResponsePtr actionResponse = make_shared<ActionResponse>();
  ActionResultPtr actionResult = make_shared<CollectActionResult>();

  actionRequest->set_action_id(action_id);
  actionRequest->set_action_op_name(COLLECT_ACTION);
  actionRequest->set_rdd_name(rdd_name);

  client.sendAction(actionRequest, actionResponse, actionResult);

  return actionResult;
}

void printHeader(const shared_ptr<google::protobuf::Message>& key, const shared_ptr<google::protobuf::Message>& value) {
  string header;
  if (key) {
    auto desc = key->GetDescriptor();
    for (int32_t i = 0; i < desc->field_count(); ++ i) {
      header += desc->field(i)->name() + " | ";
    }
  }

  if (value) {
    auto desc = value->GetDescriptor();
    for (int32_t i = 0; i < desc->field_count(); ++ i) {
      header += desc->field(i)->name() + " | ";
    }
  }

  VLOG(0) << "| " << header;
}

void printData(const shared_ptr<google::protobuf::Message>& key, const shared_ptr<google::protobuf::Message>& value) {
  string data;
  if (key) {
    auto desc = key->GetDescriptor();
    for (int32_t i = 0; i < desc->field_count(); ++ i) {
      PbVariant var;
      MessageHelper().getMessageValue(key.get(), desc->field(i), var);
      string v = var.toString();
      while (v.size() < desc->field(i)->name().size()) {
        v = " " + v;
      }
      data += v + " | ";
    }
  }

  if (value) {
    auto desc = value->GetDescriptor();
    for (int32_t i = 0; i < desc->field_count(); ++ i) {
      PbVariant var;
      MessageHelper().getMessageValue(value.get(), desc->field(i), var);
      string v = var.toString();
      while (v.size() < desc->field(i)->name().size()) {
        v = " " + v;
      }
      data += v + " | ";
    }
  }

  VLOG(0) << "| " << data;
}

}
}
}

TEST(rdd_dag, action) {
  idgs::rdd::test::init();
  idgs::rdd::test::buildComplexRDD();

  VLOG(0) << "sleep 3s to wait RDD build.";
  sleep(3);

  auto reduceResult = idgs::rdd::test::collectAction("100000", idgs::rdd::test::REDUCE_LINEITEM_RDD);

  VLOG(0) << "sleep 5s to wait transformer done.";
  sleep(5);

  size_t count = 0;

  VLOG(0) << "";
  VLOG(0) << "======== reduce result =======";
  CollectActionResult* result = dynamic_cast<CollectActionResult*>(reduceResult.get());

  ASSERT_EQ(1, result->pair_size());
  ASSERT_EQ(1, result->pair(0).value_size());
  auto reduceValue = idgs::rdd::test::helper.createMessage("idgs.rdd.pb.ReduceLineitem");
  ProtoSerdes<0>::deserialize(result->pair(0).value(0), reduceValue.get());

  idgs::rdd::test::printHeader(NULL, reduceValue);
  idgs::rdd::test::printData(NULL, reduceValue);
  ++ count;

  VLOG(0) << "";
  VLOG(0) << "RDD \"" << idgs::rdd::test::REDUCE_LINEITEM_RDD << "\" fetch " << count << " row(s).";

  count = 0;
  VLOG(0) << "";
  VLOG(0) << "=========== reduce by key result ==========";
  auto reduceByKeyResult = idgs::rdd::test::collectAction("100001", idgs::rdd::test::REDUCE_BY_KEY_LINEITEM_RDD);

  auto reduceByKeyKey = idgs::rdd::test::helper.createMessage("idgs.rdd.pb.GroupLineitem");
  auto reduceByKeyValue = idgs::rdd::test::helper.createMessage("idgs.rdd.pb.ReduceLineitem");
  idgs::rdd::test::printHeader(reduceByKeyKey, reduceByKeyValue);

  result = dynamic_cast<CollectActionResult*>(reduceByKeyResult.get());
  for (int32_t i = 0; i < result->pair_size(); ++ i) {
    reduceByKeyKey = idgs::rdd::test::helper.createMessage("idgs.rdd.pb.GroupLineitem");
    ProtoSerdes<0>::deserialize(result->pair(i).key(), reduceByKeyKey.get());
    for (int32_t j = 0; j < result->pair(i).value_size(); ++ j) {
      reduceByKeyValue = idgs::rdd::test::helper.createMessage("idgs.rdd.pb.ReduceLineitem");
      ProtoSerdes<0>::deserialize(result->pair(i).value(j), reduceByKeyValue.get());

      idgs::rdd::test::printData(reduceByKeyKey, reduceByKeyValue);
      ++ count;
    }
  }

  VLOG(0) << "";
  VLOG(0) << "RDD \"" << idgs::rdd::test::REDUCE_BY_KEY_LINEITEM_RDD << "\" fetch " << count << " row(s).";

  count = 0;

  VLOG(0) << "";
  VLOG(0) << "======================= join result =======================";
  auto joinResult = idgs::rdd::test::collectAction("100002", idgs::rdd::test::JOIN_ORDERS_LINEITEM_RDD);

  auto joinKey = idgs::rdd::test::helper.createMessage("idgs.rdd.pb.JoinOrdersLineitemKey");
  auto joinValue = idgs::rdd::test::helper.createMessage("idgs.rdd.pb.JoinOrdersLineitem");
  idgs::rdd::test::printHeader(joinKey, joinValue);

  result = dynamic_cast<CollectActionResult*>(joinResult.get());
  for (int32_t i = 0; i < result->pair_size(); ++ i) {
    joinKey = idgs::rdd::test::helper.createMessage("idgs.rdd.pb.JoinOrdersLineitemKey");
    ProtoSerdes<0>::deserialize(result->pair(i).key(), joinKey.get());
    for (int32_t j = 0; j < result->pair(i).value_size(); ++ j) {
      joinValue = idgs::rdd::test::helper.createMessage("idgs.rdd.pb.JoinOrdersLineitem");
      ProtoSerdes<0>::deserialize(result->pair(i).value(j), joinValue.get());

      idgs::rdd::test::printData(joinKey, joinValue);
      ++ count;
    }
  }

  VLOG(0) << "";
  VLOG(0) << "RDD \"" << idgs::rdd::test::JOIN_ORDERS_LINEITEM_RDD << "\" fetch " << count << " row(s).";

  count = 0;

  VLOG(0) << "";
  VLOG(0) << "=================== union result ==================";
  auto unionResult = idgs::rdd::test::collectAction("100003", idgs::rdd::test::UNION_ORDERS_RDD);

  auto unionValue = idgs::rdd::test::helper.createMessage("idgs.rdd.pb.UnionOrders");
  idgs::rdd::test::printHeader(NULL, unionValue);

  result = dynamic_cast<CollectActionResult*>(unionResult.get());
  for (int32_t i = 0; i < result->pair_size(); ++ i) {
    for (int32_t j = 0; j < result->pair(i).value_size(); ++ j) {
      unionValue = idgs::rdd::test::helper.createMessage("idgs.rdd.pb.UnionOrders");
      ProtoSerdes<0>::deserialize(result->pair(i).value(j), unionValue.get());

      idgs::rdd::test::printData(NULL, unionValue);
      ++ count;
    }
  }

  VLOG(0) << "";
  VLOG(0) << "RDD \"" << idgs::rdd::test::UNION_ORDERS_RDD << "\" fetch " << count << " row(s).";
}
