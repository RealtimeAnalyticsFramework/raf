
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include "gtest/gtest.h"


TEST(rdd_destroy, dummy) {
  return;
}

//#include "idgs/cancelable_timer.h"
//#include "idgs/expr/expression_helper.h"
//#include "idgs/client/rdd/rdd_client.h"
//
//#include "idgs/rdd/pb/rdd_action.pb.h"

//using namespace std;
//using namespace idgs;
//using namespace idgs::pb;
//using namespace idgs::rdd;
//using namespace idgs::rdd::pb;
//using namespace idgs::client;
//using namespace idgs::client::admin;
//using namespace idgs::client::rdd;
//
//std::shared_ptr<Expr> buildTpchQ6FilterExpression() {
//  auto expr = AND(GE(FIELD("l_shipdate"), CONST("1994-01-01")),
//                    LT(FIELD("l_shipdate"), CONST("1995-01-01")),
//                    GE(FIELD("l_discount"), CONST("0.05", DOUBLE)),
//                    LE(FIELD("l_discount"), CONST("0.07", DOUBLE)),
//                    LT(FIELD("l_quantity"), CONST("24", DOUBLE))
//                   );
//  shared_ptr<Expr> expression(expr);
//  return expression;
//}
//
//std::shared_ptr<Expr> buildTpchQ6ActionExpression() {
//  auto expr = MULTIPLY(FIELD("l_extendedprice"), FIELD("l_discount"));
//  shared_ptr<Expr> expression(expr);
//  return expression;
//}
//
//idgs::pb::ActorId delegateRddIdForDestroyIT;
//idgs::pb::ActorId filterRddIdForDestroyIT;
//
//TEST(rdd_destroy, createRDD) {
//  TEST_TIMEOUT(30);
//
//  int32_t totalCount = 100;
//  char* q6Time = getenv("TPCH_Q6_LOOP");
//  if (q6Time) {
//    totalCount = atoi(q6Time);
//    if (totalCount < 0) {
//      totalCount = 100;
//    }
//  }
//
//  RddClient client;
//  ResultCode code = client.init("conf/client.conf");
//  if (code != RC_SUCCESS) {
//    LOG(ERROR) << "Error in init client, cause by " << idgs::getErrorDescription(code);
//    exit(1);
//  }
//
//  DelegateRddRequestPtr delegateRddRequest = std::make_shared<CreateDelegateRddRequest>();
//  DelegateRddResponsePtr delegateRddResponse = std::make_shared<CreateDelegateRddResponse>();
//  delegateRddRequest->set_schema_name("tpch");
//  delegateRddRequest->set_store_name("LineItem");
//  delegateRddRequest->set_rdd_name("LINEITEM_DELEGATE_RDD");
//
//  client.createStoreDelegateRDD(delegateRddRequest, delegateRddResponse);
//  delegateRddIdForDestroyIT = delegateRddResponse->rdd_id();
//
//  RddRequestPtr filterRequest = std::make_shared<CreateRddRequest>();
//  RddResponsePtr filterResponse = std::make_shared<CreateRddResponse>();
//  filterRequest->set_transformer_op_name(FILTER_TRANSFORMER);
//
//  auto in = filterRequest->add_in_rdd();
//  in->set_rdd_name("LINEITEM_DELEGATE_RDD");
//
//  auto out = filterRequest->mutable_out_rdd();
//  out->set_rdd_name("TPCH_Q6_RDD");
//  out->set_key_type_name("idgs.sample.tpch.pb.LineItemKey");
//  out->set_value_type_name("idgs.sample.tpch.pb.LineItem");
//
//  auto exp = buildTpchQ6FilterExpression();
//  in->mutable_filter_expr()->CopyFrom(* exp);
//
//  client.createRdd(filterRequest, filterResponse);
//  filterRddIdForDestroyIT = filterResponse->rdd_id();
//
//  auto actionExp = buildTpchQ6ActionExpression();
//  ActionRequestPtr actionRequest = std::make_shared<ActionRequest>();
//  ActionResponsePtr actionResponse = std::make_shared<ActionResponse>();
//  ActionResultPtr actionResult = std::make_shared<SumActionResult>();
//
//  actionRequest->set_action_id("100000");
//  actionRequest->set_action_op_name(SUM_ACTION);
//  actionRequest->mutable_expression()->CopyFrom(* actionExp);
//
//  client.sendAction(actionRequest, actionResponse, actionResult, filterRddIdForDestroyIT);
//
//  ASSERT_EQ("100000", actionResponse->action_id());
//  ASSERT_EQ(RRC_SUCCESS, actionResponse->result_code());
//  VLOG(0) << "result is : " << dynamic_cast<SumActionResult*>(actionResult.get())->total();
//}
//
//TEST(rdd_destroy, destroy) {
//  ResultCode code = RC_SUCCESS;
//  auto client = getTcpClientPool().getTcpClient(code);
//  ASSERT_EQ(RC_SUCCESS, code);
//
//  shared_ptr<DestroyRddRequest> delegateRequest = make_shared<DestroyRddRequest>();
//  delegateRequest->set_rdd_name("LINEITEM_DELEGATE_RDD");
//
//  ClientActorMessagePtr delegateClientActorMsg = std::make_shared<ClientActorMessage>();
//  delegateClientActorMsg->setOperationName(RDD_DESTROY);
//  delegateClientActorMsg->setChannel(TC_TCP);
//  delegateClientActorMsg->setDestActorId(RDD_SERVICE_ACTOR);
//  delegateClientActorMsg->setDestMemberId(ANY_MEMBER);
//  delegateClientActorMsg->setSourceActorId("client_actor_id");
//  delegateClientActorMsg->setSourceMemberId(CLIENT_MEMBER);
//  delegateClientActorMsg->setPayload(delegateRequest);
//
//  ClientActorMessagePtr delegateResp;
//  client->sendRecv(delegateClientActorMsg, delegateResp);
//  sleep(3);
//
//  shared_ptr<DestroyRddRequest> request = make_shared<DestroyRddRequest>();
//  request->set_rdd_name("TPCH_Q6_RDD");
//
//  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
//  clientActorMsg->setOperationName(RDD_DESTROY);
//  clientActorMsg->setChannel(TC_TCP);
//  clientActorMsg->setDestActorId(RDD_SERVICE_ACTOR);
//  clientActorMsg->setDestMemberId(ANY_MEMBER);
//  clientActorMsg->setSourceActorId("client_actor_id");
//  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
//  clientActorMsg->setPayload(request);
//
//  ClientActorMessagePtr resp;
//  client->sendRecv(clientActorMsg, resp);
//  sleep(3);
//}
//
//TEST(rdd_destroy, varifer_delegate_rdd_not_found) {
//  ResultCode code = RC_SUCCESS;
//  auto client = getTcpClientPool().getTcpClient(code);
//  ASSERT_EQ(RC_SUCCESS, code);
//
//  std::shared_ptr<idgs::admin::pb::AdminRequest> adminReq = std::make_shared<idgs::admin::pb::AdminRequest>();
//  auto moduleRequest = adminReq->add_module_op_request();
//  moduleRequest->set_module_name("rdd");
//  moduleRequest->add_attributes()->set_attribute("rdd_info;rdd_name=LINEITEM_DELEGATE_RDD");
//
//  idgs::client::ClientActorMessagePtr clientReq = std::make_shared<idgs::client::ClientActorMessage>();
//  clientReq->setOperationName("get");
//  clientReq->setDestActorId(ADMIN_ACTOR_ID);
//  clientReq->setSourceActorId("client_actor_id");
//  clientReq->setDestMemberId(idgs::pb::ANY_MEMBER);
//  clientReq->setSourceMemberId(idgs::pb::CLIENT_MEMBER);
//  clientReq->setPayload(adminReq);
//
//  idgs::client::ClientActorMessagePtr clientResp;
//  code = client->sendRecv(clientReq, clientResp);
//
//  idgs::admin::pb::AdminResponse adminResp;
//  clientResp->parsePayload(&adminResp);
//
//  auto & attr = adminResp.module_op_response(0).attributes(0);
//  ASSERT_EQ("rdd", attr.module_name());
//  ASSERT_EQ(idgs::admin::pb::Error, attr.status());
//  ASSERT_EQ("can not find rdd actor for name : LINEITEM_DELEGATE_RDD", attr.value());
//}
//
//TEST(rdd_destroy, varifer_filter_rdd_not_found) {
//  ResultCode code = RC_SUCCESS;
//  auto client = getTcpClientPool().getTcpClient(code);
//  ASSERT_EQ(RC_SUCCESS, code);
//
//  std::shared_ptr<idgs::admin::pb::AdminRequest> adminReq = std::make_shared<idgs::admin::pb::AdminRequest>();
//  auto moduleRequest = adminReq->add_module_op_request();
//  moduleRequest->set_module_name("rdd");
//  moduleRequest->add_attributes()->set_attribute("rdd_info;rdd_name=TPCH_Q6_RDD");
//
//  idgs::client::ClientActorMessagePtr clientReq = std::make_shared<idgs::client::ClientActorMessage>();
//  clientReq->setOperationName("get");
//  clientReq->setDestActorId(ADMIN_ACTOR_ID);
//  clientReq->setSourceActorId("client_actor_id");
//  clientReq->setDestMemberId(idgs::pb::ANY_MEMBER);
//  clientReq->setSourceMemberId(idgs::pb::CLIENT_MEMBER);
//  clientReq->setPayload(adminReq);
//
//  idgs::client::ClientActorMessagePtr clientResp;
//  code = client->sendRecv(clientReq, clientResp);
//
//  idgs::admin::pb::AdminResponse adminResp;
//  clientResp->parsePayload(&adminResp);
//
//  auto & attr = adminResp.module_op_response(0).attributes(0);
//  ASSERT_EQ("rdd", attr.module_name());
//  ASSERT_EQ(idgs::admin::pb::Error, attr.status());
//  ASSERT_EQ("can not find rdd actor for name : TPCH_Q6_RDD", attr.value());
//}
//
//TEST(rdd_destroy, varifer_rdd_actor_is_free) {
//  ResultCode code = RC_SUCCESS;
//  auto client = getTcpClientPool().getTcpClient(code);
//  ASSERT_EQ(RC_SUCCESS, code);
//
//  std::shared_ptr<idgs::admin::pb::AdminRequest> adminReq = std::make_shared<idgs::admin::pb::AdminRequest>();
//  auto moduleRequest = adminReq->add_module_op_request();
//  moduleRequest->set_module_name("cluster");
//  moduleRequest->add_attributes()->set_attribute("actors.stateful_actors");
//
//  idgs::client::ClientActorMessagePtr clientReq = std::make_shared<idgs::client::ClientActorMessage>();
//  clientReq->setOperationName("get");
//  clientReq->setDestActorId(ADMIN_ACTOR_ID);
//  clientReq->setSourceActorId("client_actor_id");
//  clientReq->setDestMemberId(idgs::pb::ANY_MEMBER);
//  clientReq->setSourceMemberId(idgs::pb::CLIENT_MEMBER);
//  clientReq->setPayload(adminReq);
//
//  idgs::client::ClientActorMessagePtr clientResp;
//  code = client->sendRecv(clientReq, clientResp);
//
//  idgs::admin::pb::AdminResponse adminResp;
//  clientResp->parsePayload(&adminResp);
//
//  auto & attr = adminResp.module_op_response(0).attributes(0);
//  ASSERT_EQ("cluster", attr.module_name());
//  ASSERT_EQ(idgs::admin::pb::Success, attr.status());
//
//  string payload = clientResp->getRpcMessage()->payload();
//  ASSERT_EQ(string::npos, payload.find("idgs::rdd::StoreDelegateRddActor"));
//  ASSERT_EQ(string::npos, payload.find("idgs::rdd::RddActor"));
//  ASSERT_EQ(string::npos, payload.find("idgs::rdd::StoreDelegateRddPartition"));
//  ASSERT_EQ(string::npos, payload.find("idgs::rdd::RddPartition"));
//}
