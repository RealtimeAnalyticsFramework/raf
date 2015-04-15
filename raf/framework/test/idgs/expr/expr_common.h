/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include "idgs/expr/expression_factory.h"

namespace idgs {
namespace expr {
  ResultCode exprEvaluate(protobuf::PbVariant& result, const idgs::pb::Expr* entryExpr, ExpressionContext* ctx = NULL) {
    Expression* expr = NULL;
    idgs::actor::PbMessagePtr key, value;
    if (ctx) {
      if (ctx->getKey() && ctx->getValue()) {
        key = (* ctx->getKey());
        value = (* ctx->getValue());
      }
    }

    auto code = ExpressionFactory::build(&expr, * entryExpr, key, value);
    if (code != RC_SUCCESS) {
      delete entryExpr;
      return code;
    }

    try {
      result = expr->evaluate(ctx);
    } catch (std::exception& ex) {
      LOG(ERROR) << ex.what();
      code = RC_ERROR;
    }

    delete expr;
    delete entryExpr;
    return code;
  }
}
}
