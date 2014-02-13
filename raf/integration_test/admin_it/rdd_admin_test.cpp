
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include "gtest/gtest.h"
#include "idgs/client/admin/admin_client.h"
#include "idgs/client/rdd/rdd_client.h"

#include "idgs/rdd/pb/rdd_transform.pb.h"
#include "idgs/rdd/pb/rdd_action.pb.h"

using namespace std;
using namespace idgs;
using namespace idgs::pb;
using namespace idgs::util;

using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;



using namespace idgs::admin::pb;
using namespace idgs::client::admin;


using namespace idgs::client;



ActorId joinOrderRDD, joinItemRDD;
namespace idgs {
namespace admin {
namespace rdd_admin_test {

idgs::pb::ActorId createStoreDelegateRDD(const std::string& storeName) {
  DelegateRddRequestPtr request(new CreateDelegateRddRequest);
  DelegateRddResponsePtr response(new CreateDelegateRddResponse);

  request->set_store_name(storeName);
  request->mutable_rdd_name()->append(storeName);

  singleton<RddClient>::getInstance().createStoreDelegateRDD(request, response);

  return response->rdd_id();
}

ActorId createLineItemGroupRdd(const string& rddName) {
  RddRequestPtr request(new CreateRddRequest);
  RddResponsePtr response(new CreateRddResponse);

  request->set_transformer_op_name(GROUP_TRANSFORMER);

  InRddInfo* in = request->add_in_rdd();
  in->mutable_rdd_name()->append(rddName);

  FieldNamePair* field = in->add_out_fields();
  auto expr = field->mutable_expr();
  expr->set_type(FIELD);
  expr->set_value("l_orderkey");

  field = in->add_out_fields();
  expr = field->mutable_expr();
  expr->set_type(FIELD);
  expr->set_value("l_extendedprice");

  field = in->add_out_fields();
  expr = field->mutable_expr();
  expr->set_type(FIELD);
  expr->set_value("l_discount");

  auto out = request->mutable_out_rdd();
  out->set_rdd_name("LINE_ITEM_GROUP");
  out->set_data_type(idgs::rdd::pb::ORDERED);
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

  singleton<RddClient>::getInstance().createRdd(request, response);
  return response->rdd_id();
}

ActorId createJoinRdd(const ActorId& orderRdd, const ActorId& lineItemRDD, const string& rddName, const JoinType& type) {
  RddRequestPtr request(new CreateRddRequest);
  RddResponsePtr response(new CreateRddResponse);

  request->set_transformer_op_name(HASH_JOIN_TRANSFORMER);

  InRddInfo* in = request->add_in_rdd();
  in->mutable_rdd_id()->CopyFrom(orderRdd);
  in->mutable_rdd_name()->append("Orders");

  FieldNamePair* field = in->add_out_fields();
  field->set_field_alias("orderkey");
  auto expr = field->mutable_expr();
  expr->set_type(FIELD);
  expr->set_value("o_orderkey");

  field = in->add_out_fields();
  field->set_field_alias("total");
  expr = field->mutable_expr();
  expr->set_type(FIELD);
  expr->set_value("o_totalprice");

  in = request->add_in_rdd();
  in->mutable_rdd_id()->CopyFrom(lineItemRDD);
  in->mutable_rdd_name()->append("LINE_ITEM_GROUP");

  field = in->add_out_fields();
  field->set_field_alias("orderkey");
  expr = field->mutable_expr();
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
  out->set_rdd_name(rddName);
  out->set_data_type(idgs::rdd::pb::ORDERED);
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

  shared_ptr<JoinRequest> param(new JoinRequest);
  param->set_type(type);

  auto joinfld = param->add_join_field();
  joinfld->set_l_join_field("o_orderkey");
  joinfld->set_r_join_field("l_orderkey");

  map<string, shared_ptr<google::protobuf::Message>> params;
  params[JOIN_PARAM] = param;
  singleton<RddClient>::getInstance().createRdd(request, response, params);
  return response->rdd_id();
}

uint32_t countAction(const ActorId& rddId) {
  ActionRequestPtr request(new ActionRequest);
  ActionResponsePtr response(new ActionResponse);
  ActionResultPtr result(new CountActionResult);

  request->set_action_id("100000");
  request->set_action_op_name(COUNT_ACTION);

  singleton<RddClient>::getInstance().sendAction(request, response, result, rddId);

  auto size = dynamic_cast<CountActionResult*>(result.get())->size();
  return size;
}

} // namespace rdd_admin_test
} // namespace admin
} // namespace idgs

