
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/util/resource_manager.h"
#include "idgs/expr/expression.h"

namespace idgs {
namespace expr {

/// The factory to build expression
class ExpressionFactory {
public:
  static void init();

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param expression Parsed expression tree.
  /// @param entryExp   Expression tree, it is protobuf message from client or other ways.
  /// @param key        Key of data, the data corresponding to field extractor of expression.
  /// @param value      Value of data, the data corresponding to field extractor of expression.
  /// @return Status code of result.
  static idgs::ResultCode build(Expression** expression, const idgs::pb::Expr& entryExp,
      const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value);

private:
  static idgs::util::resource_manager<Expression*> exprs;
};

} /* namespace expr */
} /* namespace idgs */
