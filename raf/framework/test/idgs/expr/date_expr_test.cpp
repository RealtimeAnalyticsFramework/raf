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

TEST(date_expr, from_unixtime) {
  ExpressionFactory::init();

  LOG(INFO) << "test from_unixtime(bigint unixtime[, string format])";

  PbVariant result;

  LOG(INFO) << "1. test from_unixtime(1394522569);";
  auto code = exprEvaluate(result, EXPR("FROM_UNIXTIME", CONST("1394522569", INT64)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("2014-03-11 15:22:49", (string) result);

  LOG(INFO) << "2. test from_unixtime(1394522569, '%Y-%m-%d %H:%M:%S');";
  code = exprEvaluate(result, EXPR("FROM_UNIXTIME", CONST("1394522569", INT64), CONST("%Y-%m-%d %H:%M:%S")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("2014-03-11 15:22:49", (string) result);

  LOG(INFO) << "3. test from_unixtime(1394522569. '%c');";
  code = exprEvaluate(result, EXPR("FROM_UNIXTIME", CONST("1394522569", INT64), CONST("%c")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("Tue Mar 11 15:22:49 2014", (string) result);
}

TEST(date_expr, unix_timestamp) {
  LOG(INFO) << "test unix_timestamp([string date[, string format]])";

  PbVariant result;

  LOG(INFO) << "1. test unix_timestamp();";
  auto code = exprEvaluate(result, EXPR("UNIX_TIMESTAMP"));
  EXPECT_EQ(RC_SUCCESS, code);
  LOG(INFO) << "current timestamp is " << result.toString();

  LOG(INFO) << "2. test unix_timestamp('2014-03-11 15:22:49');";
  code = exprEvaluate(result, EXPR("UNIX_TIMESTAMP", CONST("2014-03-11 15:22:49")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("1394522569", (string) result);
}

TEST(date_expr, to_date) {
  LOG(INFO) << "test to_date(string date)";

  PbVariant result;

  LOG(INFO) << "1. test to_date('2014-03-11 15:22:49');";
  auto code = exprEvaluate(result, EXPR("TO_DATE", CONST("2014-03-11 15:22:49")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("2014-03-11", (string) result);
}

TEST(date_expr, get) {
  LOG(INFO) << "test year, month, day, dayofmonth, hour, minute, second";

  PbVariant result;

  LOG(INFO) << "1. test year('2014-03-11 15:22:49');";
  auto code = exprEvaluate(result, EXPR("YEAR", CONST("2014-03-11 15:22:49")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(2014, (int32_t) result);

  LOG(INFO) << "2. test month('2014-03-11 15:22:49');";
  code = exprEvaluate(result, EXPR("MONTH", CONST("2014-03-11 15:22:49")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(3, (int32_t) result);

  LOG(INFO) << "3. test day('2014-03-11 15:22:49');";
  code = exprEvaluate(result, EXPR("DAY", CONST("2014-03-11 15:22:49")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(11, (int32_t) result);

  LOG(INFO) << "4. test dayofmonth('2014-03-11 15:22:49');";
  code = exprEvaluate(result, EXPR("DAYOFMONTH", CONST("2014-03-11 15:22:49")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(11, (int32_t) result);

  LOG(INFO) << "5. test hour('2014-03-11 15:22:49');";
  code = exprEvaluate(result, EXPR("HOUR", CONST("2014-03-11 15:22:49")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(15, (int32_t) result);

  LOG(INFO) << "6. test minute('2014-03-11 15:22:49');";
  code = exprEvaluate(result, EXPR("MINUTE", CONST("2014-03-11 15:22:49")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(22, (int32_t) result);

  LOG(INFO) << "7. test second('2014-03-11 15:22:49');";
  code = exprEvaluate(result, EXPR("SECOND", CONST("2014-03-11 15:22:49")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(49, (int32_t) result);
}

TEST(date_expr, weekofyear) {
  LOG(INFO) << "test weekofyear(string date)";

  PbVariant result;

  LOG(INFO) << "1. test weekofyear('2014-03-11 15:22:49');";
  auto code = exprEvaluate(result, EXPR("WEEKOFYEAR", CONST("2014-03-11 15:22:49")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(11, (int32_t) result);
}

TEST(date_expr, datedif) {
  LOG(INFO) << "test datediff(string enddate, string startdate)";

  PbVariant result;

  LOG(INFO) << "1. test datediff('2014-04-11 00:00:00', '2014-03-11 15:22:49');";
  auto code = exprEvaluate(result, EXPR("DATEDIFF", CONST("2014-04-11 00:00:00"), CONST("2014-03-11 15:22:49")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(31, (int32_t) result);

  LOG(INFO) << "2. test datediff('2014-03-11 15:22:49', '2014-04-11 00:00:00');";
  code = exprEvaluate(result, EXPR("DATEDIFF", CONST("2014-03-11 15:22:49"), CONST("2014-04-11 00:00:00")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(-31, (int32_t) result);

  LOG(INFO) << "3. test datediff('2014-03-11 15:22:49', '2014-03-11');";
  code = exprEvaluate(result, EXPR("DATEDIFF", CONST("2014-03-11 15:22:49"), CONST("2014-03-11")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(0, (int32_t) result);

  LOG(INFO) << "4. test datediff('2014-03-14', '2013-12-14');";
  code = exprEvaluate(result, EXPR("DATEDIFF", CONST("2014-03-14"), CONST("2013-12-14")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(90, (int32_t) result);
}

TEST(date_expr, dateadd_datesub) {
  LOG(INFO) << "test date_add(string startdate, int days), date_sub(string startdate, int days)";

  PbVariant result;

  LOG(INFO) << "1. test date_add('2013-12-11 15:22:49', 31);";
  auto code = exprEvaluate(result, EXPR("DATE_ADD", CONST("2013-12-11 15:22:49"), CONST("31", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("2014-01-11", (string) result);

  LOG(INFO) << "2. test date_add('2014-01-11', -31);";
  code = exprEvaluate(result, EXPR("DATE_ADD", CONST("2014-01-11"), CONST("-31", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("2013-12-11", (string) result);

  LOG(INFO) << "3. test date_sub('2013-12-11 15:22:49', -31);";
  code = exprEvaluate(result, EXPR("DATE_SUB", CONST("2013-12-11 15:22:49"), CONST("-31" , INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("2014-01-11", (string) result);

  LOG(INFO) << "4. test date_sub('2014-01-11', 31);";
  code = exprEvaluate(result, EXPR("DATE_SUB", CONST("2014-01-11"), CONST("31", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("2013-12-11", (string) result);

  LOG(INFO) << "5. test date_add('2014-01-01', 1096);";
  code = exprEvaluate(result, EXPR("DATE_ADD", CONST("2014-01-01"), CONST("1096", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("2017-01-01", (string) result);
}
