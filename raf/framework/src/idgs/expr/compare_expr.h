
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "expression.h"


namespace idgs {
namespace expr {

/// Equal expression
/// To handle operator '='.
class EQExpression: public BinaryExpression, public idgs::util::CloneEnabler<EQExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("EQ");
    return name;
  }
};

/// Not equal expression
/// To handle operator '!='.
class NEExpression: public BinaryExpression, public idgs::util::CloneEnabler<NEExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("NE");
    return name;
  }
};

/// Less expression
/// To handle operator '<'.
class LTExpression: public BinaryExpression, public idgs::util::CloneEnabler<LTExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("LT");
    return name;
  }
};

/// Less and equal expression
/// To handle operator '<='.
class LEExpression: public BinaryExpression, public idgs::util::CloneEnabler<LEExpression, Expression> {
public:
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("LE");
    return name;
  }
};

/// Greater expression
/// To handle operator '>'.
class GTExpression: public BinaryExpression, public idgs::util::CloneEnabler<GTExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("GT");
    return name;
  }
};

/// Greater and equal expression
/// To handle operator '>='.
class GEExpression: public BinaryExpression, public idgs::util::CloneEnabler<GEExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("GE");
    return name;
  }
};

/// Like expression
/// To handle operator 'like'.
class LikeExpression: public BinaryExpression, public idgs::util::CloneEnabler<LikeExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("LIKE");
    return name;
  }
};

class BetweenExpression: public NAryExpression, public idgs::util::CloneEnabler<BetweenExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual bool parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key,
        const idgs::actor::PbMessagePtr& value) override;

  virtual const std::string& name() {
    static std::string name("BETWEEN");
    return name;
  }
};

} // namespace expr
} // namespace idgs 
