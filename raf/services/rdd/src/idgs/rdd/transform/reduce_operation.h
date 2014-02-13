
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <sstream>
#include "idgs/expr/expression.h"
#include <google/protobuf/message.h>
#include "idgs/rdd/pb/rdd_transform.pb.h"
#include "tbb/tbb.h"

#include "protobuf/pbvariant.h"

namespace idgs {
namespace rdd {
namespace transform {
namespace reduce {

struct ReduceOperation {
  idgs::expr::Expression* expr;
  std::set<std::string> set;
  tbb::spin_rw_mutex mutex;
  const google::protobuf::FieldDescriptor* descriptor;
  bool key; // field belong to key or value
  bool is_distinct;
  idgs::expr::ExpressionContext ctx;
  ReduceOperation() :
      expr(NULL), descriptor(NULL), key(true), is_distinct(false) {
  }

  std::string toString() {
    std::stringstream s;
    s << "type: " << pb::ReduceType_Name(getType()) << std::endl;
    if (descriptor) {
      s << "descriptor: " << descriptor->DebugString();
    }
    return s.str();
  }

  /// reduce operation
  virtual void reduce(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) = 0;

  virtual pb::ReduceType getType() = 0;

  virtual protobuf::PbVariant getResult() = 0;

  virtual void reset() = 0;

  virtual ~ReduceOperation() {

  }
};

struct CountReduceOperation: public ReduceOperation {
  size_t count;
  CountReduceOperation() :
      count(0) {

  }
  void reduce(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
    ctx.setKeyValue(&key, &value);
    protobuf::PbVariant expr_value = expr->evaluate(&ctx);
    if (is_distinct) {
      auto element = expr_value.toString();
      bool exist = false;
      {
        tbb::spin_rw_mutex::scoped_lock lock(mutex, false); /// read lock
        exist = set.find(element) != set.end();
      }
      if (!exist) {
        {
          tbb::spin_rw_mutex::scoped_lock lock(mutex, true); /// write lock
          exist = set.find(element) != set.end();
          if (!exist) { /// double-checked
            set.insert(element);
            ++count;
          }
        }
      }
    } else {
      ++count;
    }
  }

  pb::ReduceType getType() {
    return pb::ReduceType::COUNT;
  }

  protobuf::PbVariant getResult() {
    return count;
  }

  void reset() {
    count = 0;
  }
};

struct SumReduceOperation: public ReduceOperation {
  double sum;
  SumReduceOperation() :
      sum(0) {

  }
  void reduce(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
    ctx.setKeyValue(&key, &value);
    protobuf::PbVariant expr_value = expr->evaluate(&ctx);
    if (is_distinct) {
      auto element = expr_value.toString();
      bool exist = false;
      {
        tbb::spin_rw_mutex::scoped_lock lock(mutex, false); /// read lock
        exist = set.find(element) != set.end();
      }
      if (!exist) {
        {
          tbb::spin_rw_mutex::scoped_lock lock(mutex, true); /// write lock
          exist = set.find(element) != set.end();
          if (!exist) { /// double-checked
            set.insert(element);
            sum += (double) expr_value;
          }
        }
      }
    } else {
      sum += (double) expr_value;
    }
  }

  pb::ReduceType getType() {
    return pb::ReduceType::SUM;
  }

  protobuf::PbVariant getResult() {
    return sum;
  }

  void reset() {
    sum = 0;
  }
};

struct MaxReduceOperation: public ReduceOperation {
  protobuf::PbVariant max;
  MaxReduceOperation() :
      max("") {

  }
  void reduce(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
    ctx.setKeyValue(&key, &value);
    protobuf::PbVariant expr_value = expr->evaluate(&ctx);
    if (max == "") {
      max = expr_value;
      return;
    }
    if (expr_value > max) {
      max = expr_value;
    }
  }

  pb::ReduceType getType() {
    return pb::ReduceType::MAX;
  }

  protobuf::PbVariant getResult() {
    return max;
  }

  void reset() {
    max = "";
  }
};

struct MinReduceOperation: public ReduceOperation {
  protobuf::PbVariant min;
  MinReduceOperation() :
      min(0) {

  }
  void reduce(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
    ctx.setKeyValue(&key, &value);
    protobuf::PbVariant expr_value = expr->evaluate(&ctx);
    if (min == "") {
      min = expr_value;
      return;
    }
    if (expr_value < min) {
      min = expr_value;
    }
  }

  pb::ReduceType getType() {
    return pb::ReduceType::MAX;
  }

  protobuf::PbVariant getResult() {
    return min;
  }

  void reset() {
    min = "";
  }
};

struct AvgReduceOperation: public ReduceOperation {
  size_t count;
  double sum;
  AvgReduceOperation() :
      count(0), sum(0) {

  }
  void reduce(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
    ctx.setKeyValue(&key, &value);
    protobuf::PbVariant expr_value = expr->evaluate(&ctx);
    if (is_distinct) {
      auto element = expr_value.toString();
      bool exist = false;
      {
        tbb::spin_rw_mutex::scoped_lock lock(mutex, false); /// read lock
        exist = set.find(element) != set.end();
      }
      if (!exist) {
        {
          tbb::spin_rw_mutex::scoped_lock lock(mutex, true); /// write lock
          exist = set.find(element) != set.end();
          if (!exist) { /// double-checked
            set.insert(element);
            ++count;
            sum += (double) expr_value;
          }
        }
      }
    } else {
      ++count;
      sum += (double) expr_value;
    }
  }

