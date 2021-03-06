
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "gtest/gtest.h"
#include "idgs/client/rdd/rdd_client.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"

using namespace idgs;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;
using namespace protobuf;

TEST(collect_action_test_big_data, collect_action) {
  /// create store delegate

  RddClient client;
  ResultCode code = client.init("conf/client.conf");
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "init rdd client error, caused by " << getErrorDescription(code);
  }
  const std::string store_name = "ssb_lineorder";
  LOG(INFO)<< "create store delegate, store_name: " << store_name;
  DelegateRddRequestPtr request = std::make_shared<CreateDelegateRddRequest>();
  DelegateRddResponsePtr response = std::make_shared<CreateDelegateRddResponse>();
  request->set_schema_name("ssb");
  request->set_store_name(store_name);
  client.createStoreDelegateRDD(request, response);
  auto delegate_actor_id = response->rdd_id();
  LOG_IF(FATAL, delegate_actor_id.actor_id().empty() || delegate_actor_id.actor_id().compare("Unknown Actor") == 0) << delegate_actor_id.DebugString();
  /// sleep for a while
  sleep(5);

  /// do action
  ActionRequestPtr action_request = std::make_shared<ActionRequest>();
  ActionResponsePtr action_response = std::make_shared<ActionResponse>();
  ActionResultPtr action_result = std::make_shared<idgs::rdd::pb::CollectActionResult>();
  action_request->set_action_id("test_collect_action");
  action_request->set_action_op_name(idgs::rdd::COLLECT_ACTION);

  auto rc = client.sendAction(action_request, action_response, action_result, delegate_actor_id);
  ASSERT_EQ(RC_SUCCESS, rc);
  CollectActionResult* result = dynamic_cast<CollectActionResult*>(action_result.get());
  assert(result);

  auto& storeConfigWrapper = client.getStoreConfig(store_name);
  auto key = storeConfigWrapper->newKey();
  auto value = storeConfigWrapper->newValue();
  int32_t limit = 20;
  size_t size = 0;
  for (auto rt = result->pair().begin(); rt != result->pair().end(); ++ rt) {
    if (limit > 0) {
      ProtoSerdes<PB_BINARY>::deserialize(rt->key(), key.get());
      LOG(INFO) << "##################key###############";
      LOG(INFO) << key->ShortDebugString();
      LOG(INFO) << "##################values(size: " << rt->value_size() << ")###############";
      for(auto it = rt->value().begin(); it != rt->value().end(); ++it) {
        ProtoSerdes<PB_BINARY>::deserialize(*it, value.get());
        LOG(INFO) << value->ShortDebugString();
      }

      -- limit;
    }

    size += rt->value_size();
  }

  LOG(INFO) << "data size : " << size;

}
