/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include <gtest/gtest.h>
#include "expr_common.h"
#include "idgs/expr/expression_helper.h"

using namespace std;
using namespace idgs;
using namespace idgs::expr;
using namespace idgs::pb;
using namespace protobuf;

TEST(cast_expr, to_string) {
  ExpressionFactory::init();

  LOG(INFO) << "test cast as string";

  PbVariant result;

  LOG(INFO) << "1. test cast(10000 as string);";
  auto code = exprEvaluate(result, EXPR("UDFTOSTRING", CONST("10000", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("10000", (string) result);

  LOG(INFO) << "2. test cast(-10000 as string);";
  code = exprEvaluate(result, EXPR("UDFTOSTRING", CONST("-10000", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("-10000", (string) result);

  LOG(INFO) << "3. test cast(100.123 as string);";
  code = exprEvaluate(result, EXPR("UDFTOSTRING", CONST("100.123", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("100.123", (string) result);

  LOG(INFO) << "4. test cast(100.0 as string);";
  code = exprEvaluate(result, EXPR("UDFTOSTRING", CONST("100", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("100.0", (string) result);
}

TEST(cast_expr, to_long) {
  LOG(INFO) << "test cast as bigint";

  PbVariant result;

  LOG(INFO) << "1. test cast(10000 as bigint);";
  auto code = exprEvaluate(result, EXPR("UDFTOLONG", CONST("10000", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(10000, (int64_t) result);

  LOG(INFO) << "2. test cast(-10000 as bigint);";
  code = exprEvaluate(result, EXPR("UDFTOLONG", CONST("-10000", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(-10000, (int64_t) result);

  LOG(INFO) << "3. test cast(100.123 as bigint);";
  code = exprEvaluate(result, EXPR("UDFTOLONG", CONST("100.123", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(100, (int64_t) result);

  LOG(INFO) << "4. test cast('12345' as bigint);";
  code = exprEvaluate(result, EXPR("UDFTOLONG", CONST("12345")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(12345, (int64_t) result);
}

TEST(cast_expr, to_int) {
  LOG(INFO) << "test cast as int";

  PbVariant result;

  LOG(INFO) << "1. test cast(10000 as int);";
  auto code = exprEvaluate(result, EXPR("UDFTOINTEGER", CONST("10000", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(10000, (int32_t) result);

  LOG(INFO) << "2. test cast(-10000 as int);";
  code = exprEvaluate(result, EXPR("UDFTOINTEGER", CONST("-10000", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(-10000, (int32_t) result);

  LOG(INFO) << "3. test cast(100.123 as int);";
  code = exprEvaluate(result, EXPR("UDFTOINTEGER", CONST("100.123", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(100, (int32_t) result);

  LOG(INFO) << "4. test cast('12345' as int);";
  code = exprEvaluate(result, EXPR("UDFTOINTEGER", CONST("12345")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(12345, (int32_t) result);
}

TEST(cast_expr, to_short) {
  LOG(INFO) << "test cast as smallint";

  PbVariant result;

  LOG(INFO) << "1. test cast(10000 as smallint);";
  auto code = exprEvaluate(result, EXPR("UDFTOSHORT", CONST("10000", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(10000, (int32_t) result);

  LOG(INFO) << "2. test cast(-10000 as smallint);";
  code = exprEvaluate(result, EXPR("UDFTOSHORT", CONST("-10000", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(-10000, (int32_t) result);

  LOG(INFO) << "3. test cast(100.123 as smallint);";
  code = exprEvaluate(result, EXPR("UDFTOSHORT", CONST("100.123", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(100, (int32_t) result);

  LOG(INFO) << "4. test cast('12345' as smallint);";
  code = exprEvaluate(result, EXPR("UDFTOSHORT", CONST("12345")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(12345, (int32_t) result);
}

TEST(cast_expr, to_byte) {
  LOG(INFO) << "test cast as titnyint";

  PbVariant result;

  LOG(INFO) << "1. test cast(10000 as titnyint);";
  auto code = exprEvaluate(result, EXPR("UDFTOBYTE", CONST("10000", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(10000, (int32_t) result);

  LOG(INFO) << "2. test cast(-10000 as titnyint);";
  code = exprEvaluate(result, EXPR("UDFTOBYTE", CONST("-10000", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(-10000, (int32_t) result);

  LOG(INFO) << "3. test cast(100.123 as titnyint);";
  code = exprEvaluate(result, EXPR("UDFTOBYTE", CONST("100.123", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(100, (int32_t) result);

  LOG(INFO) << "4. test cast('12345' as titnyint);";
  code = exprEvaluate(result, EXPR("UDFTOBYTE", CONST("12345")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(12345, (int32_t) result);
}

TEST(cast_expr, to_float) {
  LOG(INFO) << "test cast as float";

  PbVariant result;

  LOG(INFO) << "1. test cast(10000 as float);";
  auto code = exprEvaluate(result, EXPR("UDFTOFLOAT", CONST("10000", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_FLOAT_EQ(10000, (float) result);

  LOG(INFO) << "2. test cast(-10000 as float);";
  code = exprEvaluate(result, EXPR("UDFTOFLOAT", CONST("-10000", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_FLOAT_EQ(-10000, (float) result);

  LOG(INFO) << "3. test cast(100.123 as float);";
  code = exprEvaluate(result, EXPR("UDFTOFLOAT", CONST("100.123", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_FLOAT_EQ(100.123, (float) result);

  LOG(INFO) << "4. test cast('12345' as float);";
  code = exprEvaluate(result, EXPR("UDFTOFLOAT", CONST("12345")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_FLOAT_EQ(12345, (float) result);
}

TEST(cast_expr, to_double) {
  LOG(INFO) << "test cast as double";

  PbVariant result;

  LOG(INFO) << "1. test cast(10000 as double);";
  auto code = exprEvaluate(result, EXPR("UDFTODOUBLE", CONST("10000", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(10000, (double) result);

  LOG(INFO) << "2. test cast(-10000 as double);";
  code = exprEvaluate(result, EXPR("UDFTODOUBLE", CONST("-10000", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(-10000, (double) result);

  LOG(INFO) << "3. test cast(100.123 as double);";
  code = exprEvaluate(result, EXPR("UDFTODOUBLE", CONST("100.123", DOUBLE)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(100.123, (double) result);

  LOG(INFO) << "4. test cast('12345' as double);";
  code = exprEvaluate(result, EXPR("UDFTODOUBLE", CONST("12345")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(12345, (double) result);
}

TEST(cast_expr, to_bool) {
  LOG(INFO) << "test cast as bool";

  PbVariant result;

  LOG(INFO) << "1. test cast(1 as bool);";
  auto code = exprEvaluate(result, EXPR("UDFTOBOOLEAN", CONST("1", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_TRUE((bool) result);

  LOG(INFO) << "2. test cast(0 as bool);";
  code = exprEvaluate(result, EXPR("UDFTOBOOLEAN", CONST("0", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_FALSE((bool) result);

  LOG(INFO) << "3. test cast('5' as bool);";
  code = exprEvaluate(result, EXPR("UDFTOBOOLEAN", CONST("5")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_TRUE((bool) result);
}

TEST(cast_expr, to_binary) {
  LOG(INFO) << "test cast as binary";

  PbVariant result;

  LOG(INFO) << "1. test cast('abc' as binary);";
  auto code = exprEvaluate(result, EXPR("UDFTOBINARY", CONST("abc")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("abc", (string) result);

  LOG(INFO) << "2. test cast('abc' as binary(2));";
  code = exprEvaluate(result, EXPR("UDFTOBINARY", CONST("abc"), CONST("2", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("ab", (string) result);
}

TEST(cast_expr, to_decimal) {
  LOG(INFO) << "test cast as decimal";

  PbVariant result;

  LOG(INFO) << "1. test cast(10000 as decimal);";
  auto code = exprEvaluate(result, EXPR("UDFTODECIMAL", CONST("10000", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(10000, (int64_t) result);

  LOG(INFO) << "2. test cast(-10000 as decimal);";
  code = exprEvaluate(result, EXPR("UDFTODECIMAL", CONST("-10000", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(-10000, (int32_t) result);

  LOG(INFO) << "3. test cast('100.123' as decimal);";
  code = exprEvaluate(result, EXPR("UDFTODECIMAL", CONST("100.123")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_DOUBLE_EQ(100.123, (double) result);
}