  pb::ReduceType getType() {
    return pb::ReduceType::AVG;
  }

  protobuf::PbVariant getResult() {
    return count == 0 ? 0 : sum / count;
  }

  void reset() {
    count = 0;
    sum = 0;
  }
};

struct ExprReduceOperation: public ReduceOperation {
  protobuf::PbVariant value;
  ExprReduceOperation() :
      value("") {

  }
  void reduce(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
    ctx.setKeyValue(&key, &value);
    protobuf::PbVariant expr_value = expr->evaluate(&ctx);
    this->value = expr_value;
  }

  pb::ReduceType getType() {
    return pb::ReduceType::EXPR;
  }

  protobuf::PbVariant getResult() {
    return value;
  }

  void reset() {

  }
};

struct ReduceOperationFactory {

  static ReduceOperation* create() {
    return create(pb::ReduceType::EXPR);
  }

  static ReduceOperation* create(pb::ReduceType type) {
    switch (type) {
    case pb::ReduceType::EXPR:
      return new ExprReduceOperation;

    case pb::ReduceType::COUNT:
      return new CountReduceOperation;

    case pb::ReduceType::SUM:
      return new SumReduceOperation;

    case pb::ReduceType::MAX:
      return new MaxReduceOperation;

    case pb::ReduceType::MIN:
      return new MinReduceOperation;

    case pb::ReduceType::AVG:
      return new AvgReduceOperation;
    default:
      return NULL; /// not support
    }
  }
};

struct ParallelReduceOperation {

  ReduceOperation* operation;

  ParallelReduceOperation() :
      operation(NULL) {

  }

  virtual ~ParallelReduceOperation() {

  }

  ParallelReduceOperation(ReduceOperation* operation) {
    this->operation = operation;
  }

  ParallelReduceOperation(const ParallelReduceOperation& copy) {
    operation = ReduceOperationFactory::create(copy.operation->getType()); /// copy
    *operation = *copy.operation;
  }

  void reduce(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
    operation->reduce(key, value);
  }

  protobuf::PbVariant getResult() const {
    return operation->getResult();
  }

  pb::ReduceType getType() const {
    return operation->getType();
  }

  void merge(const ParallelReduceOperation& x, const ParallelReduceOperation& y) {
    switch (x.getType()) {
      case pb::ReduceType::COUNT: {
        CountReduceOperation* op = new CountReduceOperation(*(dynamic_cast<CountReduceOperation*>(x.operation))); /// copy
        op->count = dynamic_cast<CountReduceOperation*>(x.operation)->count
            + dynamic_cast<CountReduceOperation*>(y.operation)->count;
        this->operation = op;
        break;
      }
      case pb::ReduceType::SUM: {
        SumReduceOperation* op = new SumReduceOperation(*(dynamic_cast<SumReduceOperation*>(x.operation)));
        op->sum = dynamic_cast<SumReduceOperation*>(x.operation)->sum
            + dynamic_cast<SumReduceOperation*>(y.operation)->sum;
        this->operation = op;
        break;
      }
      case pb::ReduceType::MAX: {
        MaxReduceOperation* op = new MaxReduceOperation(*(dynamic_cast<MaxReduceOperation*>(x.operation)));
        auto a = dynamic_cast<MaxReduceOperation*>(x.operation)->max;
        auto b = dynamic_cast<MaxReduceOperation*>(y.operation)->max;
        op->max = a > b ? a : b;
        this->operation = op;
        break;
      }
      case pb::ReduceType::MIN: {
        MinReduceOperation* op = new MinReduceOperation(*(dynamic_cast<MinReduceOperation*>(x.operation)));
        auto a = dynamic_cast<MinReduceOperation*>(x.operation)->min;
        auto b = dynamic_cast<MinReduceOperation*>(y.operation)->min;
        op->min = a < b ? a : b;
        this->operation = op;
        break;
      }
      case pb::ReduceType::AVG: {
        AvgReduceOperation* op = new AvgReduceOperation(*(dynamic_cast<AvgReduceOperation*>(x.operation)));
        op->count = dynamic_cast<AvgReduceOperation*>(x.operation)->count
            + dynamic_cast<AvgReduceOperation*>(y.operation)->count;
        op->sum = dynamic_cast<AvgReduceOperation*>(x.operation)->sum
            + dynamic_cast<AvgReduceOperation*>(y.operation)->sum;
        this->operation = op;
        break;
      }
      case pb::ReduceType::EXPR: {
        ExprReduceOperation* op = new ExprReduceOperation(*(dynamic_cast<ExprReduceOperation*>(x.operation)));
        op->value = dynamic_cast<ExprReduceOperation*>(x.operation)->value;
        this->operation = op;
        break;
      }
    }
  }
};

} // namespace reduce
} // namespace transform
} // namespace rdd
} // namespace idgs
