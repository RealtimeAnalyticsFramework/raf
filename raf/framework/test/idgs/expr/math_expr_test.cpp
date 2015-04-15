/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $
#include <math.h>

#include <gtest/gtest.h>
#include "expr_common.h"
#include "idgs/expr/expression_helper.h"

using namespace std;
using namespace idgs;
using namespace idgs::expr;
using namespace idgs::pb;
using namespace protobuf;

TEST(math_expr, round) {
  ExpressionFactory::init();

  LOG(INFO) << "test round(n[,d])";

  PbVariant result;

  LOG(INFO) << "1. test round(12345);";
  auto code = exprEvaluate(result, EXPR("ROUND", CONST("12345", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(12345, (int64_t) result);

  LOG(INFO) << "2. test round(12345.6789);";
  code = exprEvaluate(result, EXPR("ROUND", CONST("12345.6789", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(12346, (int64_t) result);

  LOG(INFO) << "3. test round(12345.6789, 2);";
  code = exprEvaluate(result, EXPR("ROUND", CONST("12345.6789", DOUBLE), CONST("2", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(12345.68, (double) result);
}

TEST(math_expr, floor) {
  LOG(INFO) << "test floor(n)";

  PbVariant result;

  LOG(INFO) << "1. test floor(12345);";
  auto code = exprEvaluate(result, EXPR("FLOOR", CONST("12345", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(12345, (int64_t) result);

  LOG(INFO) << "2. test floor(12345.6789);";
  code = exprEvaluate(result, EXPR("FLOOR", CONST("12345.6789", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(12345, (double) result);
}

TEST(math_expr, ceil) {
  LOG(INFO) << "test ceil(n) ceiling(n)";

  PbVariant result;

  LOG(INFO) << "1. test ceil(12345);";
  auto code = exprEvaluate(result, EXPR("CEIL", CONST("12345", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(12345, (int64_t) result);

  LOG(INFO) << "2. test ceiling(12345.6789);";
  code = exprEvaluate(result, EXPR("CEILING", CONST("12345.6789", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(12346, (double) result);
}

TEST(math_expr, rand) {
  LOG(INFO) << "test rand() rand(n)";

  PbVariant result;

  LOG(INFO) << "1. test rand();";
  auto code = exprEvaluate(result, EXPR("RAND"));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_LE(0, (double) result);
  EXPECT_GE(1, (double) result);

  LOG(INFO) << "2. test rand(12345);";
  code = exprEvaluate(result, EXPR("RAND", CONST("12345", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_LE(0, (double) result);
  EXPECT_GE(1, (double) result);

  LOG(INFO) << "3. test error;";
  code = exprEvaluate(result, EXPR("RAND", CONST("abc")));
  EXPECT_EQ(RC_ERROR, code);
}

TEST(math_expr, exp) {
  LOG(INFO) << "test exp(n)";

  PbVariant result;

  LOG(INFO) << "1. test exp();";
  auto code = exprEvaluate(result, EXPR("EXP", CONST("3", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(20.085536923187668, (double) result);

  LOG(INFO) << "3. test error;";
  code = exprEvaluate(result, EXPR("EXP", CONST("abc")));
  EXPECT_EQ(RC_ERROR, code);
}

TEST(math_expr, ln) {
  LOG(INFO) << "test ln(n)";

  PbVariant result;

  LOG(INFO) << "1. test ln();";
  auto code = exprEvaluate(result, EXPR("LN", CONST("20.085536923187668", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(3, (double) result);

  LOG(INFO) << "2. test error;";
  code = exprEvaluate(result, EXPR("LN", CONST("abc")));
  EXPECT_EQ(RC_ERROR, code);
}

TEST(math_expr, log) {
  LOG(INFO) << "test log10(n) log2(n) log(n)";

  PbVariant result;

  LOG(INFO) << "1. test log10(1000000000);";
  auto code = exprEvaluate(result, EXPR("LOG10", CONST("1000000000", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(9, (double) result);

  LOG(INFO) << "2. test log2(65536);";
  code = exprEvaluate(result, EXPR("LOG2", CONST("65536", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(16, (double) result);

  LOG(INFO) << "2. test log(2, 65536);";
  code = exprEvaluate(result, EXPR("LOG", CONST("2", INT64), CONST("65536", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(16, (double) result);

  LOG(INFO) << "4. test error;";
  code = exprEvaluate(result, EXPR("LOG10", CONST("abc")));
  EXPECT_EQ(RC_ERROR, code);
}

TEST(math_expr, pow) {
  LOG(INFO) << "test pow(base, n)";

  PbVariant result;

  LOG(INFO) << "1. test pow(2, 0);";
  auto code = exprEvaluate(result, EXPR("POW", CONST("2", INT64), CONST("0", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(1, (int32_t) result);

  LOG(INFO) << "2. test pow(2, 16);";
  code = exprEvaluate(result, EXPR("POW", CONST("2", INT64), CONST("16", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(65536, (int32_t) result);

  LOG(INFO) << "3. test pow(16, 0.25);";
  code = exprEvaluate(result, EXPR("POW", CONST("16", INT64), CONST("0.25", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(2, (int32_t) result);

  LOG(INFO) << "4. test error;";
  code = exprEvaluate(result, EXPR("POW", CONST("abc"), CONST("abc")));
  EXPECT_EQ(RC_ERROR, code);
}

TEST(math_expr, sqrt) {
  LOG(INFO) << "test sqrt(n)";

  PbVariant result;

  LOG(INFO) << "1. test sqrt(0);";
  auto code = exprEvaluate(result, EXPR("SQRT", CONST("0", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(0, (int32_t) result);

  LOG(INFO) << "2. test sqrt(1);";
  code = exprEvaluate(result, EXPR("SQRT", CONST("1", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(1, (int32_t) result);

  LOG(INFO) << "3. test sqrt(16);";
  code = exprEvaluate(result, EXPR("SQRT", CONST("16", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(4, (int32_t) result);

  LOG(INFO) << "4. test error;";
  code = exprEvaluate(result, EXPR("SQRT", CONST("abc")));
  EXPECT_EQ(RC_ERROR, code);

  code = exprEvaluate(result, EXPR("SQRT", CONST("-4")));
  EXPECT_EQ(RC_ERROR, code);
}

TEST(math_expr, bin) {
  LOG(INFO) << "test bin(n)";

  PbVariant result;

  LOG(INFO) << "1. test bin(15);";
  auto code = exprEvaluate(result, EXPR("BIN", CONST("15", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("1111", (string) result);

  LOG(INFO) << "2. test bin(-1);";
  code = exprEvaluate(result, EXPR("BIN", CONST("-1", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("1111111111111111111111111111111111111111111111111111111111111111", (string) result);
}

TEST(math_expr, hex) {
  LOG(INFO) << "test hex(int n), hex(string), unhex(string)";

  PbVariant result;

  LOG(INFO) << "1. test hex(15);";
  auto code = exprEvaluate(result, EXPR("HEX", CONST("15", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("F", (string) result);

  LOG(INFO) << "2. test hex(-1);";
  code = exprEvaluate(result, EXPR("HEX", CONST("-1", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("FFFFFFFFFFFFFFFF", (string) result);

  LOG(INFO) << "3. test hex('ABC');";
  code = exprEvaluate(result, EXPR("HEX", CONST("ABC")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("414243", (string) result);

  LOG(INFO) << "4. test unhex(hex('ABC'));";
  code = exprEvaluate(result, EXPR("UNHEX", EXPR("HEX", CONST("ABC"))));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("ABC", (string) result);
}

TEST(math_expr, conv) {
  LOG(INFO) << "test conv(string num, int from_base, int to_base)";

  PbVariant result;

  LOG(INFO) << "1. test conv('255', 10, 16);";
  auto code = exprEvaluate(result, EXPR("CONV", CONST("255"), CONST("10", INT32), CONST("16", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("FF", (string) result);

  LOG(INFO) << "2. test conv('11111111', 2, 16);";
  code = exprEvaluate(result, EXPR("CONV", CONST("11111111"), CONST("2", INT32), CONST("16", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("FF", (string) result);

  LOG(INFO) << "3. test conv('FF', 16, 8);";
  code = exprEvaluate(result, EXPR("CONV", CONST("FF"), CONST("16", INT32), CONST("8", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("377", (string) result);

  LOG(INFO) << "4. test conv('377', 8, 10);";
  code = exprEvaluate(result, EXPR("CONV", CONST("377"), CONST("8", INT32), CONST("10", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("255", (string) result);
}

TEST(math_expr, abs) {
  LOG(INFO) << "test abs(num)";

  PbVariant result;

  LOG(INFO) << "1. test abs(100);";
  auto code = exprEvaluate(result, EXPR("ABS", CONST("100", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(100, (int32_t) result);

  LOG(INFO) << "2. test abs(-100);";
  code = exprEvaluate(result, EXPR("ABS", CONST("-100", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(100, (int32_t) result);

  LOG(INFO) << "3. test abs(-100.123);";
  code = exprEvaluate(result, EXPR("ABS", CONST("-100.123", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(100.123, (double) result);
}

TEST(math_expr, pmod) {
  LOG(INFO) << "test pmod(int a, int b), pmod(double a, double b)";

  PbVariant result;

  LOG(INFO) << "1. test pmod(100, 11);";
  auto code = exprEvaluate(result, EXPR("PMOD", CONST("100", INT32), CONST("11", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(1, (int32_t) result);

  LOG(INFO) << "2. test pmod(100, -11);";
  code = exprEvaluate(result, EXPR("PMOD", CONST("-100", INT32), CONST("-11", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(1, (int32_t) result);

  LOG(INFO) << "3. test pmod(-21.5, 3);";
  code = exprEvaluate(result, EXPR("PMOD", CONST("-21.5", DOUBLE), CONST("3", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(0.5, (double) result);

  LOG(INFO) << "4. test pmod(21, 0);";
  code = exprEvaluate(result, EXPR("PMOD", CONST("21", INT32), CONST("0", INT32)));
  EXPECT_EQ(RC_ERROR, code);
}

TEST(math_expr, trigonometric_function) {
  LOG(INFO) << "test sin, asin, cos, acos";

  PbVariant result;

  LOG(INFO) << "1. test sin(0.5235987755982989) 30 degree;";
  auto code = exprEvaluate(result, EXPR("SIN", CONST("0.5235987755982989", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(0.5, (double) result);

  LOG(INFO) << "2. test asin(0.5);";
  code = exprEvaluate(result, EXPR("ASIN", CONST("0.5", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(0.5235987755982989, (double) result);

  LOG(INFO) << "3. test cos(1.0471975511965979) 60 degree;";
  code = exprEvaluate(result, EXPR("COS", CONST("1.0471975511965979", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(0.5, (double) result);

  LOG(INFO) << "4. test acos(0.5);";
  code = exprEvaluate(result, EXPR("ACOS", CONST("0.5", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(1.0471975511965979, (double) result);
}

TEST(math_expr, positive) {
  LOG(INFO) << "test positive(int a), positive(double a)";

  PbVariant result;

  LOG(INFO) << "1. test positive(100);";
  auto code = exprEvaluate(result, EXPR("POSITIVE", CONST("100", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(100, (int64_t) result);

  LOG(INFO) << "2. test positive(-100.125);";
  code = exprEvaluate(result, EXPR("POSITIVE", CONST("-100.125", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(-100.125, (double) result);
}

TEST(math_expr, negative) {
  LOG(INFO) << "test positive(int a), positive(double a)";

  PbVariant result;

  LOG(INFO) << "1. test positive(100);";
  auto code = exprEvaluate(result, EXPR("NEGATIVE", CONST("100", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(-100, (int64_t) result);

  LOG(INFO) << "2. test positive(-100.125);";
  code = exprEvaluate(result, EXPR("NEGATIVE", CONST("-100.125", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(100.125, (double) result);
}

TEST(math_expr, degrees) {
  LOG(INFO) << "test degrees(double a), radians(double a)";

  PbVariant result;

  LOG(INFO) << "1. test degrees(100);";
  auto code = exprEvaluate(result, EXPR("DEGREES", CONST("100", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(100 * 180.0 / M_PI, (double) result);

  LOG(INFO) << "2. test radians(100);";
  code = exprEvaluate(result, EXPR("RADIANS", CONST("100", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(100 / 180.0 * M_PI, (double) result);
}

TEST(math_expr, sign) {
  LOG(INFO) << "test sign(double a)";

  PbVariant result;

  LOG(INFO) << "1. test sign(100);";
  auto code = exprEvaluate(result, EXPR("SIGN", CONST("100", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(1, (int32_t) result);

  LOG(INFO) << "2. test sign(-100.125);";
  code = exprEvaluate(result, EXPR("SIGN", CONST("-100.125", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(-1, (int32_t) result);

  LOG(INFO) << "3. test sign(0);";
  code = exprEvaluate(result, EXPR("SIGN", CONST("0", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(0, (int32_t) result);
}
