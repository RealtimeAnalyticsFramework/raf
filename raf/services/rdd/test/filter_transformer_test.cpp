
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
////#if defined(__GNUC__) || defined(__clang__)
////#include "idgs_gch.h"
////#endif // GNUC_ $

#include "idgs/rdd/transform/filter_transformer.h"
#include "idgs/store/data_store.h"
#include "protobuf/message_helper.h"

using namespace std;
using namespace idgs::store;
using namespace protobuf;

idgs::rdd::RddPartition* input;

void initialize() {
  ::idgs::util::singleton<DataStore>::getInstance().loadCfgFile("services/store/test/data_store.conf");

  input = new idgs::rdd::RddPartition(0);
  MessageHelper& helper = idgs::util::singleton<MessageHelper>::getInstance();
  string keyType = "idgs.sample.tpch.pb.LineItemKey", valueType = "idgs.sample.tpch.pb.LineItem";
  input->setRddPartitionContext(helper.createMessage(keyType), helper.createMessage(valueType));

  string date[] = {"2000-01-01", "1994-04-20", "1994-12-11", "1994-01-01", "1993-10-26", "1994-01-02", "1993-12-31", "1890-05-31", "1994-01-01", "1994-03-25"};
  double discount[] = {0.06, 0.04, 0.05, 0.06, 0.09, 0.07, 0.07, 0.08, 0.05, 0.06};
  double quantity[] = {19, 20, 29, 24, 25, 18, 20, 24, 23, 22};
  double price[] = {1900, 2000, 2900, 2400, 2500, 1800, 2000, 2400, 2300, 2200};

  for (int32_t i = 0; i < 10; ++ i) {
    auto key = helper.createMessage(keyType);
    key->GetReflection()->SetInt64(key.get(), key->GetDescriptor()->FindFieldByName("l_linenumber"), i + 1);
    key->GetReflection()->SetInt64(key.get(), key->GetDescriptor()->FindFieldByName("l_orderkey"), 1);

    auto value = helper.createMessage(valueType);
    value->GetReflection()->SetString(value.get(), value->GetDescriptor()->FindFieldByName("l_shipdate"), date[i]);
    value->GetReflection()->SetDouble(value.get(), value->GetDescriptor()->FindFieldByName("l_discount"), discount[i]);
    value->GetReflection()->SetDouble(value.get(), value->GetDescriptor()->FindFieldByName("l_quantity"), quantity[i]);
    value->GetReflection()->SetDouble(value.get(), value->GetDescriptor()->FindFieldByName("l_extendedprice"), price[i]);

    input->putLocal(key, value);
  }
}

