
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "expression_context.h"
#include "idgs/pb/expr.pb.h"
#include "idgs/util/cloneable.h"

namespace idgs {
namespace expr {

/// The interface of express,
/// All Expression should be stateless, and all immutable data should passed in context.
class Expression: public virtual idgs::util::Cloneable<Expression> {
public:
  Expression();
  virtual ~Expression();

public:
  /// evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx the context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const = 0;

  /// same as evaluate.
  inline protobuf::PbVariant operator ()(ExpressionContext* ctx) const {
    return evaluate(ctx);
  }

  virtual const std::string& name() = 0;

  // should be (const idgs::pb::Expr& entryExp, const ExpressionContext* ctx);
  virtual bool parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key,
      const idgs::actor::PbMessagePtr& value) {
    return parseSubExpression(entryExp, key, value);
  }

  virtual bool parseSubExpression(const idgs::pb::Expr& entryExp,
      const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) = 0;
};

/// unary expression which has no child expression.
/// en.wikipedia.org/wiki/Arity
class NullaryExpression: public Expression {
public:

  virtual bool parseSubExpression(const idgs::pb::Expr& entryExp,
      const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value);
};

/// unary expression which has only 1 child expression.
/// en.wikipedia.org/wiki/Arity
class UnaryExpression: public Expression {
public:
  UnaryExpression() :
      child(NULL) {
  }

  virtual ~UnaryExpression() {
    if (child) {
      delete child;
      child = NULL;
    }
  }

  virtual bool parseSubExpression(const idgs::pb::Expr& entryExp,
      const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value);

protected:
  Expression* child;
};

/// binary expression which has only 2 child expression.
/// en.wikipedia.org/wiki/Arity
class BinaryExpression: public Expression {
public:
  BinaryExpression() :
      leftChild(NULL), rightChild(NULL) {
  }

  virtual ~BinaryExpression() {
    if (leftChild) {
      delete leftChild;
      leftChild = NULL;
    }
    if (rightChild) {
      delete rightChild;
      rightChild = NULL;
    }
  }

  virtual bool parseSubExpression(const idgs::pb::Expr& entryExp,
      const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value);

protected:
  Expression* leftChild;
  Expression* rightChild;
};

/// Ternary expression which has only 3 child expression.
/// en.wikipedia.org/wiki/Arity
class TernaryExpression: public Expression {
public:
  TernaryExpression() :
      leftChild(NULL), middleChild(NULL), rightChild(NULL) {
  }

  virtual ~TernaryExpression() {
    if (leftChild) {
      delete leftChild;
      leftChild = NULL;
    }

    if (middleChild) {
      delete middleChild;
      middleChild = NULL;
    }

    if (rightChild) {
      delete rightChild;
      rightChild = NULL;
    }
  }

  virtual bool parseSubExpression(const idgs::pb::Expr& entryExp,
      const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value);

protected:
  Expression* leftChild;
  Expression* middleChild;
  Expression* rightChild;
};

/// multiple expression which has only more than 2 child expression.
/// en.wikipedia.org/wiki/Arity
class NAryExpression: public Expression {
public:
  NAryExpression() {
  }

  virtual ~NAryExpression() {
    for (auto it = children.begin(); it != children.end(); ++it) {
      delete *it;
    }

    children.clear();
  }

  virtual bool parseSubExpression(const idgs::pb::Expr& entryExp,
      const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value);

protected:
  std::vector<Expression*> children;
};

} // namespace expr
} // namespace idgs 
