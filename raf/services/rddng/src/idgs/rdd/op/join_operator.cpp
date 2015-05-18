/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "join_operator.h"

namespace idgs {
namespace rdd {
namespace op {

JoinOperator::JoinOperator() : joinType(pb::INNER_JOIN) {
}

JoinOperator::~JoinOperator() {
}

bool JoinOperator::parse(const pb::InRddInfo& inRddInfo, const pb::OutRddInfo& outRddInfo,
    std::shared_ptr<RddLocal>& inRddLocal, std::shared_ptr<RddLocal>& outRddLocal) {
  auto& msg = outRddLocal->getTransformerMsg();
  pb::CreateRddRequest* request = dynamic_cast<pb::CreateRddRequest*>(msg->getPayload().get());
  if (request->in_rdd_size() <= 1) {
    return false;
  }

  if (!mapOperator.parse(inRddInfo, outRddInfo, inRddLocal, outRddLocal)) {
    LOG(ERROR) << "Failed to parse operator";
    return false;
  }

  pb::JoinRequest join;
  if (!msg->parseAttachment(TRANSFORMER_PARAM, &join)) {
    LOG(ERROR) << "Failed to parse attachement: " << TRANSFORMER_PARAM;
    return false;
  }
  joinType = join.type();

  if (joinType == pb::OUTER_JOIN) {
    inRddLocal->setPersistType(pb::ORDERED);
  }

  return true;
}

bool JoinOperator::evaluate(idgs::expr::ExpressionContext* ctx) {
  return mapOperator.evaluate(ctx);
}


} /* namespace op */
} /* namespace rdd */
} /* namespace idgs */
