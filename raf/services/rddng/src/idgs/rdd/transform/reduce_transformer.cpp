
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "reduce_transformer.h"

#include "idgs/rdd/op/reduce_operator.h"

using namespace idgs::actor;
using namespace idgs::rdd::op;
using namespace idgs::rdd::pb;

namespace idgs {
namespace rdd {
namespace transform {

ReduceTransformer::ReduceTransformer() {

}

ReduceTransformer::~ReduceTransformer() {
}

RddResultCode ReduceTransformer::transform(TransformerContext* ctx, const BaseRddPartition* input, PairRddPartition* output) {
  ReduceOperator* reduceOp = dynamic_cast<ReduceOperator*>(ctx->getRddOperator());
  size_t reduceSize = reduceOp->options.size();
  auto exprCtx = ctx->getExpressionContext();
  idgs::pb::Integer reduceKey;
  reduceKey.set_value(1);

  PbMessagePtr outkey, outvalue;
  if (!input->empty()) {
    input->foreach([reduceOp, reduceSize, &ctx, &exprCtx, reduceKey, &outkey, &outvalue, input] (const PbMessagePtr& key, const PbMessagePtr& value) {
      exprCtx->setKeyValue(&key, &value);
      exprCtx->setOutputKeyValue(&outkey, &outvalue);

      if (reduceOp->evaluate(exprCtx)) {
        outkey->CopyFrom(reduceKey);
        auto& reduceOptions = ctx->getReduceOption(outkey, reduceSize);

        for (int32_t i = 0; i < reduceSize; ++ i) {
          reduceOp->options[i]->reduce(exprCtx, reduceOptions[i]);
        }
      }
    });
  }

  if (!outkey && !outvalue) {
    outkey.reset(output->getKeyTemplate()->New());
    outkey->CopyFrom(reduceKey);
    outvalue.reset(output->getValueTemplate()->New());
  }

  output->processRow(outkey, outvalue, true);

  return RRC_SUCCESS;
}

RddResultCode ReduceTransformer::transform(TransformerContext* ctx, PairRddPartition* output) {
  op::ReduceOperator* reduceOp = dynamic_cast<op::ReduceOperator*>(ctx->getRddOperator());
  auto exprCtx = ctx->getExpressionContext();
  if (reduceOp->evaluate(exprCtx)) {
    idgs::pb::Integer reduceKey;
    reduceKey.set_value(1);

    auto& outkey = * exprCtx->getOutputKey();
    outkey->CopyFrom(reduceKey);

    size_t reduceSize = reduceOp->options.size();
    auto& reduceOptions = ctx->getReduceOption(outkey, reduceSize);

    for (int32_t i = 0; i < reduceSize; ++ i) {
      reduceOp->options[i]->reduce(exprCtx, reduceOptions[i]);
    }

    output->processRow(outkey, * exprCtx->getOutputValue(), true);
  }

  return RRC_SUCCESS;
}

RddResultCode ReduceTransformer::aggregate(TransformerContext* ctx, PairRddPartition* output) {
  ReduceOperator* reduceOp = dynamic_cast<ReduceOperator*>(ctx->getRddOperator());
  size_t reduceSize = reduceOp->options.size();

  auto exprCtx = ctx->getExpressionContext();
  auto& outkey = * exprCtx->getOutputKey();
  auto& outvalue = * exprCtx->getOutputValue();

  auto& reduceOptions = ctx->getReduceOption(outkey, reduceSize);

  for (int32_t i = 0; i < reduceSize; ++ i) {
    reduceOp->options[i]->aggregate(exprCtx, reduceOptions[i]);
  }

  output->persist(outkey, outvalue, true);

  return RRC_SUCCESS;
}

} // namespace transform
} // namespace rdd
} // namespace idgs

