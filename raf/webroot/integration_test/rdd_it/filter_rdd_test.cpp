
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

using namespace idgs;
using namespace idgs::pb;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;

namespace idgs {
namespace rdd {
namespace filter_test {

RddClient client;
ActorId delegateRddId, filterRddId;

}  // namespace filter_test
}  // namespace rdd
}  // namespace idgs

TEST(RDD_filter, create_delegate_rdd) {
  TEST_TIMEOUT(20);

  ResultCode code = idgs::rdd::filter_test::client.init("conf/client.conf");
  if (code != RC_SUCCESS) {
    exit(1);
  }

  LOG(INFO) << "create message of create store delegate";
  DelegateRddRequestPtr request = std::make_shared<CreateDelegateRddRequest>();
  DelegateRddResponsePtr response = std::make_shared<CreateDelegateRddResponse>();

  request->set_schema_name("tpch");
  request->set_store_name("Orders");
  request->set_rdd_name("Orders");

  idgs::rdd::filter_test::client.createStoreDelegateRDD(request, response);

  ASSERT_EQ(RRC_SUCCESS, response->result_code());
  idgs::rdd::filter_test::delegateRddId = response->rdd_id();
  ASSERT_NE(-3, idgs::rdd::filter_test::delegateRddId.member_id());
  ASSERT_NE("Unknown Actor", idgs::rdd::filter_test::delegateRddId.actor_id());

  LOG(INFO) << "delegate rdd member id : " << idgs::rdd::filter_test::delegateRddId.member_id()
            << ", actor id : " << idgs::rdd::filter_test::delegateRddId.actor_id();

  sleep(2);
}

TEST(RDD_filter, create_filter_rdd) {
  TEST_TIMEOUT(20);

  RddRequestPtr request = std::make_shared<CreateRddRequest>();
  RddResponsePtr response = std::make_shared<CreateRddResponse>();

  request->set_transformer_op_name(FILTER_TRANSFORMER);

  auto in = request->add_in_rdd();
  in->mutable_rdd_id()->set_member_id(idgs::rdd::filter_test::delegateRddId.member_id());
  in->mutable_rdd_id()->set_actor_id(idgs::rdd::filter_test::delegateRddId.actor_id());
  in->mutable_rdd_name()->append("Orders");

  auto out = request->mutable_out_rdd();
  out->set_rdd_name("FILTER_RDD");
  out->set_key_type_name("idgs.sample.tpch.pb.OrdersKey");
  out->set_value_type_name("idgs.sample.tpch.pb.Orders");

  idgs::rdd::filter_test::client.createRdd(request, response);

  ASSERT_EQ(RRC_SUCCESS, response->result_code());
  idgs::rdd::filter_test::filterRddId = response->rdd_id();
  ASSERT_NE(-3, idgs::rdd::filter_test::filterRddId.member_id());
  ASSERT_NE("Unknown Actor", idgs::rdd::filter_test::filterRddId.actor_id());

  LOG(INFO) << "filter rdd member id : " << idgs::rdd::filter_test::filterRddId.member_id()
      << ", actor id : "<< idgs::rdd::filter_test::filterRddId.actor_id();

  sleep(2);
}

TEST(RDD_filter, count_action) {
  TEST_TIMEOUT(20);

  LOG(INFO) << "create and send message of action";

  ActionRequestPtr request = std::make_shared<ActionRequest>();
  ActionResponsePtr response = std::make_shared<ActionResponse>();
  ActionResultPtr result = std::make_shared<CountActionResult>();

  request->set_action_id("110000");
  request->set_action_op_name(COUNT_ACTION);

  idgs::rdd::filter_test::client.sendAction(request, response, result, idgs::rdd::filter_test::filterRddId);

  ASSERT_EQ("110000", response->action_id());
  ASSERT_EQ(RRC_SUCCESS, response->result_code());

  size_t size = dynamic_cast<CountActionResult*>(result.get())->size();
  LOG(INFO) << "data size : " << size;
  ASSERT_EQ(1000, size);
}

TEST(RDD_filter, count_action_second_time) {
  TEST_TIMEOUT(20);

  RddRequestPtr request = std::make_shared<CreateRddRequest>();
  RddResponsePtr response = std::make_shared<CreateRddResponse>();

  request->set_transformer_op_name(FILTER_TRANSFORMER);

  auto in = request->add_in_rdd();
  in->mutable_rdd_id()->set_member_id(idgs::rdd::filter_test::filterRddId.member_id());
  in->mutable_rdd_id()->set_actor_id(idgs::rdd::filter_test::filterRddId.actor_id());
  in->mutable_rdd_name()->append("FILTER_RDD");

  auto out = request->mutable_out_rdd();
  out->set_rdd_name("FILTER_RDD_2nd");
  out->set_key_type_name("idgs.sample.tpch.pb.OrdersKey");
  out->set_value_type_name("idgs.sample.tpch.pb.Orders");

  idgs::rdd::filter_test::client.createRdd(request, response);

  auto rddid = response->rdd_id();

  sleep(3);

  LOG(INFO) << "create and send message of action";

  ActionRequestPtr actionRequest = std::make_shared<ActionRequest>();
  ActionResponsePtr actionResponse = std::make_shared<ActionResponse>();
  ActionResultPtr actionResult = std::make_shared<CountActionResult>();

  actionRequest->set_action_id("120000");
  actionRequest->set_action_op_name(COUNT_ACTION);

  idgs::rdd::filter_test::client.sendAction(actionRequest, actionResponse, actionResult, rddid);

  ASSERT_EQ("120000", actionResponse->action_id());
  ASSERT_EQ(RRC_SUCCESS, actionResponse->result_code());

  size_t size = dynamic_cast<CountActionResult*>(actionResult.get())->size();
  LOG(INFO) << "data size : " << size;
  ASSERT_EQ(1000, size);
}
