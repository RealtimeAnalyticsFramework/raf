
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $

#include "sum_action.h"

#include "idgs/rdd/pb/rdd_action.pb.h"

#include "idgs/util/utillity.h"

namespace idgs {
namespace rdd {
namespace action {

SumAction::SumAction() {
}

SumAction::~SumAction() {
}

pb::RddResultCode SumAction::action(ActionContext* ctx, const BaseRddPartition* input) {
  double sum = 0;
  input->foreach([&sum, &ctx] (const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
    ctx->setKeyValue(&key, &value);
    if (ctx->evaluateFilterExpr()) {
      auto value = ctx->evaluateExpr();
      sum += (double) value;
    }
  });

  ctx->addPartitionResult(sum);
  return pb::RRC_SUCCESS;
}

pb::RddResultCode SumAction::aggregate(ActionContext* ctx) {
  auto results = ctx->getAggregateResult();
  double sum = 0;
  auto it = results.begin();
  for (; it != results.end(); ++ it) {
    if (!it->empty()) {
      double value = 0;
      ResultCode code = sys::convert<double>((* it)[0], value);
      if (code != RC_SUCCESS) {
        LOG(ERROR)<< "cannot convert \"" << (* it)[0] << "\" to a number.";
        return pb::RRC_DATA_CONVERT_ERROR;
      }

      sum += value;
    }
  }

  std::shared_ptr<pb::SumActionResult> result = std::make_shared<pb::SumActionResult>();
  result->set_total(sum);
  ctx->setActionResult(result);

  return pb::RRC_SUCCESS;
}

} /* namespace op */
} /* namespace rdd */
} /* namespace idgs */
