
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "expression_factory.h"

#include "idgs/expr/branch_expr.h"
#include "idgs/expr/arithmetic_expr.h"
#include "idgs/expr/const_expr.h"
#include "idgs/expr/logical_expr.h"
#include "idgs/expr/compare_expr.h"
#include "idgs/expr/string_expr.h"
#include "idgs/expr/parsed_field_extractor.h"

using namespace idgs::actor;

namespace idgs {
namespace expr {

idgs::util::resource_manager<Expression*> ExpressionFactory::exprs;

void ExpressionFactory::init() {
  Expression* e;

  // const
  e = new ConstExpression();
  exprs.put(e->name(), e);

  // field
  e = new ParsedFieldExtractor();
  exprs.put(e->name(), e);

  // logical
  e = new AndExpression();
  exprs.put(e->name(), e);
  e = new OrExpression();
  exprs.put(e->name(), e);
  e = new NotExpression();
  exprs.put(e->name(), e);

  // compare
  e = new EQExpression();
  exprs.put(e->name(), e);
  e = new NEExpression();
  exprs.put(e->name(), e);
  e = new LTExpression();
  exprs.put(e->name(), e);
  e = new LEExpression();
  exprs.put(e->name(), e);
  e = new GTExpression();
  exprs.put(e->name(), e);
  e = new GEExpression();
  exprs.put(e->name(), e);
  e = new BetweenExpression();
  exprs.put(e->name(), e);

  // branch
  e = new IfExpression();
  exprs.put(e->name(), e);
  e = new VariableSetExpression();
  exprs.put(e->name(), e);
  e = new VariableGetExpression();
  exprs.put(e->name(), e);

  // string
  e = new LikeExpression();
  exprs.put(e->name(), e);
  e = new SubStrExpression();
  exprs.put(e->name(), e);

  // arithmetic
  e = new AddExpression();
  exprs.put(e->name(), e);
  e = new SubtractExpression();
  exprs.put(e->name(), e);
  e = new MultiplyExpression();
  exprs.put(e->name(), e);
  e = new DivideExpression();
  exprs.put(e->name(), e);
  e = new ModulusExpression();
  exprs.put(e->name(), e);
  e = new HashExpression();
  exprs.put(e->name(), e);

}

idgs::ResultCode ExpressionFactory::build(Expression** expression, const idgs::pb::Expr& entryExp,
    const PbMessagePtr& key, const PbMessagePtr& value) {
  assert(* expression == NULL);
  if (!entryExp.has_type()) {
    LOG(ERROR) << "Expr: no type.";
    return idgs::RC_UNKNOWN_OPERATION;
  }

  const std::string& exprName = idgs::pb::ExpressionType_Name(entryExp.type());
  Expression* expr = exprs.get(exprName);
  if (!expr) {
    LOG(ERROR) << "Expression not found: " << exprName;
    return idgs::RC_UNKNOWN_OPERATION;
  }

  *expression = expr->clone();
  if ((*expression)->parse(entryExp, key, value)) {
    return idgs::RC_SUCCESS;
  } else {
    delete (*expression);
    (*expression) = NULL;
    LOG(ERROR) << "Failed to parse.";
    return idgs::RC_UNKNOWN_OPERATION;
  }

  return idgs::RC_SUCCESS;
}

} /* namespace expr */
} /* namespace idgs */
