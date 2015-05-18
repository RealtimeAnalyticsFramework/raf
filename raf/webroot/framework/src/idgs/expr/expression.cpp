
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "expression.h"
#include "expression_factory.h"

namespace idgs {
namespace expr {

Expression::Expression() {
}

Expression::~Expression() {
}

bool NullaryExpression::parseSubExpression(const idgs::pb::Expr& entryExp,
    const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
  if (entryExp.expression_size()) {
    LOG(ERROR) << "Invalid subexpression: " << entryExp.name();
    return false;
  }

  return true;
}

bool UnaryExpression::parseSubExpression(const idgs::pb::Expr& entryExp,
    const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
  if (entryExp.expression_size() != 1) {
    LOG(ERROR) << "Invalid subexpression: " << entryExp.name();
    return false;
  }

  idgs::ResultCode rc = ExpressionFactory::build(&child, entryExp.expression(0), key, value);

  if (rc != idgs::RC_SUCCESS) {
    LOG(ERROR) << "Failed to parse expression.";
    return false;
  }

  return true;
}

bool BinaryExpression::parseSubExpression(const idgs::pb::Expr& entryExp,
    const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
  if (entryExp.expression_size() != 2) {
    LOG(ERROR) << "Invalid subexpression: " << entryExp.name();
    return false;
  }

  idgs::ResultCode rc = ExpressionFactory::build(&leftChild, entryExp.expression(0), key, value);
  if (rc != idgs::RC_SUCCESS) {
    LOG(ERROR) << "Failed to parse expression.";
    return false;
  }

  rc = ExpressionFactory::build(&rightChild, entryExp.expression(1), key, value);
  if (rc != idgs::RC_SUCCESS) {
    LOG(ERROR) << "Failed to parse expression.";
    return false;
  }

  return true;
}

bool TernaryExpression::parseSubExpression(const idgs::pb::Expr& entryExp,
    const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
  if (entryExp.expression_size() != 3) {
    LOG(ERROR) << "Invalid subexpression: " << entryExp.name();
    return false;
  }

  idgs::ResultCode rc = ExpressionFactory::build(&leftChild, entryExp.expression(0), key, value);
  if (rc != idgs::RC_SUCCESS) {
    LOG(ERROR) << "Failed to parse expression.";
    return false;
  }

  rc = ExpressionFactory::build(&middleChild, entryExp.expression(1), key, value);
  if (rc != idgs::RC_SUCCESS) {
    LOG(ERROR) << "Failed to parse expression.";
    return false;
  }

  rc = ExpressionFactory::build(&rightChild, entryExp.expression(2), key, value);
  if (rc != idgs::RC_SUCCESS) {
    LOG(ERROR) << "Failed to parse expression.";
    return false;
  }

  return true;
}

bool NAryExpression::parseSubExpression(const idgs::pb::Expr& entryExp,
    const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
  if (entryExp.expression_size() == 0) {
    return false;
  }
  for (int32_t i = 0; i < entryExp.expression_size(); ++i) {
    Expression* expr = NULL;
    idgs::ResultCode rc = ExpressionFactory::build(&expr, entryExp.expression(i), key, value);
    if (rc != idgs::RC_SUCCESS) {
      LOG(ERROR) << "Failed to parse expression, type is "
          << entryExp.expression(i).name() << ", caused by "
          << idgs::getErrorDescription(rc);
      return false;
    }

    children.push_back(expr);
  }

  return true;
}

} // namespace expr
} // namespace idgs 
