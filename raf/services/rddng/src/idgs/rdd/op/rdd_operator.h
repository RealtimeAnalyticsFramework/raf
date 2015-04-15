
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/expr/expression_context.h"
#include "idgs/rdd/pb/rdd_svc.pb.h"
#include "idgs/util/cloneable.h"
#include "idgs/util/resource_manager.h"

namespace idgs {
namespace rdd {

class RddLocal;

namespace op {

class RddOperator : public virtual idgs::util::Cloneable<RddOperator> {
public:

  virtual const std::string& getName() const = 0;

  /// evaluate the operator, and the state of ctx may be changed by the operator.
  /// @param ctx the context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual bool evaluate(idgs::expr::ExpressionContext* ctx) = 0;

  virtual bool parse(const idgs::rdd::pb::InRddInfo& inRddInfo, const idgs::rdd::pb::OutRddInfo& outRddInfo,
      RddLocal* inRddLocal, RddLocal* outRddLocal) = 0;

  /// @see #evaluate
  bool operator ()(idgs::expr::ExpressionContext* ctx) {
    return evaluate(ctx);
  }

public:
  std::vector<RddOperator*> paramOperators;
};

} // namespace op

typedef std::shared_ptr<idgs::rdd::op::RddOperator> RddOperatorPtr;
typedef idgs::util::resource_manager<RddOperatorPtr> RddOperatorMgr;

} // namespace rdd
} // namespace idgs
