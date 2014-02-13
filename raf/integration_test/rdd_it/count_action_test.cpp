
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "gtest/gtest.h"
#include "idgs/client/rdd/rdd_client.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"
#include "idgs/store/data_map.h"

using namespace idgs;
using namespace idgs::pb;
using namespace idgs::util;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;

TEST(count_action_test, count) {
  /// create store delegate
  ResultCode code = singleton<RddClient>::getInstance().init("integration_test/rdd_it/client.conf");
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "init rdd client error, caused by " << getErrorDescription(code);
  }
  const std::string store_name = "ssb_lineorder";
  LOG(INFO)<< "create store delegate, store_name: " << store_name;
  DelegateRddRequestPtr request(new CreateDelegateRddRequest);
  DelegateRddResponsePtr response(new CreateDelegateRddResponse);
  request->set_store_name(store_name);
  singleton<RddClient>::getInstance().createStoreDelegateRDD(request, response);
  auto delegate_actor_id = response->rdd_id();
  LOG_IF(FATAL, delegate_actor_id.actor_id().empty() || delegate_actor_id.actor_id().compare("Unknown Actor") == 0) << delegate_actor_id.DebugString();
  /// sleep for a while
  sleep(5);

  /// do count action
  {
    ActionRequestPtr action_request(new ActionRequest);
    ActionResponsePtr action_response(new ActionResponse);
    ActionResultPtr action_result(new idgs::rdd::pb::CountActionResult);

    action_request->set_action_id("test_count_action1");
    action_request->set_action_op_name(idgs::rdd::COUNT_ACTION);
    auto rc  = singleton<RddClient>::getInstance().sendAction(action_request, action_response, action_result, delegate_actor_id);
    ASSERT_EQ(RC_SUCCESS, rc);
    CountActionResult* result = dynamic_cast<CountActionResult*>(action_result.get());
    auto count = result->size();
    LOG(INFO) << "count size: " << count;
    /// compare
    ASSERT_EQ(6005, result->size());
  }

  /// do count action with filter
  {
    ActionRequestPtr action_request(new ActionRequest);
    ActionResponsePtr action_response(new ActionResponse);
    ActionResultPtr action_result(new idgs::rdd::pb::CountActionResult);
    /// lo_orderkey ==10
    auto filter_expr = action_request->mutable_filter();
    filter_expr->set_type(EQ);
    /// field expr: lo_orderkey
    auto expr = filter_expr->add_expression();
    expr->set_type(FIELD);
    expr->set_value("lo_orderkey");
    /// const expr: 10
    expr = filter_expr->add_expression();
    expr->set_type(CONST);
    expr->set_const_type(UINT64);
    expr->set_value("1");

    action_request->set_action_id("test_count_action3");
    action_request->set_action_op_name(idgs::rdd::COUNT_ACTION);
    auto rc  = singleton<RddClient>::getInstance().sendAction(action_request, action_response, action_result, delegate_actor_id);
    ASSERT_EQ(RC_SUCCESS, rc);
    CountActionResult* result = dynamic_cast<CountActionResult*>(action_result.get());
    auto count = result->size();
    LOG(INFO) << "count size: " << count;
    /// compare
    ASSERT_EQ(6, result->size());
  }
}

