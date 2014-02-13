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
using namespace idgs::expr;

namespace idgs {
namespace tpc {
namespace transform {

SsbQ1_1Transformer::SsbQ1_1Transformer() {
}

SsbQ1_1Transformer::~SsbQ1_1Transformer() {
}

RddResultCode SsbQ1_1Transformer::transform(const ActorMessagePtr& msg, const std::vector<BaseRddPartition*>& input,
    RddPartition* output) {
  if (input.size() != 2) {
    return RRC_INVALID_RDD_INPUT;
  }

  if (input[0]->empty() || input[1]->empty()) {
    return RRC_SUCCESS;
  }

  auto orderExp = output->getFilterExpression(0);
  auto dateExp = output->getFilterExpression(1);

  auto valueTemplate = input[0]->getValueTemplate();
  auto orderDes = valueTemplate->GetDescriptor();
  auto orderRef = valueTemplate->GetReflection();
  auto orderDateField = orderDes->FindFieldByName("lo_orderdate");

  auto keyTemplate = input[1]->getKeyTemplate();
  valueTemplate = input[1]->getValueTemplate();
  auto dateDes = keyTemplate->GetDescriptor();
  auto dateRef = keyTemplate->GetReflection();
  auto dateField = dateDes->FindFieldByName("d_datekey");

  set<uint64_t> marchedDate;
  ExpressionContext ctx;
  input[1]->foreach(
      [&marchedDate, dateExp, dateRef, dateField, &ctx] (const PbMessagePtr& key, const PbMessagePtr& value) {
        ctx.setKeyValue(&key, &value);
        if ((bool) dateExp->evaluate(&ctx)) {
          marchedDate.insert(dateRef->GetUInt64(*key, dateField));
        }
      });

  input[0]->foreach(
      [marchedDate, output, orderExp, orderRef, orderDateField, dateRef, dateField, &ctx] (const PbMessagePtr& key, const PbMessagePtr& value) {
        ctx.setKeyValue(&key, &value);
        if ((bool) orderExp->evaluate(&ctx)) {
          uint64_t orderdate = orderRef->GetUInt64(* value, orderDateField);

          if(marchedDate.find(orderdate) != marchedDate.end()) {
            output->putLocal(key, value);
          }
        }
      });

  return RRC_SUCCESS;
}

} /* namespace transform */
} /* namespace tpc */
} /* namespace idgs */
