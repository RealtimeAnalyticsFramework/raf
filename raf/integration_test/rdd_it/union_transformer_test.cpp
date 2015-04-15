
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "gtest/gtest.h"
#include "idgs/cancelable_timer.h"
#include "idgs/client/rdd/rdd_client.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"
#include "idgs/expr/expression_helper.h"

using namespace std;
using namespace idgs;
using namespace idgs::rdd::pb;
using namespace idgs::pb;

using namespace idgs::client::rdd;

namespace idgs {
namespace rdd {
namespace union_test {

RddClient client;

ResultCode init(const std::string& config) {
  return client.init(config);
}

void createStoreDelegateRDD(const std::string& schemaName, const std::string& storeName, const std::string& rddName) {
  DelegateRddRequestPtr request = std::make_shared<CreateDelegateRddRequest>();
  DelegateRddResponsePtr response = std::make_shared<CreateDelegateRddResponse>();

  request->set_schema_name(schemaName);
  request->set_store_name(storeName);
  request->set_rdd_name(rddName);

  client.createStoreDelegateRDD(request, response);

  if (response->result_code() != RRC_SUCCESS) {
    LOG(ERROR) << "Create store delegate RDD with store " << storeName << " error, caused by code " << response->result_code();
    exit(1);
  }

  LOG(INFO) << "Store delegate rdd with store " << storeName << " member id : " << response->rdd_id().member_id() << ", actor id : "<< response->rdd_id().actor_id();
}

void createUnionRDD(const std::string& rddName, const std::string& inRddName1, const std::string& inRddName2) {
  RddRequestPtr request = std::make_shared<CreateRddRequest>();
  RddResponsePtr response = std::make_shared<CreateRddResponse>();

  request->set_transformer_op_name(UNION_TRANSFORMER);

  InRddInfo* in = request->add_in_rdd();
  in->set_rdd_name(inRddName1);

  FieldNamePair* field = in->add_out_fields();
  field->set_field_alias("order_key");
  auto expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("l_orderkey");

  field = in->add_out_fields();
  field->set_field_alias("line_number");
  expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("l_linenumber");

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

  in = request->add_in_rdd();
  in->set_rdd_name(inRddName2);

  field = in->add_out_fields();
  field->set_field_alias("order_key");
  expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("lo_orderkey");

  field = in->add_out_fields();
  field->set_field_alias("line_number");
  expr = field->mutable_expr();
  expr->set_name("FIELD");
  expr->set_value("lo_linenumber");

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

  auto out = request->mutable_out_rdd();
  out->set_rdd_name(rddName);
  out->set_key_type_name("idgs.sample.tpch.pb.CLineOrderKey");
  out->set_value_type_name("idgs.sample.tpch.pb.CLineOrder");

  auto fld = out->add_key_fields();
  fld->set_field_name("order_key");
  fld->set_field_type(UINT64);

  fld = out->add_key_fields();
  fld->set_field_name("line_number");
  fld->set_field_type(UINT64);

  fld = out->add_value_fields();
  fld->set_field_name("discount");
  fld->set_field_type(DOUBLE);

  fld = out->add_value_fields();
  fld->set_field_name("price");
  fld->set_field_type(DOUBLE);

  client.createRdd(request, response);

  if (response->result_code() != RRC_SUCCESS) {
    LOG(ERROR) << "Create union RDD error, caused by code " << response->result_code();
    exit(1);
  }

  LOG(INFO) << "Union rdd member id : " << response->rdd_id().member_id() << ", actor id : "<< response->rdd_id().actor_id();
}

void countAction(const std::string& rddName) {
  ActionRequestPtr request = std::make_shared<ActionRequest>();
  ActionResponsePtr response = std::make_shared<ActionResponse>();
  ActionResultPtr result = std::make_shared<CountActionResult>();

  request->set_action_id(COUNT_ACTION);
  request->set_action_op_name(COUNT_ACTION);
  request->set_rdd_name(rddName);

  client.sendAction(request, response, result);

  if (response->result_code() != RRC_SUCCESS) {
    LOG(ERROR) << "Execute count action error, caused by code " << response->result_code();
    exit(1);
  }

  auto size = dynamic_cast<CountActionResult*>(result.get())->size();
  LOG(INFO) << "data size : " << size;
}

void sumAction(const std::string& rddName) {
  shared_ptr<Expr> exp(MULTIPLY(FIELD("price"), FIELD("discount")));

  ActionRequestPtr request = std::make_shared<ActionRequest>();
  ActionResponsePtr response = std::make_shared<ActionResponse>();
  ActionResultPtr result = std::make_shared<SumActionResult>();

  request->set_action_id(SUM_ACTION);
  request->set_action_op_name(SUM_ACTION);
  request->set_rdd_name(rddName);

  request->mutable_expression()->CopyFrom(*exp);

  client.sendAction(request, response, result);

  if (response->result_code() != RRC_SUCCESS) {
    LOG(ERROR) << "Execute sum action error, caused by code " << response->result_code();
    exit(1);
  }

  auto total = dynamic_cast<SumActionResult*>(result.get())->total();
  LOG(INFO) << "Sum total price : " << std::fixed << total;
}

} // namespace union_test
} // namespace rdd
} // namespace idgs

TEST(RDD_union, count_action) {
  TEST_TIMEOUT(30);
  if (idgs::rdd::union_test::init("conf/client.conf") != RC_SUCCESS) {
    exit(1);
  }

  LOG(INFO) << "Create store delegate RDD for store LineItem.";
  idgs::rdd::union_test::createStoreDelegateRDD("tpch", "LineItem", "RDD_TB_LINE_ITEM");

  LOG(INFO) << "Create store delegate RDD for store ssb_lineorder.";
  idgs::rdd::union_test::createStoreDelegateRDD("ssb", "ssb_lineorder", "RDD_TB_LINE_ORDER");

  sleep(3);

  LOG(INFO) << "Create Union RDD.";
  idgs::rdd::union_test::createUnionRDD("UNION_RDD", "RDD_TB_LINE_ITEM", "RDD_TB_LINE_ORDER");
  sleep(2);

  LOG(INFO) << "Count data size after union.";
  idgs::rdd::union_test::countAction("UNION_RDD");
  sleep(2);

  LOG(INFO) << "Calculate sum(discount * price).";
  idgs::rdd::union_test::sumAction("UNION_RDD");
}
