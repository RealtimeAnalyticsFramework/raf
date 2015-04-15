
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "transformer.h"

namespace idgs {
namespace rdd {
namespace transform {

Transformer::Transformer() {
}

Transformer::~Transformer() {
}

idgs::rdd::pb::RddResultCode Transformer::transform(TransformerContext* ctx, const idgs::rdd::BaseRddPartition* input, idgs::rdd::PairRddPartition* output) {
  if (!input->empty()) {
    input->foreach([this, ctx, output] (const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
      idgs::actor::PbMessagePtr outkey, outvalue;

      auto exprCtx = ctx->getExpressionContext();
      exprCtx->setKeyValue(&key, &value);
      exprCtx->setOutputKeyValue(&outkey, &outvalue);

      auto code = transform(ctx, output);
      if (code != idgs::rdd::pb::RRC_SUCCESS) {
        LOG(ERROR) << output->getPartitionName() << " transformer " << getName() << " error, caused by " << RddResultCode_Name(code);
      }
    });
  }

  return idgs::rdd::pb::RRC_SUCCESS;
}

idgs::rdd::pb::RddResultCode Transformer::transform(TransformerContext* ctx, idgs::rdd::PairRddPartition* output) {
  auto exprCtx = ctx->getExpressionContext();
  if (ctx->getRddOperator()->evaluate(exprCtx)) {
    auto outkey = * exprCtx->getOutputKey();
    auto outvalue = * exprCtx->getOutputValue();

    if (!outkey || !outvalue) {
      LOG(ERROR) << output->getPartitionName() << " operator " << ctx->getRddOperator()->getName() << " returns null";
      LOG_IF(ERROR, exprCtx->getKey() && *exprCtx->getKey()) << "Input key " << (*exprCtx->getKey())->DebugString();
      LOG_IF(ERROR, exprCtx->getValue() && *exprCtx->getValue()) << "input value: " << (*exprCtx->getValue())->DebugString();
    }

    output->processRow(outkey, outvalue);
  }

  return idgs::rdd::pb::RRC_SUCCESS;
}

idgs::rdd::pb::RddResultCode Transformer::aggregate(TransformerContext* ctx, idgs::rdd::PairRddPartition* output) {
  return idgs::rdd::pb::RRC_SUCCESS;
}

} // namespace transform
} // namespace rdd
} // namespace idgs 
