
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "gtest/gtest.h"
#include "idgs/cancelable_timer.h"
#include "idgs/client/rdd/rdd_client.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"

using namespace std;
using namespace idgs;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;

namespace idgs {
namespace rdd {
namespace rdd_name_test {

RddClient client;

}  // namespace rdd_name_test
}  // namespace rdd
}  // namespace idgs


TEST(rdd, create_store_delegate) {
  TEST_TIMEOUT(30);

  string storeName = "ssb_lineorder", rddName = "ssb_lineorder_delegate";

  if (idgs::rdd::rdd_name_test::client.init("conf/client.conf") != RC_SUCCESS) {
    exit(1);
  }

  DelegateRddRequestPtr request = std::make_shared<CreateDelegateRddRequest>();
  DelegateRddResponsePtr response = std::make_shared<CreateDelegateRddResponse>();

  request->set_schema_name("ssb");
  request->set_store_name(storeName);
  request->set_rdd_name(rddName);

  idgs::rdd::rdd_name_test::client.createStoreDelegateRDD(request, response);

  EXPECT_EQ(response->result_code(), RRC_SUCCESS);
  ASSERT_NE(-3, response->rdd_id().member_id());
  ASSERT_NE("Unknown Actor", response->rdd_id().actor_id());

  LOG(INFO) << "store delegate rdd member id : " << response->rdd_id().member_id() << ", actor id : " << response->rdd_id().actor_id();

  sleep(2);
}

TEST(rdd, createRdd) {
  TEST_TIMEOUT(30);

  string delegateRddName = "ssb_lineorder_delegate", rddName = "ssb_lineorder_filter", transformerName = "FILTER_TRANSFORMER";

  RddRequestPtr request = std::make_shared<CreateRddRequest>();
  RddResponsePtr response = std::make_shared<CreateRddResponse>();

  request->set_transformer_op_name(transformerName);
  auto in = request->add_in_rdd();
  in->set_rdd_name(delegateRddName);

  auto out = request->mutable_out_rdd();
  out->set_rdd_name(rddName);
  out->set_key_type_name("idgs.sample.ssb.pb.LineOrderKey");
  out->set_value_type_name("idgs.sample.ssb.pb.LineOrder");

  idgs::rdd::rdd_name_test::client.createRdd(request, response);

  ASSERT_EQ(RRC_SUCCESS, response->result_code());
  ASSERT_NE(-3, response->rdd_id().member_id());
  ASSERT_NE("Unknown Actor", response->rdd_id().actor_id());

  LOG(INFO) << "rdd member id : " << response->rdd_id().member_id() << ", actor id : "<< response->rdd_id().actor_id();

  sleep(2);
}

TEST(rdd, action) {
  TEST_TIMEOUT(30);

  string actionId = "ssb_lineorder_action", actionName = COUNT_ACTION, rddName = "ssb_lineorder_filter";

  ActionRequestPtr request = std::make_shared<ActionRequest>();
  ActionResponsePtr response = std::make_shared<ActionResponse>();
  ActionResultPtr result = std::make_shared<CountActionResult>();

  request->set_action_id(actionId);
  request->set_action_op_name(actionName);
  request->set_rdd_name(rddName);

  idgs::rdd::rdd_name_test::client.sendAction(request, response, result);

  ASSERT_EQ(actionId, response->action_id());
  ASSERT_EQ(RRC_SUCCESS, response->result_code());

  auto size = dynamic_cast<CountActionResult*>(result.get())->size();
  LOG(INFO) << "================ action result ================";
  LOG(INFO) << "\tdata size : " << size;
  LOG(INFO) << "===============================================";

}
