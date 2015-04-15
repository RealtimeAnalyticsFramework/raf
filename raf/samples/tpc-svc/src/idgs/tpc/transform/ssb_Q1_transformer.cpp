/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "ssb_Q1_transformer.h"

using namespace std;
using namespace idgs::actor;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::rdd::transform;

namespace idgs {
namespace tpc {
namespace transform {

SsbQ1_1Transformer::SsbQ1_1Transformer() {
}

SsbQ1_1Transformer::~SsbQ1_1Transformer() {
}

RddResultCode SsbQ1_1Transformer::transform(TransformerContext* ctx, const BaseRddPartition* input, PairRddPartition* output) {
  auto param = ctx->getParamRdds().at(0);
  if (input->empty() || param->empty()) {
    return RRC_SUCCESS;
  }

  auto op = ctx->getRddOperator();
  auto paramOp = op->paramOperators.at(0);
  auto exprCtx = ctx->getExpressionContext();

  auto keyTemplate = param->getKeyTemplate();
  auto valueTemplate = param->getValueTemplate();
  auto dateDes = keyTemplate->GetDescriptor();
  auto dateRef = keyTemplate->GetReflection();
  auto dateField = dateDes->FindFieldByName("d_datekey");

  set<uint64_t> marchedDate;

  param->foreach([&marchedDate, paramOp, &exprCtx, dateRef, dateField] (const PbMessagePtr& key, const PbMessagePtr& value) {
    idgs::actor::PbMessagePtr outkey, outvalue;
    exprCtx->setKeyValue(&key, &value);
    exprCtx->setOutputKeyValue(&outkey, &outvalue);

    if (paramOp->evaluate(exprCtx)) {
      marchedDate.insert(dateRef->GetUInt64(* key, dateField));
    }
  });

  valueTemplate = input->getValueTemplate();
  auto orderDes = valueTemplate->GetDescriptor();
  auto orderRef = valueTemplate->GetReflection();
  auto orderDateField = orderDes->FindFieldByName("lo_orderdate");

  input->foreach([marchedDate, output, op, &exprCtx, orderRef, orderDateField] (const PbMessagePtr& key, const PbMessagePtr& value) {
    idgs::actor::PbMessagePtr outkey, outvalue;
    exprCtx->setKeyValue(&key, &value);
    exprCtx->setOutputKeyValue(&outkey, &outvalue);

    if (op->evaluate(exprCtx)) {
      uint64_t orderdate = orderRef->GetUInt64(* value, orderDateField);

      if(marchedDate.find(orderdate) != marchedDate.end()) {
        output->put(key, value);
      }
    }
  });

  return RRC_SUCCESS;
}

} /* namespace transform */
} /* namespace tpc */
} /* namespace idgs */
