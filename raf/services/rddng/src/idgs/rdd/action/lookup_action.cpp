
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

RddResultCode LookupAction::action(ActionContext* ctx, const BaseRddPartition* input) {
  idgs::actor::PbMessagePtr key(ctx->getKeyTemplate()->New());
  if (!ctx->getActionParam(ACTION_PARAM, key.get())) {
    return RRC_INVALID_ACTION_PARAM;
  }

  auto values = input->getValue(key);
  string s;
  auto serdesMode = ctx->getSerdesMode();
  for (auto it = values.begin(); it != values.end(); ++ it) {
    ctx->setKeyValue(&key, &(* it));
    if (ctx->evaluateFilterExpr()) {
      ProtoSerdesHelper::serialize(serdesMode, it->get(), &s);
      ctx->addPartitionResult(s);
    }
  }

  return RRC_SUCCESS;
}

RddResultCode LookupAction::aggregate(ActionContext* ctx) {
  shared_ptr<LookupActionResult> result = make_shared<LookupActionResult>();
  auto& input = ctx->getAggregateResult();
  for (auto itPpartition = input.begin(); itPpartition != input.end(); ++ itPpartition) {
    if (itPpartition->empty()) {
      continue;
    }

    for (auto itValue = itPpartition->begin(); itValue != itPpartition->end(); ++itValue) {
      result->add_values(* itValue);
    }
  }

  ctx->setActionResult(result);

  return RRC_SUCCESS;
}

} /* namespace op */
} /* namespace rdd */
} /* namespace idgs */
