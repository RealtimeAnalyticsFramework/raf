
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "math_expr.h"

#include <math.h>

using namespace idgs::pb;
using namespace idgs::actor;
using namespace protobuf;

namespace idgs {
namespace expr {

bool RoundExpression::parse(const Expr& entryExp, const PbMessagePtr& key, const PbMessagePtr& value) {
  if (entryExp.expression_size() != 2 && entryExp.expression_size() != 1) {
    LOG(ERROR) << "Failed to parse expression. round(n[, d])";
    return false;
  }

  return parseSubExpression(entryExp, key, value);
}

PbVariant RoundExpression::evaluate(ExpressionContext* ctx) const {
  auto value = children[0]->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression round(n[, d]), n must be number";
    throw std::invalid_argument("Invalid argument exception");
  }

  if (value.type <= 4) {
    return value;
  }

  double result = 0;
  double n = (double) value;
  if (children.size() == 2) {
    value = children[1]->evaluate(ctx);
    if (value.type > 4) {
      LOG(ERROR) << "expression round(n[, d]), d must be a integer";
      throw std::invalid_argument("Invalid argument exception");
    }

    int32_t d = (int32_t) value;
    double p = pow(10, d);
    result = round(n * p) / p;
  } else {
    result = round(n);
  }

  return PbVariant(result);
}

PbVariant FloorExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression floor(n), n must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  if (value.type <= 4) {
    return value;
  } else {
    return PbVariant(floor((double) value));
  }
}

PbVariant CeilExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression floor(n), n must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  if (value.type <= 4) {
    return value;
  } else {
    return PbVariant(ceil((double) value));
  }
}

bool RandExpression::parse(const Expr& entryExp, const PbMessagePtr& key, const PbMessagePtr& value) {
  if (entryExp.expression_size() == 0) {
    return true;
  }

  if (entryExp.expression_size() > 1) {
    LOG(ERROR) << "Failed to parse expression. rand() or rand(seed)";
    return false;
  }

  return parseSubExpression(entryExp, key, value);
}

PbVariant RandExpression::evaluate(ExpressionContext* ctx) const {
  double result = 0;
  if (child) {
    auto value = child->evaluate(ctx);
    if (value.type > 4) {
      LOG(ERROR) << "expression rand(seed), seed must be a integer";
      throw std::invalid_argument("Invalid argument exception");
    }

    uint32_t seed = (uint32_t) value;
    result = (rand_r(&seed) % 10) / 10;
  } else {
    result = (rand() % 10) / 10;
  }

  return PbVariant(result);
}

PbVariant ExpExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression exp(n), n must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double n = (double) value;
  double result = exp(n);

  return PbVariant(result);
}

PbVariant LnExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression ln(n), n must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double n = (double) value;
  double result = log(n);

  return PbVariant(result);
}

PbVariant Log10Expression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression log10(n), n must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double n = (double) value;
  double result = log10(n);

  return PbVariant(result);
}

PbVariant Log2Expression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression log2(n), n must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double n = (double) value;
  double result = log2(n);

  return PbVariant(result);
}

PbVariant LogExpression::evaluate(ExpressionContext* ctx) const {
  auto value = leftChild->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression log(double base, double n), base must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double base = (double) value;

  value = rightChild->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression log(double base, double n), n must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double n = (double) value;
  double result = log(n) / log(base);

  return PbVariant(result);
}

PbVariant PowExpression::evaluate(ExpressionContext* ctx) const {
  auto value = leftChild->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression pow(double base, double power), base must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double base = (double) value;

  value = rightChild->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression pow(double base, double power), power must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double power = (double) value;
  double result = pow(base, power);

  return PbVariant(result);
}

PbVariant SqrtExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression sqrt(n), n must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double n = (double) value;
  if (n < 0) {
    LOG(ERROR) << "expression sqrt(n), n must be > 0";
    throw std::invalid_argument("Invalid argument exception");
  }

  double result = sqrt(n);

  return PbVariant(result);
}

PbVariant BinExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 4) {
    LOG(ERROR) << "expression bin(BIGINT n), n must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  int64_t n = (int64_t) value;

  static int32_t length = 8 * sizeof(void*);
  std::string result;
  int32_t len = 0;
  do {
    char c = (01 & n) + '0';
    result = c + result;
    n >>= 1;
    ++ len;
  } while (n != 0 && len < length);

  return PbVariant(result);
}

char HexExpression::toHexString(int digit) {
  if ((digit >= 16) || (digit < 0)) {
    return '\0';
  }
  if (digit < 10) {
    return (char) ('0' + digit);
  }
  return (char) ('A' - 10 + digit);
}

PbVariant HexExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 4 && value.type != PbVariant::vt_string) {
    LOG(ERROR) << "expression hex(BIGINT n), hex(string n), n must be a number or a string";
    throw std::invalid_argument("Invalid argument exception");
  }

  std::string result;

  if (value.type == PbVariant::vt_string) {
    auto s = (std::string) value;
    const char* str = s.c_str();
    for (int i = 0; i < s.size(); ++ i) {
      char c1 = toHexString((str[i] & 0xF0) >> 4);
      char c2 = toHexString(str[i] & 0x0F);
      result = result + c1 + c2;
    }
  } else {
    int32_t length = 2 * sizeof(void*);
    int32_t n = (int32_t) value;

    int len = 0;
    do {
      char c = toHexString(n & 0xF);
      result = c + result;
      n >>= 4;
      ++ len;
    } while (n != 0 && len < length);
  }

  return PbVariant(result);
}

