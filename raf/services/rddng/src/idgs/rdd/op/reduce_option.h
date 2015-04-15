/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include "idgs/expr/expression.h"
#include "protobuf/message_helper.h"


namespace idgs {
namespace rdd {
namespace op {

struct ReduceOption {
  protobuf::PbVariant value;
  protobuf::PbVariant aggrValue;
  std::set<protobuf::PbVariant> distinctSet;
  size_t reduceCount;

  bool isDistinctValue(protobuf::PbVariant& value) {
    auto it = distinctSet.find(value);
    if (it == distinctSet.end()) {
      distinctSet.insert(value);
      return false;
    } else {
      return true;
    }
  }

  void reset() {
    value.is_null = true;
    aggrValue.is_null = true;
    reduceCount = 0;
    distinctSet.clear();
  }
};

typedef std::vector<ReduceOption> ReduceOptions;

class ReduceOperation {
public:
  ReduceOperation() : field(NULL), distinct(false) {
  }

  virtual ~ReduceOperation() {
  }

public:
  virtual const std::string& getName() const = 0;
  virtual void reduce(idgs::expr::ExpressionContext* ctx, ReduceOption& option) = 0;
  virtual void aggregate(idgs::expr::ExpressionContext* ctx, ReduceOption& option) = 0;

public:
  const google::protobuf::FieldDescriptor* field;
  bool distinct;

protected:
  protobuf::MessageHelper helper;
};

class CountReduceOperation : public ReduceOperation {
public:
  virtual const std::string& getName() const override {
    static std::string name = "COUNT";
    return name;
  }

  virtual void reduce(idgs::expr::ExpressionContext* ctx, ReduceOption& option) override;
  virtual void aggregate(idgs::expr::ExpressionContext* ctx, ReduceOption& option) override;

};

class SumReduceOperation : public ReduceOperation {
public:
  virtual const std::string& getName() const override {
    static std::string name = "SUM";
    return name;
  }

  virtual void reduce(idgs::expr::ExpressionContext* ctx, ReduceOption& option) override;
  virtual void aggregate(idgs::expr::ExpressionContext* ctx, ReduceOption& option) override;
};

class MaxReduceOperation : public ReduceOperation {
public:
  virtual const std::string& getName() const override {
    static std::string name = "MAX";
    return name;
  }

  virtual void reduce(idgs::expr::ExpressionContext* ctx, ReduceOption& option) override;
  virtual void aggregate(idgs::expr::ExpressionContext* ctx, ReduceOption& option) override;

};

class MinReduceOperation : public ReduceOperation {
public:
  virtual const std::string& getName() const override {
    static std::string name = "MIN";
    return name;
  }

  virtual void reduce(idgs::expr::ExpressionContext* ctx, ReduceOption& option) override;
  virtual void aggregate(idgs::expr::ExpressionContext* ctx, ReduceOption& option) override;

};

class AvgReduceOperation : public ReduceOperation {
public:
  virtual const std::string& getName() const override {
    static std::string name = "AVG";
    return name;
  }

  virtual void reduce(idgs::expr::ExpressionContext* ctx, ReduceOption& option) override;
  virtual void aggregate(idgs::expr::ExpressionContext* ctx, ReduceOption& option) override;

};

class ExprReduceOperation : public ReduceOperation {
public:
  virtual const std::string& getName() const override {
    static std::string name = "EXPR";
    return name;
  }

  virtual void reduce(idgs::expr::ExpressionContext* ctx, ReduceOption& option) override;
  virtual void aggregate(idgs::expr::ExpressionContext* ctx, ReduceOption& option) override;

};

class ReduceOperationFactory {
public:
  static ReduceOperation* getOption();
  static ReduceOperation* getOption(const std::string& name);

};

} /* namespace op */
} /* namespace store */
} /* namespace idgs */
