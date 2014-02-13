
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include <fstream>
#include "gtest/gtest.h"
#include "idgs/cancelable_timer.h"
#include "idgs/util/utillity.h"
#include "idgs/client/rdd/rdd_client.h"
#include "idgs/rdd/pb/rdd_action.pb.h"
#include "idgs/tpc/tpc_svc_const.h"
#include "idgs/store/data_map.h"

using namespace std;
using namespace idgs;
using namespace idgs::pb;
using namespace idgs::util;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;
using namespace idgs::tpc;

std::shared_ptr<Expr> buildSsbQ1OrderFilterExpression() {
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

  return expression;
}

std::shared_ptr<Expr> buildSsbQ1DateFilterExpression() {
  shared_ptr<Expr> expression(new Expr);
  expression->set_type(EQ);

  Expr* elem1 = expression->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("d_year");

  Expr* elem2 = expression->add_expression();
  elem2->set_type(CONST);
  elem2->set_const_type(UINT32);
  elem2->set_value("1992");

  return expression;
}

std::shared_ptr<Expr> buildSsbQ1ActionExpression() {
  shared_ptr<Expr> expression(new Expr);
  expression->set_type(MULTIPLY);
  auto elem1 = expression->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("lo_extendedprice");

  auto elem2 = expression->add_expression();
  elem2->set_type(FIELD);
  elem2->set_value("lo_discount");

  return expression;
}

TEST(ssb_query, Q1) {
  TEST_TIMEOUT(60);

  int32_t totalCount = 100;
  char* q1Time = getenv("SSB_Q1_LOOP");
  if (q1Time) {
    totalCount = atoi(q1Time);
    if (totalCount < 0) {
      totalCount = 100;
    }
  }

  RddClient& client = singleton<RddClient>::getInstance();
  ResultCode code = client.init("samples/tpc-svc/conf/client.conf");
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in init client, cause by " << idgs::getErrorDescription(code);
    exit(1);
  }

  LOG(INFO) << "Create store delegate RDD for lineorder.";
  DelegateRddRequestPtr request(new CreateDelegateRddRequest);
  DelegateRddResponsePtr response(new CreateDelegateRddResponse);
  request->set_store_name("ssb_lineorder");
  request->set_rdd_name("ssb_lineorder");

  singleton<RddClient>::getInstance().createStoreDelegateRDD(request, response);
  auto orderDelegateRddID = response->rdd_id();

  LOG(INFO) << "Create store delegate RDD for date.";
  request->set_store_name("ssb_date");
  request->set_rdd_name("ssb_date");

  singleton<RddClient>::getInstance().createStoreDelegateRDD(request, response);
  auto dateDelegateRddID = response->rdd_id();
  sleep(3);

  LOG(INFO) << "Create RDD for SSBQ1.";

  RddRequestPtr ssbQ1Request(new CreateRddRequest);
  RddResponsePtr ssbQ1Response(new CreateRddResponse);

  ssbQ1Request->set_transformer_op_name(SSB_Q1_1_TRANSFORMER);

  auto inRddOrder = ssbQ1Request->add_in_rdd();
  inRddOrder->mutable_rdd_id()->set_actor_id(orderDelegateRddID.actor_id());
  inRddOrder->mutable_rdd_id()->set_member_id(orderDelegateRddID.member_id());
  inRddOrder->mutable_rdd_name()->append("ssb_lineorder");
  auto orderExp = buildSsbQ1OrderFilterExpression();
  inRddOrder->mutable_filter_expr()->CopyFrom(* orderExp);

  auto inRddDate = ssbQ1Request->add_in_rdd();
  inRddDate->mutable_rdd_id()->set_actor_id(dateDelegateRddID.actor_id());
  inRddDate->mutable_rdd_id()->set_member_id(dateDelegateRddID.member_id());
  inRddDate->mutable_rdd_name()->append("ssb_date");

  auto dateExp = buildSsbQ1DateFilterExpression();
  inRddDate->mutable_filter_expr()->CopyFrom(* dateExp);

  auto out = ssbQ1Request->mutable_out_rdd();
  out->set_rdd_name("SSB_Q1.1");
  out->set_data_type(ORDERED);
  out->set_key_type_name("idgs.sample.ssb.pb.LineOrderKey");
  out->set_value_type_name("idgs.sample.ssb.pb.LineOrder");

  client.createRdd(ssbQ1Request, ssbQ1Response);
  auto ssbQ1RddID = ssbQ1Response->rdd_id();
  sleep(5);

  LOG(INFO) << "Execute action of SSBQ1.";
  LOG(INFO) << "loop count: " << totalCount;

  unsigned long totalTime = 0, first_time = 0;
  double result = 0;

  for (int32_t i = 0; i < totalCount; ++ i) {
    auto start = idgs::sys::getCurrentTime();
    ActionRequestPtr actionRequest(new ActionRequest);
    ActionResponsePtr actionResponse(new ActionResponse);
    ActionResultPtr actionResult(new SumActionResult);

    actionRequest->set_action_id("100000");
    actionRequest->set_action_op_name(SSB_Q1_1_ACTION);

    client.sendAction(actionRequest, actionResponse, actionResult, ssbQ1RddID);

    ASSERT_EQ("100000", actionResponse->action_id());
    ASSERT_EQ(RRC_SUCCESS, actionResponse->result_code());

    auto end = idgs::sys::getCurrentTime();

    totalTime += (end - start);
    if (i == 0) {
      first_time = end - start;
      result = dynamic_cast<SumActionResult*>(actionResult.get())->total();
    }

    if (!i) {
      LOG(INFO) << "First action (filter transform is required) costs " << first_time << "ms.";
    } else if (!(i % totalCount)) {
      LOG(INFO) << "Q1 iteration: " << i << ", average latency: " << ((float) (totalTime - first_time) / (i - 1)) << "ms.";
    }
  }

  LOG(INFO) << "Result is : " << std::fixed << result;
  LOG(INFO) << "Except first time, the average time is " << ((float) (totalTime - first_time) / (totalCount - 1)) << "ms.";
  LOG(INFO) << "Total average time is " << ((float) totalTime / totalCount) << "ms.";

  {
  ofstream ofs("ssb-q1.1.txt");
  ofs << "first" << "," << "average" << endl;
  ofs << first_time << "," << ((float) (totalTime - first_time) / (totalCount - 1)) << endl;
  ofs.close();
  }
}
