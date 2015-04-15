
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "const_expr.h"
#include "idgs/util/utillity.h"


namespace idgs {
namespace expr {

bool ConstExpression::parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key,
    const idgs::actor::PbMessagePtr& value) {
  if (entryExp.expression_size()) {
    LOG(ERROR)<< "Invalid subexpression: " << entryExp.name();
    return false;
  }

  ResultCode code = RC_SUCCESS;
  if (entryExp.has_const_type()) {
    switch (entryExp.const_type()) {
      case idgs::pb::DOUBLE: {
        double var = 0;
        code = sys::convert<double>(entryExp.value(), var);
        constValue = var;
        break;
      }
      case idgs::pb::FLOAT: {
        float var = 0;
        code = sys::convert<float>(entryExp.value(), var);
        constValue = var;
        break;
      }
      case idgs::pb::INT64: {
        int64_t var = 0;
        code = sys::convert<int64_t>(entryExp.value(), var);
        constValue = var;
        break;
      }
      case idgs::pb::UINT64: {
        uint64_t var = 0;
        code = sys::convert<uint64_t>(entryExp.value(), var);
        constValue = var;
        break;
      }
      case idgs::pb::INT32: {
        int32_t var = 0;
        code = sys::convert<int32_t>(entryExp.value(), var);
        constValue = var;
        break;
      }
      case idgs::pb::UINT32: {
        uint32_t var = 0;
        code = sys::convert<uint32_t>(entryExp.value(), var);
        constValue = var;
        break;
      }
      case idgs::pb::BOOL: {
        bool var = false;
        if (str::toUpper(entryExp.value()) == "TRUE") {
          var = true;
        } else if (str::toUpper(entryExp.value()) == "FALSE") {
          var = false;
        } else {
          int32_t v = 0;
          code = sys::convert<int32_t>(entryExp.value(), v);
          if (code == RC_SUCCESS) {
            var = (v != 0);
          }
        }

        constValue = var;
        break;
      }
      case idgs::pb::STRING: {
        constValue = entryExp.value();
        break;
      }
      case idgs::pb::BYTES: {
        constValue = entryExp.value();
        break;
      }
      case idgs::pb::ENUM: {
        int32_t var = 0;
        code = sys::convert<int32_t>(entryExp.value(), var);
        constValue = var;
        break;
      }
      default: {
        LOG(ERROR) << "This type cannot be convert to a const.";
        return false;
      }
    }
  } else {
    constValue = entryExp.value();
  }

  if (code != RC_SUCCESS) {
    LOG(ERROR) << "parse const expression error, caused by " << getErrorDescription(code);
    return false;
  }

  return true;
}

protobuf::PbVariant ConstExpression::evaluate(ExpressionContext* ctx) const {
  return constValue;
}

} // namespace expr
} // namespace idgs 
