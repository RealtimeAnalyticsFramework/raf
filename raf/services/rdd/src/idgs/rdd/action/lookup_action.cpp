
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $

#include "lookup_action.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"
#include "protobuf/message_helper.h"
#include "idgs/store/data_map.h"

using namespace std;
using namespace protobuf;
using namespace idgs::rdd::pb;

namespace idgs {
namespace rdd {
namespace action {

LookupAction::LookupAction() {

}

LookupAction::~LookupAction() {

}

RddResultCode LookupAction::action(const idgs::actor::ActorMessagePtr& msg, const BaseRddPartition* input,
    std::vector<PbVariant>& output) {
  ActionRequest* payload = dynamic_cast<ActionRequest*>(msg->getPayload().get());
  const std::string& key_type = payload->param();
  auto key = ::idgs::util::singleton<MessageHelper>::getInstance().createMessage(key_type);
  if (!msg->parseAttachment(ACTION_PARAM, key.get())) {
    LOG(ERROR)<< "Not exists attachment: ReduceValueRequest";
    return RRC_INVALID_TRANSFORMER_PARAM;
  }
  auto values = input->getValue(key);
  string s;
  for (auto it = values.begin(); it != values.end(); ++it) {
    ProtoSerdes<DEFAULT_PB_SERDES>::serialize(it->get(), &s);
    output.push_back(s);
  }
  return RRC_SUCCESS;
} /// end without filter

RddResultCode LookupAction::aggregate(const idgs::actor::ActorMessagePtr& actionRequest, idgs::actor::ActorMessagePtr& actionResponse, const vector<vector<string>>& input) {
  shared_ptr<LookupActionResult> result(new LookupActionResult);
  for (auto i = input.begin(); i != input.end(); ++i) {
    if (i->empty()) { /// partition i, data not found
      continue;
    }
    for (auto j = i->begin(); j != i->end(); ++j) {
      result->add_values(*j);
    }
  }
  actionResponse->setAttachment(ACTION_RESULT, result);
  return RRC_SUCCESS;
}

} /* namespace op */
} /* namespace rdd */
} /* namespace idgs */
