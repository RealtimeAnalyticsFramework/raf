
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "count_action.h"

#include "idgs/rdd/pb/rdd_action.pb.h"

#include "idgs/util/utillity.h"

namespace idgs {
namespace rdd {
namespace action {

CountAction::CountAction() {
}

CountAction::~CountAction() {
}

pb::RddResultCode CountAction::action(ActionContext* ctx, const BaseRddPartition* input) {
  protobuf::PbVariant result;
  if (!ctx->hasFilterExpr()) {
    result = input->valueSize();
  } else {
    size_t count = 0;
    input->foreach([&count, &ctx] (const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
      ctx->setKeyValue(&key, &value);
      if (ctx->evaluateFilterExpr()) {
        ++ count;
      }
    });

    result = count;
  }

  ctx->addPartitionResult(result);
  return pb::RRC_SUCCESS;
}

pb::RddResultCode CountAction::aggregate(ActionContext* ctx) {
  auto& results = ctx->getAggregateResult();
  size_t size = 0;
  for (size_t partition = 0; partition < results.size(); ++ partition) {
    if (results[partition].empty()) {
      continue;
    }

    size_t pSize = 0;
    ResultCode code = sys::convert<size_t>(results[partition][0], pSize);
    if (code != RC_SUCCESS) {
      LOG(ERROR)<< "cannot convert \"" << results[partition][0] << "\" to a number.";
      return pb::RRC_DATA_CONVERT_ERROR;
    }

    size += pSize;
  }

  std::shared_ptr<pb::CountActionResult> result = std::make_shared<pb::CountActionResult>();
  result->set_size(size);
  ctx->setActionResult(result);

  return pb::RRC_SUCCESS;
}

} // namespace action
} // namespace rdd
} // namespace idgs 