//TEST(filter_data, GE) {
//  idgs::util::singleton<idgs::cluster::ClusterFramework>::getInstance().loadCfgFile("framework/conf/cluster.conf");
//  LOG(INFO) << "starting initialize data.";
//  initialize();
//
//  LOG(INFO) << "test data where quantity >= 24.";
//  shared_ptr<CreateRddRequest> request(new CreateRddRequest);
//  request->set_transformer_op_name(FILTER_TRANSFORMER);
//
//  shared_ptr<Expr> exp(new Expr);
//  exp->set_type(GE);
//  auto lquantity = exp->add_expression();
//  lquantity->set_type(FIELD);
//  lquantity->set_value("l_quantity");
//
//  auto rquantity = exp->add_expression();
//  rquantity->set_type(CONST);
//  rquantity->set_const_type(DOUBLE);
//  rquantity->set_value("24");
//
//  in->mutable_filter_expr()->CopyFrom(* exp);
//  ActorMessagePtr msg(new ActorMessage);
//  msg->setPayload(request);
//  idgs::ResultCode code;
//  msg->toBuffer(code);
//  //EXPECT_EQ(RC_SUCCESS, code);
//
//  LOG(INFO) << "start filter";
//  vector<idgs::rdd::BaseRddPartition*> inputRdd;
//  inputRdd.push_back(input);
//  idgs::rdd::RddPartition* output = new idgs::rdd::RddPartition(0);
//  auto resultCode = ::idgs::util::singleton<FilterTransformer>::getInstance().transform(msg, inputRdd, output);
//  EXPECT_EQ(RRC_SUCCESS, resultCode);
//  LOG(INFO) << "result is " << resultCode;
//
//  EXPECT_EQ(4, output->size());
//
//  output->foreach([] (const PbMessagePtr& key, const PbMessagePtr& value) {
//    LOG(INFO) << "-------------------------------";
//    LOG(INFO) << "linenumber : " << key->GetReflection()->GetInt64(* key, key->GetDescriptor()->FindFieldByName("l_linenumber"));
//    LOG(INFO) << "shipdate : " << value->GetReflection()->GetString(* value, value->GetDescriptor()->FindFieldByName("l_shipdate"));
//    LOG(INFO) << "discount : " << value->GetReflection()->GetDouble(* value, value->GetDescriptor()->FindFieldByName("l_discount"));
//    LOG(INFO) << "quantity : " << value->GetReflection()->GetDouble(* value, value->GetDescriptor()->FindFieldByName("l_quantity"));
//    LOG(INFO) << "price : " << value->GetReflection()->GetDouble(* value, value->GetDescriptor()->FindFieldByName("l_extendedprice"));
//    LOG(INFO) << "-------------------------------";
//  });
//
//  delete output;
//}
//
//TEST(filter_data, LT) {
//  LOG(INFO) << "test data where shipdata < 1994-01-01.";
//
//  shared_ptr<CreateRddRequest> request(new CreateRddRequest);
//  request->set_transformer_op_name(FILTER_TRANSFORMER);
//
//  shared_ptr<Expr> exp(new Expr);
//  exp->set_type(LT);
//  auto ldate = exp->add_expression();
//  ldate->set_type(FIELD);
//  ldate->set_value("l_shipdate");
//
//  auto rdate = exp->add_expression();
//  rdate->set_type(CONST);
//  rdate->set_const_type(STRING);
//  rdate->set_value("1994-01-01");
//
//  in->mutable_filter_expr()->CopyFrom(* exp);
//
//  ActorMessagePtr msg(new ActorMessage);
//  msg->setPayload(request);
//  idgs::ResultCode code;
//  msg->toBuffer(code);
//  //EXPECT_EQ(RC_SUCCESS, code);
//
//  vector<idgs::rdd::BaseRddPartition*> inputRdd;
//  inputRdd.push_back(input);
//  idgs::rdd::RddPartition* output = new idgs::rdd::RddPartition(0);
//  auto resultCode = ::idgs::util::singleton<FilterTransformer>::getInstance().transform(msg, inputRdd, output);
//  EXPECT_EQ(RRC_SUCCESS, resultCode);
//
//  EXPECT_EQ(3, output->size());
//
//  output->foreach([] (const PbMessagePtr& key, const PbMessagePtr& value) {
//    LOG(INFO) << "-------------------------------";
//    LOG(INFO) << "linenumber : " << key->GetReflection()->GetInt64(* key, key->GetDescriptor()->FindFieldByName("l_linenumber"));
//    LOG(INFO) << "shipdate : " << value->GetReflection()->GetString(* value, value->GetDescriptor()->FindFieldByName("l_shipdate"));
//    LOG(INFO) << "discount : " << value->GetReflection()->GetDouble(* value, value->GetDescriptor()->FindFieldByName("l_discount"));
//    LOG(INFO) << "quantity : " << value->GetReflection()->GetDouble(* value, value->GetDescriptor()->FindFieldByName("l_quantity"));
//    LOG(INFO) << "price : " << value->GetReflection()->GetDouble(* value, value->GetDescriptor()->FindFieldByName("l_extendedprice"));
//    LOG(INFO) << "-------------------------------";
//  });
//
//  delete output;
//}
//
//TEST(filter_data, group_test) {
//  LOG(INFO) << "test data where shipdata >= 1994-01-01";
//  LOG(INFO) << "            and shipdata < 1995-01-01";
//  LOG(INFO) << "            and discount >= 0.05";
//  LOG(INFO) << "            and discount <= 0.07";
//  LOG(INFO) << "            and quantity < 24";
//
//  shared_ptr<CreateRddRequest> request(new CreateRddRequest);
//  request->set_transformer_op_name(FILTER_TRANSFORMER);
//
//  shared_ptr<Expr> expression(new Expr);
//  expression->set_type(AND);
//
//  // >= 1994-01-01
//  Expr* exp1 = expression->add_expression();
//  exp1->set_type(GE);
//
//  Expr* elem1 = exp1->add_expression();
//  elem1->set_type(FIELD);
//  elem1->set_value("l_shipdate");
//
//  Expr* elem2 = exp1->add_expression();
//  elem2->set_type(CONST);
//  elem2->set_const_type(STRING);
//  elem2->set_value("1994-01-01");
//
//  // < 1995-01-01
//  Expr* exp2 = expression->add_expression();
//  exp2->set_type(LT);
//
//  elem1 = exp2->add_expression();
//  elem1->set_type(FIELD);
//  elem1->set_value("l_shipdate");
//
//  elem2 = exp2->add_expression();
//  elem2->set_type(CONST);
//  elem2->set_const_type(STRING);
//  elem2->set_value("1995-01-01");
//
//  // discount >= 0.05
//  Expr* exp3 = expression->add_expression();
//  exp3->set_type(GE);
//
//  elem1 = exp3->add_expression();
//  elem1->set_type(FIELD);
//  elem1->set_value("l_discount");
//
//  elem2 = exp3->add_expression();
//  elem2->set_type(CONST);
//  elem2->set_const_type(STRING);
//  elem2->set_value("0.05");
//
//  // discount <= 0.07
//  auto exp4 = expression->add_expression();
//  exp4->set_type(LE);
//  elem1 = exp4->add_expression();
//  elem1->set_type(FIELD);
//  elem1->set_value("l_discount");
//
//  elem2 = exp4->add_expression();
//  elem2->set_type(CONST);
//  elem2->set_const_type(STRING);
//  elem2->set_value("0.07");
//
//  // quantity < 24
//  auto emp5 = expression->add_expression();
//  emp5->set_type(LT);
//  elem1 = emp5->add_expression();
//  elem1->set_type(FIELD);
//  elem1->set_value("l_quantity");
//
//  elem2 = emp5->add_expression();
//  elem2->set_type(CONST);
//  elem2->set_const_type(STRING);
//  elem2->set_value("24");
//
//  in->mutable_filter_expr()->CopyFrom(* expression);
//
//  ActorMessagePtr msg(new ActorMessage);
//  msg->setPayload(request);
//
//  idgs::ResultCode code;
//  msg->toBuffer(code);
//  //EXPECT_EQ(RC_SUCCESS, code);
//
//  vector<idgs::rdd::BaseRddPartition*> inputRdd;
//  inputRdd.push_back(input);
//  idgs::rdd::RddPartition* output = new idgs::rdd::RddPartition(0);
//  auto resultCode = ::idgs::util::singleton<FilterTransformer>::getInstance().transform(msg, inputRdd, output);
//  EXPECT_EQ(RRC_SUCCESS, resultCode);
//
//  EXPECT_EQ(3, output->size());
//
//  output->foreach([] (const PbMessagePtr& key, const PbMessagePtr& value) {
//    LOG(INFO) << "-------------------------------";
//    LOG(INFO) << "linenumber : " << key->GetReflection()->GetInt64(* key, key->GetDescriptor()->FindFieldByName("l_linenumber"));
//    LOG(INFO) << "shipdate : " << value->GetReflection()->GetString(* value, value->GetDescriptor()->FindFieldByName("l_shipdate"));
//    LOG(INFO) << "discount : " << value->GetReflection()->GetDouble(* value, value->GetDescriptor()->FindFieldByName("l_discount"));
//    LOG(INFO) << "quantity : " << value->GetReflection()->GetDouble(* value, value->GetDescriptor()->FindFieldByName("l_quantity"));
//    LOG(INFO) << "price : " << value->GetReflection()->GetDouble(* value, value->GetDescriptor()->FindFieldByName("l_extendedprice"));
//    LOG(INFO) << "-------------------------------";
//  });
//
//  delete output;
//  delete input;
//}