using namespace idgs::admin::rdd_admin_test;

TEST(rdd_admin, get_rdd_info_test) {

  //----------------------------- create rdd ------------------------------------------------------------------
  ResultCode code = singleton<RddClient>::getInstance().init("integration_test/rdd_it/client.conf");
  if (code != RC_SUCCESS) {
    exit(1);
  }

  joinOrderRDD = idgs::admin::rdd_admin_test::createStoreDelegateRDD("Orders");

  auto itemDelegateRDD = idgs::admin::rdd_admin_test::createStoreDelegateRDD("LineItem");

  sleep(2);

  joinItemRDD = createLineItemGroupRdd("LineItem");

  sleep(2);

  auto joinRDD = createJoinRdd(joinOrderRDD, joinItemRDD, "ORDER_ITEM_INNER_JOIN", INNER_JOIN);

  sleep(5);

  //---------------------------- create rdd info request ----------------------------------------------------

  shared_ptr<TcpClientInterface> client = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(code);

  std::shared_ptr<RddModuleRequest> rddInfoRequest(new RddModuleRequest);
  rddInfoRequest->addDependingRddInfoAttr("ORDER_ITEM_INNER_JOIN");
  idgs::client::ClientActorMessagePtr actorReq = AdminUtil::createAdminRequest(rddInfoRequest, GET);

  idgs::client::ClientActorMessagePtr tcpResponse = client->sendRecv(actorReq, &code);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in get data to partition store, cause by " << getErrorDescription(code);
    exit(1);
  }

  idgs::admin::pb::AdminResponse response;
  tcpResponse->parsePayload(&response);

  AttributeResponse attrResp;
  EXPECT_EQ(true, AdminUtil::getAttributeResponse(response, RDD, idgs::admin::rdd::RDD_INFO+";rdd_name=ORDER_ITEM_INNER_JOIN", attrResp));

  LOG(INFO) << "get response: " << attrResp.DebugString();
  LOG(INFO) << "get attribute: " << attrResp.attribute();

  EXPECT_EQ(idgs::admin::pb::ResponseStatus::Success, attrResp.status());
  //EXPECT_EQ("membership_table.all_members_info", attrResp.attribute());

  LOG(INFO) << "----- Admin test for RDD info is Success!!!!";

  //--------------------------- create rdd dependency request ---------------------------------------

  sleep(3);

  std::shared_ptr<RddModuleRequest> rddDependencyRequest(new RddModuleRequest);
  rddDependencyRequest->addRddDependencyAttr("ORDER_ITEM_INNER_JOIN");
  idgs::client::ClientActorMessagePtr actorReq2 = AdminUtil::createAdminRequest(rddDependencyRequest, GET);

  idgs::client::ClientActorMessagePtr tcpResponse2 = client->sendRecv(actorReq2, &code);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in get data to partition store, cause by " << getErrorDescription(code);
    exit(1);
  }

  idgs::admin::pb::AdminResponse dpdResponse;
  tcpResponse2->parsePayload(&dpdResponse);

  AttributeResponse attrResp2;
  LOG(INFO) << "get response: " << attrResp2.DebugString();
  LOG(INFO) << "get attribute: " << attrResp2.attribute();

  EXPECT_EQ(true, AdminUtil::getAttributeResponse(dpdResponse, RDD, idgs::admin::rdd::RDD_DEPENDENCY+";rdd_name=ORDER_ITEM_INNER_JOIN", attrResp2));


  EXPECT_EQ(idgs::admin::pb::ResponseStatus::Success, attrResp2.status());
  //EXPECT_EQ("membership_table.all_members_info", attrResp.attribute());

  LOG(INFO) << "----- Admin test for RDD Dependency is Success!!!!";
}

