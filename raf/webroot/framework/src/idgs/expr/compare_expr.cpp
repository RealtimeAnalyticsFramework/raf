
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "compare_expr.h"

using namespace protobuf;

namespace idgs {
namespace expr {

PbVariant EQExpression::evaluate(ExpressionContext* ctx) const {
  PbVariant lvar = leftChild->evaluate(ctx);
  PbVariant rvar = rightChild->evaluate(ctx);
  bool result = (lvar == rvar);

  return PbVariant(result);
}

PbVariant NEExpression::evaluate(ExpressionContext* ctx) const {
  PbVariant lvar = leftChild->evaluate(ctx);
  PbVariant rvar = rightChild->evaluate(ctx);
  bool result = (lvar != rvar);

  return PbVariant(result);
}

PbVariant LTExpression::evaluate(ExpressionContext* ctx) const {
  PbVariant lvar = leftChild->evaluate(ctx);
  PbVariant rvar = rightChild->evaluate(ctx);
  bool result = (lvar < rvar);

  return PbVariant(result);
}

PbVariant LEExpression::evaluate(ExpressionContext* ctx) const {
  PbVariant lvar = leftChild->evaluate(ctx);
  PbVariant rvar = rightChild->evaluate(ctx);
  bool result = (lvar <= rvar);

  return PbVariant(result);
}

PbVariant GTExpression::evaluate(ExpressionContext* ctx) const {
  PbVariant lvar = leftChild->evaluate(ctx);
  PbVariant rvar = rightChild->evaluate(ctx);
  bool result = (lvar > rvar);

  return PbVariant(result);
}

PbVariant GEExpression::evaluate(ExpressionContext* ctx) const {
  PbVariant lvar = leftChild->evaluate(ctx);
  PbVariant rvar = rightChild->evaluate(ctx);
  bool result = (lvar >= rvar);

  return PbVariant(result);
}

bool BetweenExpression::parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
  if (entryExp.expression_size() != 4) {
    LOG(ERROR) << "Failed to parse expression. BETWEEN(between or not between, expression, left condition, right condition";
    return false;
  }

  if (entryExp.expression(0).name() != "CONST" || entryExp.expression(0).const_type() != pb::BOOL) {
    LOG(ERROR) << "Failed to parse expression. BETWEEN(between or not between, expression, left condition, right condition";
    return false;
  }

  return parseSubExpression(entryExp, key, value);
}

PbVariant BetweenExpression::evaluate(ExpressionContext* ctx) const {
  PbVariant isNot = children[0]->evaluate(ctx);
  PbVariant field = children[1]->evaluate(ctx);
  PbVariant lvar = children[2]->evaluate(ctx);
  PbVariant rvar = children[3]->evaluate(ctx);
  bool result = (field >= lvar) && (field <= rvar);

  return PbVariant((isNot) ? !result : result);
}

PbVariant InExpression::evaluate(ExpressionContext* ctx) const {
  bool result = false;
  PbVariant field = children[0]->evaluate(ctx);
  for (int32_t i = 1; i < children.size(); ++ i) {
    PbVariant value = children[i]->evaluate(ctx);
    if (field == value) {
      result = true;
      break;
    }
  }

  return PbVariant(result);
}

PbVariant IsNullExpression::evaluate(ExpressionContext* ctx) const {
  PbVariant var = child->evaluate(ctx);
  bool result = var.is_null;
  return PbVariant(result);
}

PbVariant IsNotNullExpression::evaluate(ExpressionContext* ctx) const {
  PbVariant var = child->evaluate(ctx);
  bool result = !var.is_null;
  return PbVariant(result);
}

} // namespace expr
} // namespace idgs 
