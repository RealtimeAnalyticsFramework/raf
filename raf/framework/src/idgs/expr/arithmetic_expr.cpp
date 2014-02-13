
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "arithmetic_expr.h"

using namespace protobuf;

namespace idgs {
namespace expr {

PbVariant AddExpression::evaluate(ExpressionContext* ctx) const {
  double result = 0;
  if (children.size() > 0) {
    auto it = children.begin();
    result = (double) (*it)->evaluate(ctx);
    for (++it; it != children.end(); ++it) {
      result = result + (double) (*it)->evaluate(ctx);
    }
  }

  return PbVariant(result);
}

PbVariant SubtractExpression::evaluate(ExpressionContext* ctx) const {
  double result = 0;
  if (children.size() > 0) {
    auto it = children.begin();
    result = (double) (*it)->evaluate(ctx);
    for (++it; it != children.end(); ++it) {
      result = result - (double) (*it)->evaluate(ctx);
    }
  }

  return PbVariant(result);
}

PbVariant MultiplyExpression::evaluate(ExpressionContext* ctx) const {
  double result = 0;
  if (children.size() > 0) {
    auto it = children.begin();
    result = (double) (*it)->evaluate(ctx);
    for (++it; it != children.end(); ++it) {
      result = result * (double) (*it)->evaluate(ctx);
    }
  }

  return PbVariant(result);
}

PbVariant DivideExpression::evaluate(ExpressionContext* ctx) const {
  double result = 0;
  if (children.size() > 0) {
    auto it = children.begin();
    result = (double) (*it)->evaluate(ctx);
    for (++it; it != children.end(); ++it) {
      double value = (double) (*it)->evaluate(ctx);
      if (value == 0) {
        throw std::overflow_error("Divide by zero exception");
      } else {
        result = result / value;
      }
    }
  }

  return PbVariant(result);
}

PbVariant ModulusExpression::evaluate(ExpressionContext* ctx) const {
  int32_t result = 0;
  if (children.size() > 0) {
    auto it = children.begin();
    result = (int32_t) (*it)->evaluate(ctx);
    for (++it; it != children.end(); ++it) {
      int32_t value = (int32_t) (*it)->evaluate(ctx);
      if (value == 0) {
        throw std::overflow_error("Divide by zero exception");
      } else {
        result = result % value;
      }
    }
  }

  return PbVariant(result);
}

PbVariant HashExpression::evaluate(ExpressionContext* ctx) const {
  size_t result = 0;
  for (auto& expr : children) {
    result = result * 99989 + expr->evaluate(ctx).hashcode();
  }

  return PbVariant(result);
}

} // namespace expr
} // namespace idgs 