PbVariant UnHexExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type != PbVariant::vt_string) {
    LOG(ERROR) << "expression unhex(string s), s must be a number or a string";
    throw std::invalid_argument("Invalid argument exception");
  }

  std::string s = (std::string) value;
  if (s.size() % 2 == 1) {
    s = "0" + s;
  }

  std::string result;
  for (int i = 0; i < s.size(); i += 2) {
    auto n = strtol(s.substr(i, 2).c_str(), NULL, 16);
    result += (char) n;
  }

  return PbVariant(result);
}

std::string ConvExpression::conv(const std::string& number, const uint32_t& fromBase, const uint32_t& toBase) {
  if (fromBase < 2 || fromBase > 36 || toBase < 2 || toBase > 36) {
    return "ERROR";
  }

  static const char NUMS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::string result = "";
  int temp = 0, x;
  bool found = false;
  for (int i = 0; i < number.length(); i++) {
    for (x = 0; x < fromBase; x++) {
      if (NUMS[x] == number[number.length() - (i + 1)]) {
        found = true;
        break;
      }
    }

    if (!found) {
      return "ERROR";
    }

    temp += (x * pow(fromBase, i));
  }

  do {
    result.push_back(NUMS[temp % toBase]);
    temp /= toBase;
  } while (temp != 0);

  return std::string(result.rbegin(), result.rend());
}

PbVariant ConvExpression::evaluate(ExpressionContext* ctx) const {
  auto number = leftChild->evaluate(ctx);
  if (number.type > 4 && number.type != PbVariant::vt_string) {
    LOG(ERROR) << "expression conv(num, int from_base, int to_base), num must be a number or a string";
    throw std::invalid_argument("Invalid argument exception");
  }

  auto fromBase = middleChild->evaluate(ctx);
  if (fromBase.type > 4) {
    LOG(ERROR) << "expression conv(num, int from_base, int to_base), from_base must be a positive integer";
    throw std::invalid_argument("Invalid argument exception");
  }

  auto toBase = rightChild->evaluate(ctx);
  if (toBase.type > 4) {
    LOG(ERROR) << "expression conv(num, int from_base, int to_base), to_base must be a positive integer";
    throw std::invalid_argument("Invalid argument exception");
  }

  int32_t fromBaseValue = (int32_t) fromBase;
  int32_t toBaseValue = (int32_t) toBase;

  if (fromBaseValue < 2 || fromBaseValue > 36) {
    LOG(ERROR) << "expression conv(num, int from_base, int to_base), from_base must be in [2, 36]";
    throw std::invalid_argument("Invalid argument exception");
  }

  if (toBaseValue < 2 || toBaseValue > 36) {
    LOG(ERROR) << "expression conv(num, int from_base, int to_base), to_base must be in [2, 36]";
    throw std::invalid_argument("Invalid argument exception");
  }

  std::string result = conv((std::string) number, fromBaseValue, toBaseValue);
  return PbVariant(result);
}

PbVariant AbsExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression abs(double n), n must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double n = (double) value;
  double result = fabs(n);

  return PbVariant(result);
}

PbVariant PModExpression::evaluate(ExpressionContext* ctx) const {
  auto value1 = leftChild->evaluate(ctx);
  if (value1.type > 6) {
    LOG(ERROR) << "expression pmod(int a, int b), pmod(double a, double b), a must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  auto value2 = rightChild->evaluate(ctx);
  if (value2.type > 6) {
    LOG(ERROR) << "expression pmod(int a, int b), pmod(double a, double b), b must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double a = (double) value1;
  double b = (double) value2;

  if (!b) {
    LOG(ERROR) << "expression pmod(int a, int b), pmod(double a, double b), b is 0";
    throw std::overflow_error("Divide by zero exception");;
  }

  double result = fabs(fmod(a, b));
  return PbVariant(result);
}

PbVariant SinExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression sin(double a), a must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double a = (double) value;
  double result = sin(a);

  return PbVariant(result);
}

PbVariant ASinExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression asin(double a), a must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double a = (double) value;
  double result = asin(a);

  return PbVariant(result);
}

PbVariant CosExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression cos(double a), a must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double a = (double) value;
  double result = cos(a);

  return PbVariant(result);
}

PbVariant ACosExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression acos(double a), a must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double a = (double) value;
  double result = acos(a);

  return PbVariant(result);
}

PbVariant PositiveExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression positive(int a) positive(double a), a must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  return PbVariant(value);
}

PbVariant NegativeExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression negative(int a) negative(double a), a must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double result = 0 - (double) value;
  return PbVariant(result);
}

PbVariant DegreesExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression degrees(double d), d must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double d = (double) value;
  double result = d * 180.0 / M_PI;

  return PbVariant(result);
}

PbVariant RadiansExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression radians(double d), d must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double d = (double) value;
  double result = d / 180.0 * M_PI;

  return PbVariant(result);
}

PbVariant SignExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 6) {
    LOG(ERROR) << "expression sign(double d), d must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  double d = (double) value;
  int32_t result = 0;
  if (d > 0) {
    result = 1;
  } else if (d < 0) {
    result = -1;
  }

  return PbVariant(result);
}

PbVariant EExpression::evaluate(ExpressionContext* ctx) const {
  return PbVariant(M_E);
}

PbVariant PIExpression::evaluate(ExpressionContext* ctx) const {
  return PbVariant(M_PI);
}

} // namespace expr
} // namespace idgs 
