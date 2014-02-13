
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include "gtest/gtest.h"
#include <fstream>
#include "idgs/cancelable_timer.h"
#include "idgs/util/utillity.h"
#include "idgs/client/rdd/rdd_client.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"
#include "idgs/tpc/tpc_svc_const.h"
#include "idgs/store/data_map.h"

using namespace std;
using namespace idgs;
using namespace idgs::pb;
using namespace idgs::util;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;
using namespace idgs::tpc;

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

std::shared_ptr<Expr> buildTpchQ6ActionExpression() {
  shared_ptr<Expr> expression(new Expr);
  expression->set_type(MULTIPLY);
  auto elem1 = expression->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("l_extendedprice");

  auto elem2 = expression->add_expression();
  elem2->set_type(FIELD);
  elem2->set_value("l_discount");

  return expression;
}

TEST(tpch_query, Q6) {
  TEST_TIMEOUT(30);

  int32_t totalCount = 100;
  char* q6Time = getenv("TPCH_Q6_LOOP");
  if (q6Time) {
    totalCount = atoi(q6Time);
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

  DelegateRddRequestPtr delegateRddRequest(new CreateDelegateRddRequest);
  DelegateRddResponsePtr delegateRddResponse(new CreateDelegateRddResponse);
  delegateRddRequest->set_store_name("LineItem");

  singleton<RddClient>::getInstance().createStoreDelegateRDD(delegateRddRequest, delegateRddResponse);
  auto delegateRddID = delegateRddResponse->rdd_id();
//  sleep(2);

  RddRequestPtr filterRequest(new CreateRddRequest);
  RddResponsePtr filterResponse(new CreateRddResponse);
  filterRequest->set_transformer_op_name(FILTER_TRANSFORMER);

  auto in = filterRequest->add_in_rdd();
  in->mutable_rdd_id()->set_actor_id(delegateRddID.actor_id());
  in->mutable_rdd_id()->set_member_id(delegateRddID.member_id());

  auto out = filterRequest->mutable_out_rdd();
  out->set_rdd_name("TPCH_Q6_RDD");
  out->set_data_type(ORDERED);
  out->set_key_type_name("idgs.sample.tpch.pb.LineItemKey");
  out->set_value_type_name("idgs.sample.tpch.pb.LineItem");

  auto exp = buildTpchQ6FilterExpression();
  in->mutable_filter_expr()->CopyFrom(* exp);

  client.createRdd(filterRequest, filterResponse);
  auto filterRddID = filterResponse->rdd_id();
//  sleep(5);

  unsigned long totalTime = 0, first_time = 0;
  double result = 0;
  auto actionExp = buildTpchQ6ActionExpression();

  LOG(INFO) << "loop count: " << totalCount;
  for (int32_t i = 0; i < totalCount; ++ i) {
    auto start = idgs::sys::getCurrentTime();
    ActionRequestPtr actionRequest(new ActionRequest);
    ActionResponsePtr actionResponse(new ActionResponse);
    ActionResultPtr actionResult(new SumActionResult);

    actionRequest->set_action_id("100000");
    actionRequest->set_action_op_name(TPCH_Q6_ACTION);
    actionRequest->mutable_expression()->CopyFrom(* actionExp);

    client.sendAction(actionRequest, actionResponse, actionResult, filterRddID);

    ASSERT_EQ("100000", actionResponse->action_id());
    ASSERT_EQ(RRC_SUCCESS, actionResponse->result_code());

    auto end = sys::getCurrentTime();

    totalTime += (end - start);
    if (i == 0) {
      first_time = end - start;
      result = dynamic_cast<SumActionResult*>(actionResult.get())->total();
    }

    if (!i) {
      LOG(INFO) << "First action (filter transform is required) costs " << first_time << "ms.";
    } else if (!(i % 1000)) {
      LOG(INFO) << "Q6 iteration: " << i << ", average latency: " << ((float) (totalTime - first_time) / (i - 1)) << "ms.";
    }
  }

  LOG(INFO) << "Result is : " << std::fixed << result;
  LOG(INFO) << "Except first time, the average time is " << ((float) (totalTime - first_time) / (totalCount - 1)) << "ms.";
  LOG(INFO) << "Total average time is " << ((float) totalTime / totalCount) << "ms.";

  {
  ofstream ofs("tpch-q6.txt");
  ofs << "first" << "," << "average" << endl;
  ofs << first_time << "," << ((float) (totalTime - first_time) / (totalCount - 1)) << endl;
  ofs.close();
  }
}
