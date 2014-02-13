
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $

#include "collect_action.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/expr/expression_factory.h"
#include "idgs/rdd/pb/rdd_action.pb.h"

using namespace std;
using namespace protobuf;
using namespace idgs::actor;
using namespace idgs::expr;
using namespace idgs::rdd::pb;

namespace idgs {
namespace rdd {
namespace action {

CollectAction::CollectAction() {

}

CollectAction::~CollectAction() {

}

RddResultCode CollectAction::action(const idgs::actor::ActorMessagePtr& msg, const BaseRddPartition* input,
    std::vector<PbVariant>& output) {
  ActionRequest* payload = dynamic_cast<ActionRequest*>(msg->getPayload().get());
  if (payload->has_filter()) { /// with filter
    Expression* filter_expr = NULL;
    auto rc = ExpressionFactory::build(&filter_expr, payload->filter(), input->getKeyTemplate(),
        input->getValueTemplate());
    if (idgs::RC_SUCCESS != rc) {
      LOG(ERROR)<< "RDD " << input->getRddName() << " parse filter expression error, caused by " << idgs::getErrorDescription(rc);
      return RRC_NOT_SUPPORT;
    }
    ExpressionContext ctx;
    input->foreachGroup(
        [filter_expr, &output, &ctx] (const PbMessagePtr& key, const std::vector<PbMessagePtr>& values) {
          KeyValuesPair pair;
          ProtoSerdes<DEFAULT_PB_SERDES>::serialize(key.get(), pair.mutable_key());
          for(auto it = values.begin(); it != values.end(); ++it) {
            auto value = *it;
            ctx.setKeyValue(&key, &value);
            bool flag = (bool) filter_expr->evaluate(&ctx); /// key expression's value is true or false
            if(flag) {
              ProtoSerdes<DEFAULT_PB_SERDES>::serialize(value.get(), pair.add_value());
              VLOG(5) << pair.DebugString();
            }
          }
          if(pair.value_size() > 0) {
            string str_pair;
            ProtoSerdes<DEFAULT_PB_SERDES>::serialize(&pair, &str_pair);
            output.push_back(str_pair);
          }
        });
  } /// end with filter
  else { /// without filter
    input->foreachGroup([&output] (const PbMessagePtr& key, const std::vector<PbMessagePtr>& values) {
      KeyValuesPair pair;
      ProtoSerdes<DEFAULT_PB_SERDES>::serialize(key.get(), pair.mutable_key());
      for(auto it = values.begin(); it != values.end(); ++it) {
        auto value = *it;
        ProtoSerdes<DEFAULT_PB_SERDES>::serialize(value.get(), pair.add_value());
        VLOG(5) << pair.DebugString();
      }
      string str_pair;
      ProtoSerdes<DEFAULT_PB_SERDES>::serialize(&pair, &str_pair);
      output.push_back(str_pair);
    });
  } /// end witout filter
  return RRC_SUCCESS;
} /// end without filter

RddResultCode CollectAction::aggregate(const idgs::actor::ActorMessagePtr& actionRequest, idgs::actor::ActorMessagePtr& actionResponse, const vector<vector<string>>& input) {
  shared_ptr<CollectActionResult> result(new CollectActionResult);
  for (auto i = input.begin(); i != input.end(); ++i) {
    if (i->empty()) { /// partition i, data not found
      continue;
    }
    for (auto j = i->begin(); j != i->end(); ++j) {
      ProtoSerdes<DEFAULT_PB_SERDES>::deserialize(*j, result->add_pair());
    }
  }
  actionResponse->setAttachment(ACTION_RESULT, result);
  return RRC_SUCCESS;
}

} /* namespace op */
} /* namespace rdd */
} /* namespace idgs */
