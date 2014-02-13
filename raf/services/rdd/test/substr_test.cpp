
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include <gtest/gtest.h>
#include "idgs/expr/expression_factory.h"
#include "idgs/expr/string_expr.h"
#include "idgs/expr/const_expr.h"
#include "idgs/client/rdd/rdd_expression_helper.h"

using namespace idgs::actor;
using namespace idgs::expr;

TEST(rdd, str) {
  ExpressionFactory::init();
  std::string var = "hello, world";
  std::shared_ptr<idgs::pb::Expr> entry_expr(E_CONST("hello, world"));
  LOG(INFO) << "parse expression: " << entry_expr->DebugString();

  Expression* expr1 = new ConstExpression;
  PbMessagePtr key(NULL);
  PbMessagePtr value(NULL);
  bool res = false;
  res = expr1->parse(*entry_expr, key, value);
  EXPECT_EQ(true, res);

  LOG(INFO) << "evaluate expression";
  protobuf::PbVariant result = expr1->evaluate(NULL);
  LOG(INFO) << result.debugString();
 }


TEST(rdd, substr) {
  ExpressionFactory::init();
  std::string var = "hello, world";

  std::shared_ptr<idgs::pb::Expr> entry_expr(E_SUBSTR(E_CONST("hello, world"), E_CONST("2", idgs::pb::INT32)));
  LOG(INFO) << "parse expression: " << entry_expr->DebugString();

  Expression* expr1 = new SubStrExpression;
  PbMessagePtr key(NULL);
  PbMessagePtr value(NULL);
  bool res = false;
  res = expr1->parse(*entry_expr, key, value);
  EXPECT_EQ(true, res);

  auto result = expr1->evaluate(NULL);
  EXPECT_EQ(var.substr(2), result.toString());
  LOG(INFO) << "###########" << var.substr(2) << "###########";
}

TEST(rdd, substr2) {
  ExpressionFactory::init();
  std::string var = "hello, world";
  std::shared_ptr<idgs::pb::Expr> entry_expr(E_SUBSTR(E_CONST("hello, world"), E_CONST("2", idgs::pb::INT32), E_CONST("4", idgs::pb::INT32)));
  LOG(INFO) << "parse expression: " << entry_expr->DebugString();
  Expression* expr2 = new SubStrExpression;
  bool res = false;
  PbMessagePtr key(NULL);
  PbMessagePtr value(NULL);
  res = expr2->parse(*entry_expr, key, value);
  EXPECT_EQ(true, res);

  auto result = expr2->evaluate(NULL);
  EXPECT_EQ(var.substr(2, 4), result.toString());
  LOG(INFO) << "###########" << var.substr(2, 4) << "###########";

}
