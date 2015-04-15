
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "reducebykey_transformer.h"

#include "idgs/rdd/op/reduce_operator.h"

using namespace std;
using namespace idgs::actor;
using namespace idgs::rdd::op;

namespace idgs {
namespace rdd {
namespace transform {

ReduceByKeyTransformer::ReduceByKeyTransformer() {

}

ReduceByKeyTransformer::~ReduceByKeyTransformer() {
}

pb::RddResultCode ReduceByKeyTransformer::transform(TransformerContext* ctx, const BaseRddPartition* input, PairRddPartition* output) {
  ReduceOperator* reduceOp = dynamic_cast<ReduceOperator*>(ctx->getRddOperator());
  size_t reduceSize = reduceOp->options.size();
  auto exprCtx = ctx->getExpressionContext();

  input->foreachGroup([reduceOp, reduceSize, &ctx, &exprCtx, &output] (const PbMessagePtr& key, const vector<PbMessagePtr>& values) {
    PbMessagePtr outkey, outvalue;

    auto& reduceOptions = ctx->getReduceOption(key, reduceSize);
    auto it = values.begin();
    for (; it != values.end(); ++ it) {
      exprCtx->setKeyValue(&key, &(* it));
      exprCtx->setOutputKeyValue(&outkey, &outvalue);

      if (reduceOp->evaluate(exprCtx)) {
        for (int32_t i = 0; i < reduceSize; ++ i) {
          reduceOp->options[i]->reduce(exprCtx, reduceOptions[i]);
        }
      }
    }

    if (outkey && outvalue) {
      output->put(outkey, outvalue);
    }
  });

  return pb::RRC_SUCCESS;
}

pb::RddResultCode ReduceByKeyTransformer::transform(TransformerContext* ctx, PairRddPartition* output) {
  ReduceOperator* reduceOp = dynamic_cast<ReduceOperator*>(ctx->getRddOperator());
  auto exprCtx = ctx->getExpressionContext();
  if (reduceOp->evaluate(exprCtx)) {
    size_t reduceSize = reduceOp->options.size();

    auto& outkey = * exprCtx->getOutputKey();
    auto& reduceOptions = ctx->getReduceOption(outkey, reduceSize);

    for (int32_t i = 0; i < reduceSize; ++ i) {
      reduceOp->options[i]->reduce(exprCtx, reduceOptions[i]);
    }

    output->put(outkey, * exprCtx->getOutputValue(), true);
  }

  return pb::RRC_SUCCESS;
}

} // namespace transform
} // namespace rdd
} // namespace idgs

