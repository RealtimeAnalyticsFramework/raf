
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
#include "idgs/expr/expression_helper.h"

using namespace std;
using namespace idgs;
using namespace idgs::pb;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;
using namespace idgs::tpc;

std::shared_ptr<Expr> buildTpchQ6FilterExpression() {
  auto expr = AND(GE(FIELD("l_shipdate"), CONST("1994-01-01")),
                    LT(FIELD("l_shipdate"), CONST("1995-01-01")),
                    GE(FIELD("l_discount"), CONST("0.05", DOUBLE)),
                    LE(FIELD("l_discount"), CONST("0.07", DOUBLE)),
                    LT(FIELD("l_quantity"), CONST("24", DOUBLE))
                   );
  shared_ptr<Expr> expression(expr);

  return expression;
}

std::shared_ptr<Expr> buildTpchQ6ActionExpression() {
  auto expr = MULTIPLY(FIELD("l_extendedprice"), FIELD("l_discount"));
  shared_ptr<Expr> expression(expr);

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

  RddClient client;
  ResultCode code = client.init("conf/client.conf");
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in init client, cause by " << idgs::getErrorDescription(code);
    exit(1);
  }

  DelegateRddRequestPtr delegateRddRequest = std::make_shared<CreateDelegateRddRequest>();
  DelegateRddResponsePtr delegateRddResponse = std::make_shared<CreateDelegateRddResponse>();
  delegateRddRequest->set_schema_name("tpch");
  delegateRddRequest->set_store_name("LineItem");

  client.createStoreDelegateRDD(delegateRddRequest, delegateRddResponse);
  auto delegateRddID = delegateRddResponse->rdd_id();
//  sleep(2);

  RddRequestPtr filterRequest = std::make_shared<CreateRddRequest>();
  RddResponsePtr filterResponse = std::make_shared<CreateRddResponse>();
  filterRequest->set_transformer_op_name(FILTER_TRANSFORMER);

  auto in = filterRequest->add_in_rdd();
  in->mutable_rdd_id()->set_actor_id(delegateRddID.actor_id());
  in->mutable_rdd_id()->set_member_id(delegateRddID.member_id());

  auto out = filterRequest->mutable_out_rdd();
  out->set_rdd_name("TPCH_Q6_RDD");
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
    ActionRequestPtr actionRequest = std::make_shared<ActionRequest>();
    ActionResponsePtr actionResponse = std::make_shared<ActionResponse>();
    ActionResultPtr actionResult = std::make_shared<SumActionResult>();

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
