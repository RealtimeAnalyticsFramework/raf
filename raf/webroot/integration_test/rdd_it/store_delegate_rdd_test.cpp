
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
namespace delegate_test {

RddClient client;
ActorId delegateRddId;

}  // namespace delegate_test
}  // namespace rdd
}  // namespace idgs


TEST(rdd_store_delegate, create) {
  TEST_TIMEOUT(8);

  if (idgs::rdd::delegate_test::client.init("conf/client.conf") != RC_SUCCESS) {
    exit(1);
  }

  LOG(INFO) << "create message of create store delegate";
  DelegateRddRequestPtr request = std::make_shared<CreateDelegateRddRequest>();
  DelegateRddResponsePtr response = std::make_shared<CreateDelegateRddResponse>();

  request->set_schema_name("tpch");
  request->set_store_name("Orders");

  idgs::rdd::delegate_test::client.createStoreDelegateRDD(request, response);

  LOG(INFO) << "test result";
  idgs::rdd::delegate_test::delegateRddId = response->rdd_id();
  LOG(INFO) << "delegate rdd id : " << idgs::rdd::delegate_test::delegateRddId.DebugString();
}

TEST(rdd_store_delegate, action) {
  TEST_TIMEOUT(8);

  sleep(3);

  ActionRequestPtr request = std::make_shared<ActionRequest>();
  ActionResponsePtr response = std::make_shared<ActionResponse>();
  ActionResultPtr result = std::make_shared<CountActionResult>();

  request->set_action_id("10000000");
  request->set_action_op_name(COUNT_ACTION);

  idgs::rdd::delegate_test::client.sendAction(request, response, result, idgs::rdd::delegate_test::delegateRddId);

  ASSERT_EQ("10000000", response->action_id());
  ASSERT_EQ(RRC_SUCCESS, response->result_code());

  auto size = dynamic_cast<CountActionResult*>(result.get())->size();
  EXPECT_EQ(1000, size);
  LOG(INFO) << "================ action result ================";
  LOG(INFO) << "\tdata size : " << size;
  LOG(INFO) << "===============================================";
}
