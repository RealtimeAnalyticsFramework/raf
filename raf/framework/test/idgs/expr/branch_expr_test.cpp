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
#include "protobuf/message_helper.h"

using namespace std;
using namespace idgs;
using namespace idgs::expr;
using namespace protobuf;

TEST(branch_expr, if_expr) {
  ExpressionFactory::init();

  LOG(INFO) << "test if(cond, v1, v2)";

  PbVariant result;

  LOG(INFO) << "1. test if(a == 1, 'true', 'false'); a = 1";
  auto code = exprEvaluate(result, EXPR("IF", EQ(CONST("1"), CONST("1")), CONST("true"), CONST("false")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("true", (string) result);

  LOG(INFO) << "2. test if (a == 1, 'true', 'false'); a = 2";
  code = exprEvaluate(result, EXPR("IF", EQ(CONST("2"), CONST("1")), CONST("true"), CONST("false")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("false", (string) result);
}

TEST(branch_expr, when_expr) {
  LOG(INFO) << "test when(cond1, v1, cond2, v2... condN, vN, default)";

  PbVariant result;

  LOG(INFO) << "1. test when(a > 0, '1', a == 0, '0', '-1'); a = 10";
  auto code = exprEvaluate(result, EXPR("WHEN", GT(CONST("10"), CONST("0")), CONST("1"), EQ(CONST("10"), CONST("0")), CONST("0"), CONST("-1")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("1", (string) result);

  LOG(INFO) << "2. test when(a > 0, '1', a == 0, '0', '-1'); a = 0";
  code = exprEvaluate(result, EXPR("WHEN", GT(CONST("0"), CONST("0")), CONST("1"), EQ(CONST("0"), CONST("0")), CONST("0"), CONST("-1")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("0", (string) result);

  LOG(INFO) << "3. test when(a > 0, '1', a == 0, '0', '-1'); a = -5";
  code = exprEvaluate(result, EXPR("WHEN", GT(CONST("-5"), CONST("0")), CONST("1"), EQ(CONST("-5"), CONST("0")), CONST("0"), CONST("-1")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("-1", (string) result);
}

TEST(branch_expr, case_expr) {
  LOG(INFO) << "test case(expr, value1, v1, value2, v2... valueN, vN, default)";

  PbVariant result;

  LOG(INFO) << "1. test case(a, 2014, 'this year', 2013, 'last year', 'following year'); a = 2014";
  auto code = exprEvaluate(result, EXPR("CASE", CONST("2014"), CONST("2014"), CONST("this year"), CONST("2013"), CONST("last year"), CONST("following year")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("this year", (string) result);

  LOG(INFO) << "2. test case(a, 2014, 'this year', 2013, 'last year', 'following year'); a = 2013";
  code = exprEvaluate(result, EXPR("CASE", CONST("2013"), CONST("2014"), CONST("this year"), CONST("2013"), CONST("last year"), CONST("following year")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("last year", (string) result);

  LOG(INFO) << "3. test case(a, 2014, 'this year', 2013, 'last year', 'following year'); a = 2015";
  code = exprEvaluate(result, EXPR("CASE", CONST("2015"), CONST("2014"), CONST("this year"), CONST("2013"), CONST("last year"), CONST("following year")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("following year", (string) result);
}

TEST(branch_expr, nvl) {
  LOG(INFO) << "test nvl(expr, default)";

  PbVariant result;
  ExpressionContext ctx;

  MessageHelper helper;
  string proto = "package idgs.expr.test;message ExprTestKey {required int32 id = 1;}message ExprTest {optional string name = 1;}";
  auto code = helper.registerDynamicMessageFromString(proto);
  EXPECT_EQ(RC_SUCCESS, code);

  auto key = helper.createMessage("idgs.expr.test.ExprTestKey");
  auto value = helper.createMessage("idgs.expr.test.ExprTest");

  helper.setMessageValue(key.get(), "id", PbVariant(1000));

  DVLOG(5) << "refcnt of key: " << key.use_count();
  DVLOG(5) << "refcnt of value: " << value.use_count();

  ctx.setKeyValue(&key, &value);
  DVLOG(5) << "refcnt of key: " << key.use_count();
  DVLOG(5) << "refcnt of value: " << value.use_count();

  LOG(INFO) << "1. test nvl(name, 'no name');";
  code = exprEvaluate(result, EXPR("NVL", FIELD("name"), CONST("no name")), &ctx);
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("no name", (string) result);

  helper.setMessageValue(value.get(), "name", PbVariant(string("has name")));
  LOG(INFO) << "2. test nvl(name, 'no name');";
  code = exprEvaluate(result, EXPR("NVL", FIELD("name"), CONST("no name")), &ctx);
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("has name", (string) result);

  DVLOG(5) << "refcnt of key: " << key.use_count();
  DVLOG(5) << "refcnt of value: " << value.use_count();

//  DVLOG(5) << "refcnt of context key: " << ctx.getKey()->use_count();
//  ctx.getKey()->reset();
//  DVLOG(5) << "refcnt of context key: " << ctx.getKey()->use_count();
//  DVLOG(5) << "refcnt of key: " << key.use_count();

  value->Clear();
  key.reset();
  value.reset();
  DVLOG(5) << "refcnt of key: " << key.use_count();
  DVLOG(5) << "refcnt of context key: " << ctx.getKey()->use_count();
  DVLOG(5) << "refcnt of value: " << value.use_count();
}

TEST(branch_expr, coalesc) {
  LOG(INFO) << "test coalesc(v1, v2, ... vN)";

  PbVariant result;
  ExpressionContext ctx;

  MessageHelper helper;
  string proto = "package idgs.expr.test;message ExprTestKey {required int32 id = 1;}message ExprTest {optional string name = 1;optional string alias = 2;optional string birth = 3;}";
  auto code = helper.registerDynamicMessageFromString(proto);
  EXPECT_EQ(RC_SUCCESS, code);

  auto key = helper.createMessage("idgs.expr.test.ExprTestKey");
  auto value = helper.createMessage("idgs.expr.test.ExprTest");

  helper.setMessageValue(key.get(), "id", PbVariant(1000));
  ctx.setKeyValue(&key, &value);

  LOG(INFO) << "1. test nvl(name, alias, birth);";
  code = exprEvaluate(result, EXPR("COALESCE", FIELD("name"), FIELD("alias"), FIELD("birth")), &ctx);
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("null", (string) result);

  helper.setMessageValue(value.get(), "birth", PbVariant(string("birth")));
  LOG(INFO) << "2. test nvl(name, alias, birth);";
  code = exprEvaluate(result, EXPR("COALESCE", FIELD("name"), FIELD("alias"), FIELD("birth")), &ctx);
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("birth", (string) result);

  helper.setMessageValue(value.get(), "alias", PbVariant(string("alias")));
  LOG(INFO) << "3. test nvl(name, alias, birth);";
  code = exprEvaluate(result, EXPR("COALESCE", FIELD("name"), FIELD("alias"), FIELD("birth")), &ctx);
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("alias", (string) result);

  helper.setMessageValue(value.get(), "name", PbVariant(string("name")));
  LOG(INFO) << "4. test nvl(name, alias, birth);";
  code = exprEvaluate(result, EXPR("COALESCE", FIELD("name"), FIELD("alias"), FIELD("birth")), &ctx);
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("name", (string) result);
}


TEST(branch_expr, destroy) {
  google::protobuf::ShutdownProtobufLibrary();
}
