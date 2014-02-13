/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include "idgs/pb/expr.pb.h"

namespace idgs {
namespace client {
namespace rdd {

inline idgs::pb::Expr* Const(const char* value, idgs::pb::DataType type = idgs::pb::STRING) {
  idgs::pb::Expr* expr = new idgs::pb::Expr;
  expr->set_type(idgs::pb::CONST);
  expr->set_const_type(type);
  expr->set_value(value);
  return expr;
}

inline idgs::pb::Expr* Field(const char* value) {
  idgs::pb::Expr* expr = new idgs::pb::Expr;
  expr->set_type(idgs::pb::FIELD);
  expr->set_value(value);
  return expr;
}

inline idgs::pb::Expr* NAry_Expression(idgs::pb::ExpressionType type, std::vector<idgs::pb::Expr*> args) {
  idgs::pb::Expr* expr = new idgs::pb::Expr;
  expr->set_type(type);

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

#define E_CONST(...)      idgs::client::rdd::Const( __VA_ARGS__ )
#define E_FIELD(...)      idgs::client::rdd::Field( __VA_ARGS__ )

#define E_AND(...)      idgs::client::rdd::NAry_Expression(idgs::pb::AND,      { __VA_ARGS__ })
#define E_OR(...)       idgs::client::rdd::NAry_Expression(idgs::pb::OR,       { __VA_ARGS__ })
#define E_NOT(...)      idgs::client::rdd::NAry_Expression(idgs::pb::NOT,      { __VA_ARGS__ })
#define E_EQ(...)       idgs::client::rdd::NAry_Expression(idgs::pb::EQ,       { __VA_ARGS__ })
#define E_NE(...)       idgs::client::rdd::NAry_Expression(idgs::pb::NE,       { __VA_ARGS__ })
#define E_LT(...)       idgs::client::rdd::NAry_Expression(idgs::pb::LT,       { __VA_ARGS__ })
#define E_LE(...)       idgs::client::rdd::NAry_Expression(idgs::pb::LE,       { __VA_ARGS__ })
#define E_GT(...)       idgs::client::rdd::NAry_Expression(idgs::pb::GT,       { __VA_ARGS__ })
#define E_GE(...)       idgs::client::rdd::NAry_Expression(idgs::pb::GE,       { __VA_ARGS__ })
#define E_IF(...)       idgs::client::rdd::NAry_Expression(idgs::pb::IF,       { __VA_ARGS__ })
#define E_LIKE(...)     idgs::client::rdd::NAry_Expression(idgs::pb::LIKE,     { __VA_ARGS__ })
#define E_SUBSTR(...)   idgs::client::rdd::NAry_Expression(idgs::pb::SUBSTR,   { __VA_ARGS__ })
#define E_ADD(...)      idgs::client::rdd::NAry_Expression(idgs::pb::ADD,      { __VA_ARGS__ })
#define E_SUBTRACT(...) idgs::client::rdd::NAry_Expression(idgs::pb::SUBTRACT, { __VA_ARGS__ })
#define E_MULTIPLY(...) idgs::client::rdd::NAry_Expression(idgs::pb::MULTIPLY, { __VA_ARGS__ })
#define E_DIVIDE(...)   idgs::client::rdd::NAry_Expression(idgs::pb::DIVIDE,   { __VA_ARGS__ })
#define E_MODULUS(...)  idgs::client::rdd::NAry_Expression(idgs::pb::MODULUS,  { __VA_ARGS__ })
#define E_BIT_AND(...)  idgs::client::rdd::NAry_Expression(idgs::pb::BIT_AND,  { __VA_ARGS__ })
#define E_BIT_OR(...)   idgs::client::rdd::NAry_Expression(idgs::pb::BIT_OR,   { __VA_ARGS__ })
#define E_BIT_NOT(...)  idgs::client::rdd::NAry_Expression(idgs::pb::BIT_NOT,  { __VA_ARGS__ })
#define E_BIT_XOR(...)  idgs::client::rdd::NAry_Expression(idgs::pb::BIT_XOR,  { __VA_ARGS__ })
