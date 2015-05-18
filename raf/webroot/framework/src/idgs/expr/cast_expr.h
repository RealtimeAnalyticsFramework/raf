
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "expression.h"

namespace idgs {
namespace expr {

/// UDFToString expression
/// To handle operator cast(expr as string).
class UDFToStringExpression: public UnaryExpression, public idgs::util::CloneEnabler<UDFToStringExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("UDFTOSTRING");
    return name;
  }
};

/// UDFToLong expression
/// To handle operator cast(expr as bigint).
class UDFToLongExpression: public UnaryExpression, public idgs::util::CloneEnabler<UDFToLongExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("UDFTOLONG");
    return name;
  }
};

/// UDFToInteger expression
/// To handle operator cast(expr as int).
class UDFToIntegerExpression: public UnaryExpression, public idgs::util::CloneEnabler<UDFToIntegerExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("UDFTOINTEGER");
    return name;
  }
};

/// UDFToShort expression
/// To handle operator cast(expr as smallint).
class UDFToShortExpression: public UnaryExpression, public idgs::util::CloneEnabler<UDFToShortExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("UDFTOSHORT");
    return name;
  }
};

/// UDFToByte expression
/// To handle operator cast(expr as tinyint).
class UDFToByteExpression: public UnaryExpression, public idgs::util::CloneEnabler<UDFToByteExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("UDFTOBYTE");
    return name;
  }
};

/// UDFToFloat expression
/// To handle operator cast(expr as float).
class UDFToFloatExpression: public UnaryExpression, public idgs::util::CloneEnabler<UDFToFloatExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("UDFTOFLOAT");
    return name;
  }
};

/// UDFToDouble expression
/// To handle operator cast(expr as double).
class UDFToDoubleExpression: public UnaryExpression, public idgs::util::CloneEnabler<UDFToDoubleExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("UDFTODOUBLE");
    return name;
  }
};

/// UDFToBoolean expression
/// To handle operator cast(expr as boolean).
class UDFToBooleanExpression: public UnaryExpression, public idgs::util::CloneEnabler<UDFToBooleanExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("UDFTOBOOLEAN");
    return name;
  }
};

/// UDFToBinary expression
/// To handle operator cast(expr as binary).
class UDFToBinaryExpression: public NAryExpression, public idgs::util::CloneEnabler<UDFToBinaryExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual bool parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) override;

  virtual const std::string& name() {
    static std::string name("UDFTOBINARY");
    return name;
  }
};

/// UDFToDecimal expression
/// To handle operator cast(expr as decimal).
class UDFToDecimalExpression: public UnaryExpression, public idgs::util::CloneEnabler<UDFToDecimalExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("UDFTODECIMAL");
    return name;
  }
};

} // namespace expr
} // namespace idgs 
