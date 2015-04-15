/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once


namespace idgs {
namespace client {
namespace rdd {

inline idgs::pb::Expr* Const(const char* value, idgs::pb::DataType type = idgs::pb::STRING) {
  idgs::pb::Expr* expr = new idgs::pb::Expr;
  expr->set_name("CONST");
  expr->set_const_type(type);
  expr->set_value(value);
  return expr;
}

inline idgs::pb::Expr* Field(const char* value) {
  idgs::pb::Expr* expr = new idgs::pb::Expr;
  expr->set_name("FIELD");
  expr->set_value(value);
  return expr;
}

inline idgs::pb::Expr* NAry_Expression(const std::string& type, std::vector<idgs::pb::Expr*> args) {
  idgs::pb::Expr* expr = new idgs::pb::Expr;
  expr->set_name(type);

  for (idgs::pb::Expr* arg : args) {
    idgs::pb::Expr* new_expr = expr->add_expression();
    new_expr->Swap(arg);
    delete arg;
  }

  return expr;
}

} // namesapce rdd
} // namespace client
} // namespace idgs

#define EXPR(EXPR_TYPE, ...)  idgs::client::rdd::NAry_Expression(EXPR_TYPE,  { __VA_ARGS__ })

#define MOVE_EXPR(TARGET, SOURCE) \
  do { \
    idgs::pb::Expr* exp = SOURCE; \
    TARGET->Swap(exp); \
    delete exp; \
  } while(0)


#define CONST(...)      idgs::client::rdd::Const( __VA_ARGS__ )
#define FIELD(...)      idgs::client::rdd::Field( __VA_ARGS__ )

#define AND(...)          idgs::client::rdd::NAry_Expression("AND",      { __VA_ARGS__ })
#define OR(...)           idgs::client::rdd::NAry_Expression("OR",       { __VA_ARGS__ })
#define NOT(...)          idgs::client::rdd::NAry_Expression("NOT",      { __VA_ARGS__ })

#define EQ(...)           idgs::client::rdd::NAry_Expression("EQ",       { __VA_ARGS__ })
#define NE(...)           idgs::client::rdd::NAry_Expression("NE",       { __VA_ARGS__ })
#define LT(...)           idgs::client::rdd::NAry_Expression("LT",       { __VA_ARGS__ })
#define LE(...)           idgs::client::rdd::NAry_Expression("LE",       { __VA_ARGS__ })
#define GT(...)           idgs::client::rdd::NAry_Expression("GT",       { __VA_ARGS__ })
#define GE(...)           idgs::client::rdd::NAry_Expression("GE",       { __VA_ARGS__ })

#define IF(...)           idgs::client::rdd::NAry_Expression("IF",       { __VA_ARGS__ })

#define LIKE(...)         idgs::client::rdd::NAry_Expression("LIKE",     { __VA_ARGS__ })
#define SUBSTR(...)       idgs::client::rdd::NAry_Expression("SUBSTR",   { __VA_ARGS__ })

#define ADD(...)          idgs::client::rdd::NAry_Expression("ADD",      { __VA_ARGS__ })
#define SUBTRACT(...)     idgs::client::rdd::NAry_Expression("SUBTRACT", { __VA_ARGS__ })
#define MULTIPLY(...)     idgs::client::rdd::NAry_Expression("MULTIPLY", { __VA_ARGS__ })
#define DIVIDE(...)       idgs::client::rdd::NAry_Expression("DIVIDE",   { __VA_ARGS__ })
#define MODULUS(...)      idgs::client::rdd::NAry_Expression("MODULUS",  { __VA_ARGS__ })

#define BIT_AND(...)      idgs::client::rdd::NAry_Expression("BIT_AND",  { __VA_ARGS__ })
#define BIT_OR(...)       idgs::client::rdd::NAry_Expression("BIT_OR",   { __VA_ARGS__ })
#define BIT_NOT(...)      idgs::client::rdd::NAry_Expression("BIT_NOT",  { __VA_ARGS__ })
#define BIT_XOR(...)      idgs::client::rdd::NAry_Expression("BIT_XOR",  { __VA_ARGS__ })

