
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $

#include "collect_action.h"

#include "idgs/rdd/pb/rdd_action.pb.h"

using namespace std;
using namespace protobuf;
using namespace idgs::actor;
using namespace idgs::rdd::pb;

namespace idgs {
namespace rdd {
namespace action {

CollectAction::CollectAction() {

}

CollectAction::~CollectAction() {

}

RddResultCode CollectAction::action(ActionContext* ctx, const BaseRddPartition* input) {
  if (!input->empty()) {
    auto limit = ctx->getLimit();
    auto& result = ctx->getPartitionResult();
    auto serdesMode = ctx->getSerdesMode();
    input->foreachGroup([&ctx, limit, &result, serdesMode] (const PbMessagePtr& key, const std::vector<PbMessagePtr>& values) {
      if (limit >= 0 && result.size() >= limit) {
        return;
      }

      KeyValuesPair pair;
      for(auto it = values.begin(); it != values.end(); ++ it) {
        auto value = * it;
        ctx->setKeyValue(&key, &value);
        if(ctx->evaluateFilterExpr()) {
          ProtoSerdesHelper::serialize(serdesMode, value.get(), pair.add_value());
        }
      }

      if (pair.value_size() > 0) {
        ProtoSerdesHelper::serialize(serdesMode, key.get(), pair.mutable_key());
        string str_pair;
        ProtoSerdesHelper::serialize(serdesMode, &pair, &str_pair);
        ctx->addPartitionResult(str_pair);
      }
    });
  }

  return RRC_SUCCESS;
}

RddResultCode CollectAction::aggregate(ActionContext* ctx) {
  shared_ptr<CollectActionResult> result = make_shared<CollectActionResult>();
  auto limit = ctx->getLimit();
  auto serdesMode = ctx->getSerdesMode();
  auto& input = ctx->getAggregateResult();
  for (auto itPpartition = input.begin(); itPpartition != input.end(); ++ itPpartition) {
    if (itPpartition->empty()) {
      continue;
    }

    for (auto itValue = itPpartition->begin(); itValue != itPpartition->end(); ++ itValue) {
      if (limit < 0 || limit > result->pair_size()) {
        ProtoSerdesHelper::deserialize(serdesMode, * itValue, result->add_pair());
      }
    }
  }

  ctx->setActionResult(result);

  return RRC_SUCCESS;
}

} /* namespace op */
} /* namespace rdd */
} /* namespace idgs */
