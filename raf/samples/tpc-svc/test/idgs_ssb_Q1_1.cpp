
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
#include "idgs/expr/expression_helper.h"

using namespace std;
using namespace idgs;
using namespace idgs::pb;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;
using namespace idgs::tpc;

std::shared_ptr<Expr> buildSsbQ1OrderFilterExpression() {
  shared_ptr<Expr> expression(AND(GE(FIELD("lo_discount"), CONST("1", DOUBLE)),
                              LE(FIELD("lo_discount"), CONST("3", DOUBLE)),
                              LT(FIELD("lo_quantity"), CONST("25", DOUBLE))
                             ));

  return expression;
}

std::shared_ptr<Expr> buildSsbQ1DateFilterExpression() {
  shared_ptr<Expr> expression(EQ(FIELD("d_year"), CONST("1992", UINT32)));
  return expression;
}

std::shared_ptr<Expr> buildSsbQ1ActionExpression() {
  shared_ptr<Expr> expression(MULTIPLY(FIELD("lo_extendedprice"), FIELD("lo_discount")));
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

  RddClient client;
  ResultCode code = client.init("conf/client.conf");
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in init client, cause by " << idgs::getErrorDescription(code);
    exit(1);
  }

  LOG(INFO) << "Create store delegate RDD for lineorder.";
  DelegateRddRequestPtr request = std::make_shared<CreateDelegateRddRequest>();
  DelegateRddResponsePtr response = std::make_shared<CreateDelegateRddResponse>();
  request->set_schema_name("ssb");
  request->set_store_name("ssb_lineorder");
  request->set_rdd_name("ssb_lineorder");

  client.createStoreDelegateRDD(request, response);
  auto orderDelegateRddID = response->rdd_id();

  LOG(INFO) << "Create store delegate RDD for date.";
  request->set_store_name("ssb_date");
  request->set_rdd_name("ssb_date");

  client.createStoreDelegateRDD(request, response);
  auto dateDelegateRddID = response->rdd_id();
  sleep(3);

  LOG(INFO) << "Create RDD for SSBQ1.";

  RddRequestPtr ssbQ1Request = std::make_shared<CreateRddRequest>();
  RddResponsePtr ssbQ1Response = std::make_shared<CreateRddResponse>();

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
  out->set_key_type_name("idgs.sample.ssb.pb.LineOrderKey");
  out->set_value_type_name("idgs.sample.ssb.pb.LineOrder");
  out->set_input_sync(false);

  client.createRdd(ssbQ1Request, ssbQ1Response);
  auto ssbQ1RddID = ssbQ1Response->rdd_id();
  sleep(5);

  LOG(INFO) << "Execute action of SSBQ1.";
  LOG(INFO) << "loop count: " << totalCount;

  unsigned long totalTime = 0, first_time = 0;
  double result = 0;

  for (int32_t i = 0; i < totalCount; ++ i) {
    auto start = idgs::sys::getCurrentTime();
    ActionRequestPtr actionRequest = std::make_shared<ActionRequest>();
    ActionResponsePtr actionResponse = std::make_shared<ActionResponse>();
    ActionResultPtr actionResult = std::make_shared<SumActionResult>();

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
