
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
#include "idgs/expr/math_expr.h"
#include "idgs/expr/date_expr.h"
#include "idgs/expr/cast_expr.h"
#include "idgs/expr/field_extractor.h"

using namespace idgs::actor;

namespace idgs {
namespace expr {

std::map<std::string, std::shared_ptr<Expression> > ExpressionFactory::exprs;

#define REG_EXPR(T)     registerExpression(std::make_shared<T>())


void ExpressionFactory::init() {

  // const
  REG_EXPR(ConstExpression);

  // field
  REG_EXPR(FieldExtractor);

  // logical
  REG_EXPR(AndExpression);
  REG_EXPR(OrExpression);
  REG_EXPR(NotExpression);

  // compare
  REG_EXPR(EQExpression);
  REG_EXPR(NEExpression);
  REG_EXPR(LTExpression);
  REG_EXPR(LEExpression);
  REG_EXPR(GTExpression);
  REG_EXPR(GEExpression);
  REG_EXPR(BetweenExpression);
  REG_EXPR(InExpression);
  REG_EXPR(IsNullExpression);
  REG_EXPR(IsNotNullExpression);

  // branch
  REG_EXPR(WhenExpression);
  REG_EXPR(CaseExpression);
  REG_EXPR(NvlExpression);
  REG_EXPR(CoalesceExpression);
  REG_EXPR(VariableSetExpression);
  REG_EXPR(VariableGetExpression);

  // string
  REG_EXPR(LikeExpression);
  REG_EXPR(SubStrExpression);
  REG_EXPR(LengthExpression);
  REG_EXPR(ReverseExpression);
  REG_EXPR(ConcatExpression);
  REG_EXPR(ConcatWSExpression);
  REG_EXPR(UpperExpression);
  REG_EXPR(LowerExpression);
  REG_EXPR(TrimExpression);
  REG_EXPR(LTrimExpression);
  REG_EXPR(RTrimExpression);
  REG_EXPR(SpaceExpression);
  REG_EXPR(RepeatExpression);
  REG_EXPR(AsciiExpression);
  REG_EXPR(LPadExpression);
  REG_EXPR(RPadExpression);
  REG_EXPR(LocateExpression);
  REG_EXPR(InStrExpression);
  REG_EXPR(FindInSetExpression);
  REG_EXPR(ParseURLExpression);
  REG_EXPR(RegExpExpression);
  REG_EXPR(RegExpReplaceExpression);
  REG_EXPR(RegExpExtractExpression);
  REG_EXPR(GetJsonObjectExpression);

  // arithmetic
  REG_EXPR(AddExpression);
  REG_EXPR(SubtractExpression);
  REG_EXPR(MultiplyExpression);
  REG_EXPR(DivideExpression);
  REG_EXPR(ModulusExpression);
  REG_EXPR(HashExpression);
  REG_EXPR(BitAndExpression);
  REG_EXPR(BitOrExpression);
  REG_EXPR(BitNotExpression);
  REG_EXPR(BitXorExpression);

  // math
  REG_EXPR(RoundExpression);
  REG_EXPR(FloorExpression);
  REG_EXPR(CeilExpression);
  REG_EXPR(RandExpression);
  REG_EXPR(ExpExpression);
  REG_EXPR(LnExpression);
  REG_EXPR(Log10Expression);
  REG_EXPR(Log2Expression);
  REG_EXPR(LogExpression);
  REG_EXPR(PowExpression);
  REG_EXPR(SqrtExpression);
  REG_EXPR(BinExpression);
  REG_EXPR(HexExpression);
  REG_EXPR(UnHexExpression);
  REG_EXPR(ConvExpression);
  REG_EXPR(AbsExpression);
  REG_EXPR(PModExpression);
  REG_EXPR(SinExpression);
  REG_EXPR(ASinExpression);
  REG_EXPR(CosExpression);
  REG_EXPR(ACosExpression);
  REG_EXPR(PositiveExpression);
  REG_EXPR(NegativeExpression);
  REG_EXPR(DegreesExpression);
  REG_EXPR(RadiansExpression);
  REG_EXPR(SignExpression);
  REG_EXPR(EExpression);
  REG_EXPR(PIExpression);

  // date
  REG_EXPR(FromUnixTimeExpression);
  REG_EXPR(UnixTimestampExpression);
  REG_EXPR(ToDateExpression);
  REG_EXPR(YearExpression);
  REG_EXPR(MonthExpression);
  REG_EXPR(DayExpression);
  REG_EXPR(HourExpression);
  REG_EXPR(MinuteExpression);
  REG_EXPR(SecondExpression);
  REG_EXPR(WeekOfYearExpression);
  REG_EXPR(DateDiffExpression);
  REG_EXPR(DateAddExpression);
  REG_EXPR(DateSubExpression);

  // cast
  REG_EXPR(UDFToStringExpression);
  REG_EXPR(UDFToLongExpression);
  REG_EXPR(UDFToIntegerExpression);
  REG_EXPR(UDFToShortExpression);
  REG_EXPR(UDFToByteExpression);
  REG_EXPR(UDFToFloatExpression);
  REG_EXPR(UDFToDoubleExpression);
  REG_EXPR(UDFToBooleanExpression);
  REG_EXPR(UDFToBinaryExpression);
  REG_EXPR(UDFToDecimalExpression);
}

idgs::ResultCode ExpressionFactory::build(Expression** expression, const idgs::pb::Expr& entryExp,
    const PbMessagePtr& key, const PbMessagePtr& value) {
  assert(* expression == NULL);
  if (!entryExp.has_name()) {
    LOG(ERROR) << "Expr: no type.";
    return idgs::RC_UNKNOWN_OPERATION;
  }

  const std::string& exprName = entryExp.name();
  auto it = exprs.find(exprName);
  if (it == exprs.end()) {
    LOG(ERROR) << "Expression not found: " << exprName;
    return idgs::RC_UNKNOWN_OPERATION;
  }

  *expression = (it->second)->clone();
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

void ExpressionFactory::registerExpression(std::shared_ptr<Expression> expr) {
  if (!expr) {
    LOG(ERROR) << "registered expression is NULL";
    return;
  }

  exprs.insert(std::make_pair(expr->name(), expr));
  auto alias = expr->alias();
  if (alias) {
    auto len = sizeof(alias) / sizeof(std::string);
    for (int i = 0; i < len; ++ i) {
      exprs.insert(std::make_pair(alias[i], expr));
    }
  }
}

} /* namespace expr */
} /* namespace idgs */
