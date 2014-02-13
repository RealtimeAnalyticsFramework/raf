
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "gtest/gtest.h"
#include "idgs/client/rdd/rdd_client.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"
#include "protobuf/message_helper.h"
#include "idgs/store/data_map.h"

using namespace std;
using namespace idgs;
using namespace idgs::util;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;
using namespace protobuf;

TEST(lookup_action_test, lookup_action) {

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

  /// do action
  ActionRequestPtr action_request(new ActionRequest);
  ActionResponsePtr action_response(new ActionResponse);
  ActionResultPtr action_result(new idgs::rdd::pb::LookupActionResult);
  action_request->set_action_id("test_lookup_action");
  action_request->set_action_op_name(idgs::rdd::LOOKUP_ACTION);
  const std::string& key_type = "idgs.sample.ssb.pb.LineOrderKey";
  action_request->set_param(key_type);
  /// lo_orderkey=5952 and lo_linenumber=3
  auto key = ::idgs::util::singleton<MessageHelper>::getInstance().createMessage(key_type);
  key->GetReflection()->SetUInt64(key.get(), key->GetDescriptor()->FindFieldByName("lo_orderkey"), 5952);
  key->GetReflection()->SetUInt64(key.get(), key->GetDescriptor()->FindFieldByName("lo_linenumber"), 3);
  /// construct attachment
  AttachMessage attach;
  attach[ACTION_PARAM] = key;
  
  /// send Action
  singleton<RddClient>::getInstance().sendAction(action_request, action_response, action_result, delegate_actor_id, attach);
  LookupActionResult* result = dynamic_cast<LookupActionResult*>(action_result.get());
  auto str_value = result->values(0);

  /// dynamic deserialized protobuf message
  auto value = ::idgs::util::singleton<MessageHelper>::getInstance().createMessage("idgs.sample.ssb.pb.LineOrder");
  ProtoSerdes<DEFAULT_PB_SERDES>::deserialize(str_value, value.get());

  /// 5952|3|29|71|1|19970414|3-MEDIUM|0|43|4175601|12862499|1|4133844|58264|1|19970606|MAIL|
//  ASSERT_EQ(29, value->GetReflection()->GetUInt64(*value, value->GetDescriptor()->FindFieldByName("lo_custkey")));
  ASSERT_EQ(19970414, value->GetReflection()->GetUInt64(*value, value->GetDescriptor()->FindFieldByName("lo_orderdate")));
  cout << "key:" << key->DebugString() << endl;
  cout << "value:" << value->DebugString() << endl;
}
