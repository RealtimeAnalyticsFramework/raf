
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "expression.h"

namespace idgs {
namespace expr {

/// And expression
/// To handle operator '&&'.
class AndExpression: public NAryExpression, public idgs::util::CloneEnabler<AndExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("AND");
    return name;
  }
};

/// Or expression
/// To handle operator '||'.
class OrExpression: public NAryExpression, public idgs::util::CloneEnabler<OrExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("OR");
    return name;
  }
};

/// Not expression
/// To handle operator '!'.
class NotExpression: public UnaryExpression, public idgs::util::CloneEnabler<NotExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("NOT");
    return name;
  }
};

} // namespace expr
} // namespace idgs 
