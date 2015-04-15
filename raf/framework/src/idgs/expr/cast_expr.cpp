
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "cast_expr.h"

using namespace std;
using namespace idgs::actor;
using namespace idgs::pb;
using namespace protobuf;

namespace idgs {
namespace expr {

PbVariant UDFToStringExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  string result = (string) value;
  auto pos = result.find(".");
  if (pos != string::npos) {
    auto end = result.find_last_not_of("0");
    if (end != string::npos) {
      if (end > pos) {
        result = result.substr(0, end + 1);
      } else if (end == pos) {
        result = result.substr(0, end + 2);
      }
    }
  }
  return PbVariant(result);
}

PbVariant UDFToLongExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  int64_t result = (int64_t) value;
  return PbVariant(result);
}

PbVariant UDFToIntegerExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  int32_t result = (int32_t) value;
  return PbVariant(result);
}

PbVariant UDFToShortExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  int32_t result = (int32_t) value;
  return PbVariant(result);
}

PbVariant UDFToByteExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  int32_t result = (int32_t) value;
  return PbVariant(result);
}

PbVariant UDFToFloatExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  float result = (float) value;
  return PbVariant(result);
}

PbVariant UDFToDoubleExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  double result = (double) value;
  return PbVariant(result);
}

PbVariant UDFToBooleanExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  bool result = (bool) value;
  return PbVariant(result);
}

bool UDFToBinaryExpression::parse(const Expr& entryExp, const PbMessagePtr& key, const PbMessagePtr& value) {
  if (entryExp.expression_size() == 0) {
    return true;
  }

  if (entryExp.expression_size() > 2) {
    LOG(ERROR) << "Failed to parse expression. cast(expr as binary[int n])";
    return false;
  }

  return parseSubExpression(entryExp, key, value);
}

PbVariant UDFToBinaryExpression::evaluate(ExpressionContext* ctx) const {
  auto value = children[0]->evaluate(ctx);
  string result = "";
  if (value.type == PbVariant::vt_string) {
    result = (string) value;
  }

  size_t size = result.size();
  if (children.size() == 2) {
    auto sizeValue = children[1]->evaluate(ctx);
    if (sizeValue.type > 4) {
      LOG(ERROR) << "expression cast(expr as binary[int n]), n must be a integer";
      throw std::invalid_argument("Invalid argument exception");
    }

    size = (size_t) sizeValue;
  }

  if (size < result.size()) {
    result = result.substr(0, size);
  }

  return PbVariant(result);
}

PbVariant UDFToDecimalExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type < 9) {
    return value;
  } else {
    double result = (double) value;
    return PbVariant(result);
  }
}

} // namespace expr
} // namespace idgs 
