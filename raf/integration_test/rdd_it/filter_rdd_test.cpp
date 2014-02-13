
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
//#if defined(__GNUC__) || defined(__clang__)
//#include "idgs_gch.h"
//#endif // GNUC_ $

#include "gtest/gtest.h"
#include "idgs/cancelable_timer.h"
#include "idgs/client/rdd/rdd_client.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"
#include "idgs/store/data_map.h"

using namespace idgs;
using namespace idgs::pb;
using namespace idgs::util;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;

ActorId delegateRddId, filterRddId;

TEST(RDD_filter, create_delegate_rdd) {
  TEST_TIMEOUT(20);

  ResultCode code = singleton<RddClient>::getInstance().init("integration_test/rdd_it/client.conf");
  if (code != RC_SUCCESS) {
    exit(1);
  }

  LOG(INFO) << "create message of create store delegate";
  DelegateRddRequestPtr request(new CreateDelegateRddRequest);
  DelegateRddResponsePtr response(new CreateDelegateRddResponse);

  request->set_store_name("Orders");
  request->set_rdd_name("Orders");

  singleton<RddClient>::getInstance().createStoreDelegateRDD(request, response);

  ASSERT_EQ(RRC_SUCCESS, response->result_code());
  delegateRddId = response->rdd_id();
  ASSERT_NE(-3, delegateRddId.member_id());
  ASSERT_NE("Unknown Actor", delegateRddId.actor_id());

  LOG(INFO) << "delegate rdd member id : " << delegateRddId.member_id() << ", actor id : " << delegateRddId.actor_id();

  sleep(2);
}

TEST(RDD_filter, create_filter_rdd) {
  TEST_TIMEOUT(20);

  RddRequestPtr request(new CreateRddRequest);
  RddResponsePtr response(new CreateRddResponse);

  request->set_transformer_op_name(FILTER_TRANSFORMER);

  auto in = request->add_in_rdd();
  in->mutable_rdd_id()->set_member_id(delegateRddId.member_id());
  in->mutable_rdd_id()->set_actor_id(delegateRddId.actor_id());
  in->mutable_rdd_name()->append("Orders");

  auto out = request->mutable_out_rdd();
  out->set_rdd_name("FILTER_RDD");
  out->set_data_type(ORDERED);
  out->set_key_type_name("idgs.sample.tpch.pb.OrdersKey");
  out->set_value_type_name("idgs.sample.tpch.pb.Orders");

  singleton<RddClient>::getInstance().createRdd(request, response);

  ASSERT_EQ(RRC_SUCCESS, response->result_code());
  filterRddId = response->rdd_id();
  ASSERT_NE(-3, filterRddId.member_id());
  ASSERT_NE("Unknown Actor", filterRddId.actor_id());

  LOG(INFO) << "filter rdd member id : " << filterRddId.member_id() << ", actor id : "<< filterRddId.actor_id();

  sleep(2);
}

TEST(RDD_filter, count_action) {
  TEST_TIMEOUT(20);

  LOG(INFO) << "create and send message of action";

  ActionRequestPtr request(new ActionRequest);
  ActionResponsePtr response(new ActionResponse);
  ActionResultPtr result(new CountActionResult);

  request->set_action_id("110000");
  request->set_action_op_name(COUNT_ACTION);

  singleton<RddClient>::getInstance().sendAction(request, response, result, filterRddId);

  ASSERT_EQ("110000", response->action_id());
  ASSERT_EQ(RRC_SUCCESS, response->result_code());

  size_t size = dynamic_cast<CountActionResult*>(result.get())->size();
  LOG(INFO) << "data size : " << size;
  ASSERT_EQ(1000, size);
}

TEST(RDD_filter, count_action_second_time) {
  TEST_TIMEOUT(20);

  RddRequestPtr request(new CreateRddRequest);
  RddResponsePtr response(new CreateRddResponse);

  request->set_transformer_op_name(FILTER_TRANSFORMER);

  auto in = request->add_in_rdd();
  in->mutable_rdd_id()->set_member_id(filterRddId.member_id());
  in->mutable_rdd_id()->set_actor_id(filterRddId.actor_id());
  in->mutable_rdd_name()->append("FILTER_RDD");

  auto out = request->mutable_out_rdd();
  out->set_rdd_name("FILTER_RDD_2nd");
  out->set_data_type(ORDERED);
  out->set_key_type_name("idgs.sample.tpch.pb.OrdersKey");
  out->set_value_type_name("idgs.sample.tpch.pb.Orders");

  singleton<RddClient>::getInstance().createRdd(request, response);

  auto rddid = response->rdd_id();

  sleep(3);

  LOG(INFO) << "create and send message of action";

  ActionRequestPtr actionRequest(new ActionRequest);
  ActionResponsePtr actionResponse(new ActionResponse);
  ActionResultPtr actionResult(new CountActionResult);

  actionRequest->set_action_id("120000");
  actionRequest->set_action_op_name(COUNT_ACTION);

  singleton<RddClient>::getInstance().sendAction(actionRequest, actionResponse, actionResult, rddid);

  ASSERT_EQ("120000", actionResponse->action_id());
  ASSERT_EQ(RRC_SUCCESS, actionResponse->result_code());

  size_t size = dynamic_cast<CountActionResult*>(actionResult.get())->size();
  LOG(INFO) << "data size : " << size;
  ASSERT_EQ(1000, size);
}
