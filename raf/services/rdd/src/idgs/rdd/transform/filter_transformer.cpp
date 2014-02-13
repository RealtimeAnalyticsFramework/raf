
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "filter_transformer.h"

using namespace std;
using namespace idgs::actor;
using namespace idgs::rdd::pb;
using namespace idgs::expr;

namespace idgs {
namespace rdd {
namespace transform {

FilterTransformer::FilterTransformer() {
}

FilterTransformer::~FilterTransformer() {
}

RddResultCode FilterTransformer::transform(const ActorMessagePtr& msg, const vector<BaseRddPartition*>& input,
    RddPartition* output) {
  if (input.size() != 1) {
    LOG(ERROR)<<"filter transformer '" << output->getRddName() << "' with partition " << output->getPartition() << " must have one input RDD, caused by code " << RRC_INVALID_RDD_INPUT;
    return RRC_INVALID_RDD_INPUT;
  }

  if (input[0]->empty()) {
    DVLOG(3) << "filter transformer '" << output->getRddName() << "' partition[" << output->getPartition() << "] have no data.";
    return RRC_SUCCESS;
  }

  RddResultCode code = RRC_SUCCESS;

  ExpressionContext ctx;
  ctx.setInPartition(input[0]);
  ctx.setOutPartition(output);
  auto& exprOp = output->getExprOperator(0);

  input[0]->foreach([&exprOp, &ctx] (const PbMessagePtr& key, const PbMessagePtr& value) {
        ctx.setKeyValue(&key, &value);
        exprOp(&ctx);
      });

  return code;
}

} // namespace transform
} // namespace rdd
} // namespace idgs 
