
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "expression.h"

namespace idgs {
namespace expr {

/// Round expression
/// To handle operator round(n[,d]).
class RoundExpression: public NAryExpression, public idgs::util::CloneEnabler<RoundExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual bool parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) override;

  virtual const std::string& name() override {
    static std::string name("ROUND");
    return name;
  }

};

/// Floor expression
/// To handle operator floor(n).
class FloorExpression: public UnaryExpression, public idgs::util::CloneEnabler<FloorExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("FLOOR");
    return name;
  }
};

/// Ceil expression
/// To handle operator ceil(n), ceiling(n).
class CeilExpression: public UnaryExpression, public idgs::util::CloneEnabler<CeilExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("CEIL");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias = "CEILING";
    return &alias;
  }
};

/// Rand expression
/// To handle operator rand(), rand(seed).
class RandExpression: public UnaryExpression, public idgs::util::CloneEnabler<RandExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual bool parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) override;

  virtual const std::string& name() override {
    static std::string name("RAND");
    return name;
  }

};

/// Exp expression
/// To handle operator exp(double n).
class ExpExpression: public UnaryExpression, public idgs::util::CloneEnabler<ExpExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("EXP");
    return name;
  }

};

/// Ln expression
/// To handle operator ln(double n).
class LnExpression: public UnaryExpression, public idgs::util::CloneEnabler<LnExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("LN");
    return name;
  }

};

/// Log10 expression
/// To handle operator log10(double n).
class Log10Expression: public UnaryExpression, public idgs::util::CloneEnabler<Log10Expression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("LOG10");
    return name;
  }

};

/// Log2 expression
/// To handle operator log2(double n).
class Log2Expression: public UnaryExpression, public idgs::util::CloneEnabler<Log2Expression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("LOG2");
    return name;
  }

};

/// Log expression
/// To handle operator log(double base, double n).
class LogExpression: public BinaryExpression, public idgs::util::CloneEnabler<LogExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("LOG");
    return name;
  }

};

/// Pow expression
/// To handle operator pow(double base, double power), power(double base, double power).
class PowExpression: public BinaryExpression, public idgs::util::CloneEnabler<PowExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("POW");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias = "POWER";
    return &alias;
  }

};

/// Sqrt expression
/// To handle operator log2(double base, double n).
class SqrtExpression: public UnaryExpression, public idgs::util::CloneEnabler<SqrtExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("SQRT");
    return name;
  }

};

/// Bin expression
/// To handle operator bin(bigint n).
class BinExpression: public UnaryExpression, public idgs::util::CloneEnabler<BinExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("BIN");
    return name;
  }

};

/// Hex expression
/// To handle operator hex(bigint n), hex(string n).
class HexExpression: public UnaryExpression, public idgs::util::CloneEnabler<HexExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("HEX");
    return name;
  }

private:
  static char toHexString(int digit);

};

/// UnHex expression
/// To handle operator unhex(string s).
class UnHexExpression: public UnaryExpression, public idgs::util::CloneEnabler<UnHexExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("UNHEX");
    return name;
  }

private:
  static int32_t toHexString(int digit);

};

/// Conv expression
/// To handle operator conv(num, int from_base, int to_base). from_base and to_base must in [2, 36]
class ConvExpression: public TernaryExpression, public idgs::util::CloneEnabler<ConvExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("CONV");
    return name;
  }

private:
  static std::string conv(const std::string& number, const uint32_t& fromBase, const uint32_t& toBase);

};

/// Abs expression
/// To handle operator abs(double n)
class AbsExpression: public UnaryExpression, public idgs::util::CloneEnabler<AbsExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("ABS");
    return name;
  }

};

/// PMod expression
/// To handle operator pmod(int a, int b), pmod(double a, double b)
class PModExpression: public BinaryExpression, public idgs::util::CloneEnabler<PModExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("PMOD");
    return name;
  }

};

/// Sin expression
/// To handle operator sin(double a)
class SinExpression: public UnaryExpression, public idgs::util::CloneEnabler<SinExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("SIN");
    return name;
  }

};

/// ASin expression
/// To handle operator asin(double a)
class ASinExpression: public UnaryExpression, public idgs::util::CloneEnabler<ASinExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("ASIN");
    return name;
  }

};

/// Cos expression
/// To handle operator cos(double a)
class CosExpression: public UnaryExpression, public idgs::util::CloneEnabler<CosExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("COS");
    return name;
  }

};

/// ACos expression
/// To handle operator acos(double a)
class ACosExpression: public UnaryExpression, public idgs::util::CloneEnabler<ACosExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("ACOS");
    return name;
  }

};

/// Positive expression
/// To handle operator positive(int a), positive(double a)
class PositiveExpression: public UnaryExpression, public idgs::util::CloneEnabler<PositiveExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("POSITIVE");
    return name;
  }

};

/// Negative expression
/// To handle operator negative(int a), negative(double a)
class NegativeExpression: public UnaryExpression, public idgs::util::CloneEnabler<NegativeExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("NEGATIVE");
    return name;
  }

};

/// Degrees expression
/// To handle operator degrees(double d)
class DegreesExpression: public UnaryExpression, public idgs::util::CloneEnabler<DegreesExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("DEGREES");
    return name;
  }

};

/// Radians expression
/// To handle operator radians(double d)
class RadiansExpression: public UnaryExpression, public idgs::util::CloneEnabler<RadiansExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("RADIANS");
    return name;
  }

};

/// Sign expression
/// To handle operator sign(double d)
class SignExpression: public UnaryExpression, public idgs::util::CloneEnabler<SignExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("SIGN");
    return name;
  }

};

/// E expression
/// To handle operator e()
class EExpression: public NullaryExpression, public idgs::util::CloneEnabler<EExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("E");
    return name;
  }

};

/// PI expression
/// To handle operator pi()
class PIExpression: public NullaryExpression, public idgs::util::CloneEnabler<PIExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("PI");
    return name;
  }

};

} // namespace expr
} // namespace idgs 
