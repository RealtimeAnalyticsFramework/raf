
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#include "expression.h"

namespace idgs {
namespace expr {

/// Parameters: [cond1, value1, cond2, value2, ... , condN, valueN, default]
/// logic:
/// if(cond1) value1
/// else if(cond2) value2
/// ...
/// else default
class WhenExpression: public NAryExpression, public idgs::util::CloneEnabler<WhenExpression, Expression> {
public:
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("WHEN");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias = "IF";
    return &alias;
  }

  virtual bool parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key,
      const idgs::actor::PbMessagePtr& value) override;
};

/// Parameters: [expr, cond_value1, value1, cond_value2, value2, ... , cond_valueN, valueN, default]
/// logic:
/// if(expr == cond_value1) value1
/// else if(expr == cond_value2) value2
/// ...
/// else default
class CaseExpression: public NAryExpression, public idgs::util::CloneEnabler<CaseExpression, Expression> {
public:
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("CASE");
    return name;
  }

  virtual bool parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key,
      const idgs::actor::PbMessagePtr& value) override;
};

/// Parameters: [expr, default]
/// logic:
/// if(expr is null) default
/// else expr
class NvlExpression: public BinaryExpression, public idgs::util::CloneEnabler<NvlExpression, Expression> {
public:
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("NVL");
    return name;
  }
};

/// Parameters: [v1, v2... vn]
/// logic:
/// if(v1 is not null) v1
/// else if (v2 is not null) v2
/// ...
/// else if (vn is not null) vn
/// else NULL
class CoalesceExpression: public NAryExpression, public idgs::util::CloneEnabler<CoalesceExpression, Expression> {
public:
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("COALESCE");
    return name;
  }
};


///
/// variable set expression
///
class VariableSetExpression: public UnaryExpression, public idgs::util::CloneEnabler<VariableSetExpression, Expression> {
public:
  VariableSetExpression();
  virtual ~VariableSetExpression();

public:
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("VSET");
    return name;
  }
  virtual bool parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key,
      const idgs::actor::PbMessagePtr& value) override;
private:
  int index;
};

///
/// variable get expression
///
class VariableGetExpression: public NullaryExpression,
    public idgs::util::CloneEnabler<VariableGetExpression, Expression> {
public:
  VariableGetExpression();
  virtual ~VariableGetExpression();

public:
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual const std::string& name() {
    static std::string name("VGET");
    return name;
  }
  virtual bool parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key,
      const idgs::actor::PbMessagePtr& value) override;

private:
  uint32_t index;
};

} /* namespace expr */
} /* namespace idgs */
