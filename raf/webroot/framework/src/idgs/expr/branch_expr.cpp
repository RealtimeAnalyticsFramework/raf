
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "idgs/expr/branch_expr.h"

namespace idgs {
namespace expr {

///
/// conditional expression, ?:
///
protobuf::PbVariant WhenExpression::evaluate(ExpressionContext* ctx) const {
  int32_t i = 0;
  int32_t size = children.size() - 1;
  for (; i < size; i += 2) {
    if ((bool) children[i]->evaluate(ctx)) {
      return children[i + 1]->evaluate(ctx);
    }
  }

  return children[i]->evaluate(ctx);
}

bool WhenExpression::parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key,
    const idgs::actor::PbMessagePtr& value) {
  if (entryExp.expression_size() < 3 || !(entryExp.expression_size() & 1)) {
    LOG(ERROR) << "Failed to parse expression. IF(cond1, value1, cond2, value2, ... , condN, valueN, default)";
    return false;
  }

  return parseSubExpression(entryExp, key, value);
}

protobuf::PbVariant CaseExpression::evaluate(ExpressionContext* ctx) const {
  int32_t i = 1;
  auto expr = children[0]->evaluate(ctx);
  int32_t size = children.size() - 1;
  for (; i < size; i += 2) {
    if (expr == children[i]->evaluate(ctx)) {
      return children[i + 1]->evaluate(ctx);
    }
  }

  return children[i]->evaluate(ctx);
}

bool CaseExpression::parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key,
    const idgs::actor::PbMessagePtr& value) {
  if (entryExp.expression_size() < 5 || !(entryExp.expression_size() & 2)) {
    LOG(ERROR) << "Failed to parse expression. CASE[expr, cond_value1, value1, cond_value2, value2, ... , cond_valueN, valueN, default]";
    return false;
  }

  return parseSubExpression(entryExp, key, value);
}

protobuf::PbVariant NvlExpression::evaluate(ExpressionContext* ctx) const {
  auto expr = leftChild->evaluate(ctx);
  auto value = rightChild->evaluate(ctx);

  return (expr.is_null) ? value : expr;
}

protobuf::PbVariant CoalesceExpression::evaluate(ExpressionContext* ctx) const {
  for (auto expr : children) {
    auto value = expr->evaluate(ctx);
    if (!value.is_null) {
      return value;
    }
  }

  return protobuf::PbVariant(std::string("null"));
}

///
/// variable set expression
///
VariableSetExpression::VariableSetExpression() {
  index = 0;
}

VariableSetExpression::~VariableSetExpression() {
}

protobuf::PbVariant VariableSetExpression::evaluate(ExpressionContext* ctx) const {
  ctx->setVariable(index, child->evaluate(ctx));
  return ctx->getVariable(index);
}

bool VariableSetExpression::parse(const idgs::pb::Expr& entryExp,
    const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {

  index = std::stoul(entryExp.value());

  return parseSubExpression(entryExp, key, value);
}

///
/// variable get expression
///
VariableGetExpression::VariableGetExpression() {
  index = 0;
}

VariableGetExpression::~VariableGetExpression() {
}

protobuf::PbVariant VariableGetExpression::evaluate(ExpressionContext* ctx) const {
  return ctx->getVariable(index);
}

bool VariableGetExpression::parse(const idgs::pb::Expr& entryExp,
    const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {

  index = std::stoul(entryExp.value());
  return parseSubExpression(entryExp, key, value);
}

} // namespace expr
} // namespace idgs
