
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "string_expr.h"

namespace idgs {
namespace expr {

bool SubStrExpression::parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key,
    const idgs::actor::PbMessagePtr& value) {
  if (entryExp.expression_size() != 3 && entryExp.expression_size() != 2) {
    LOG(ERROR) << "Failed to parse expression. SUBSTR(source_string, start_position, [length])";
    return false;
  }

  return parseSubExpression(entryExp, key, value);
}

protobuf::PbVariant SubStrExpression::evaluate(ExpressionContext* ctx) const {
  auto value = children[0]->evaluate(ctx);
  std::string str = (std::string) value;

  auto expr_value = children[1]->evaluate(ctx);
  size_t pos = (size_t) expr_value;
  if (children.size() == 2) {
    return protobuf::PbVariant(str.substr(pos));
  }

  expr_value = children[2]->evaluate(ctx);
  size_t len = (size_t) expr_value;
  return protobuf::PbVariant(str.substr(pos, len));
}

} // namespace expr
} // namespace idgs 
