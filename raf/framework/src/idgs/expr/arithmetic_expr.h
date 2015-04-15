
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "expression.h"

namespace idgs {
namespace expr {

/// Add expression
/// To handle operator '+'.
class AddExpression: public NAryExpression, public idgs::util::CloneEnabler<AddExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("ADD");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias("+");
    return &alias;
  }
};

/// Subtract expression
/// To handle operator '-'.
class SubtractExpression: public NAryExpression, public idgs::util::CloneEnabler<SubtractExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("SUBTRACT");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias("-");
    return &alias;
  }
};

/// Multiply expression
/// To handle operator '*'.
class MultiplyExpression: public NAryExpression, public idgs::util::CloneEnabler<MultiplyExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("MULTIPLY");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias("*");
    return &alias;
  }
};

/// Divide expression
/// To handle operator '/'.
class DivideExpression: public NAryExpression, public idgs::util::CloneEnabler<DivideExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("DIVIDE");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias("/");
    return &alias;
  }
};

/// Modulus expression
/// To handle operator '%'.
class ModulusExpression: public BinaryExpression, public idgs::util::CloneEnabler<ModulusExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("MODULUS");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias("%");
    return &alias;
  }
};


/// hashcode expression
class HashExpression: public NAryExpression, public idgs::util::CloneEnabler<HashExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("HASH");
    return name;
  }
};

/// bit and expression
class BitAndExpression: public NAryExpression, public idgs::util::CloneEnabler<BitAndExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("BIT_AND");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias("&");
    return &alias;
  }
};

/// bit or expression
class BitOrExpression: public NAryExpression, public idgs::util::CloneEnabler<BitOrExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("BIT_OR");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias("|");
    return &alias;
  }
};

/// bit not expression
class BitNotExpression: public UnaryExpression, public idgs::util::CloneEnabler<BitNotExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("BIT_NOT");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias("~");
    return &alias;
  }
};

/// bit xor expression
class BitXorExpression: public NAryExpression, public idgs::util::CloneEnabler<BitXorExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("BIT_XOR");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias("^");
    return &alias;
  }
};

} // namespace expr
} // namespace idgs 
