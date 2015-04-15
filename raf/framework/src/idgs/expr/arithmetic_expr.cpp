
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "arithmetic_expr.h"

#include <math.h>

using namespace protobuf;

namespace idgs {
namespace expr {

PbVariant AddExpression::evaluate(ExpressionContext* ctx) const {
  double result = 0;
  if (children.size() > 0) {
    auto it = children.begin();
    result = (double) (* it)->evaluate(ctx);
    for (++ it; it != children.end(); ++ it) {
      result = result + (double) (* it)->evaluate(ctx);
    }
  }

  return PbVariant(result);
}

PbVariant SubtractExpression::evaluate(ExpressionContext* ctx) const {
  double result = 0;
  if (children.size() > 0) {
    auto it = children.begin();
    result = (double) (* it)->evaluate(ctx);
    for (++ it; it != children.end(); ++ it) {
      result = result - (double) (* it)->evaluate(ctx);
    }
  }

  return PbVariant(result);
}

PbVariant MultiplyExpression::evaluate(ExpressionContext* ctx) const {
  double result = 0;
  if (children.size() > 0) {
    auto it = children.begin();
    result = (double) (* it)->evaluate(ctx);
    for (++ it; it != children.end(); ++ it) {
      result = result * (double) (* it)->evaluate(ctx);
    }
  }

  return PbVariant(result);
}

PbVariant DivideExpression::evaluate(ExpressionContext* ctx) const {
  double result = 0;
  if (children.size() > 0) {
    auto it = children.begin();
    result = (double) (*it)->evaluate(ctx);
    for (++ it; it != children.end(); ++ it) {
      double value = (double) (* it)->evaluate(ctx);
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
  auto value1 = leftChild->evaluate(ctx);
  if (value1.type > 6) {
    LOG(ERROR) << "expression mod(a, b), a must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  auto value2 = rightChild->evaluate(ctx);
  if (value2.type > 6) {
    LOG(ERROR) << "expression mod(a, b), b must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double a = (double) value1;
  double b = (double) value2;

  if (!b) {
    LOG(ERROR) << "expression mod(a, b), b is 0";
    throw std::overflow_error("Divide by zero exception");;
  }

  double result = fmod(a, b);
  return PbVariant(result);
}

PbVariant HashExpression::evaluate(ExpressionContext* ctx) const {
  size_t result = 0;
  for (auto& expr : children) {
    result = result * 99989 + expr->evaluate(ctx).hashcode();
  }

  return PbVariant(result);
}

PbVariant BitAndExpression::evaluate(ExpressionContext* ctx) const {
  int32_t result = 0;
  if (!children.empty()) {
    result = (int32_t) children[0]->evaluate(ctx);
    for (int32_t i = 1; i < children.size(); ++ i) {
      result = result & (int32_t)children[i]->evaluate(ctx);
    }
  }

  return PbVariant(result);
}

PbVariant BitOrExpression::evaluate(ExpressionContext* ctx) const {
  int32_t result = 0;
  if (!children.empty()) {
    result = (int32_t) children[0]->evaluate(ctx);
    for (int32_t i = 1; i < children.size(); ++ i) {
      result = result | (int32_t)children[i]->evaluate(ctx);
    }
  }

  return PbVariant(result);
}

PbVariant BitNotExpression::evaluate(ExpressionContext* ctx) const {
  int32_t result = (int32_t) child->evaluate(ctx);
  result = ~result;

  return PbVariant(result);
}

PbVariant BitXorExpression::evaluate(ExpressionContext* ctx) const {
  int32_t result = 0;
  if (!children.empty()) {
    result = (int32_t) children[0]->evaluate(ctx);
    for (int32_t i = 1; i < children.size(); ++ i) {
      result = result ^ (int32_t)children[i]->evaluate(ctx);
    }
  }

  return PbVariant(result);
}

} // namespace expr
} // namespace idgs 
