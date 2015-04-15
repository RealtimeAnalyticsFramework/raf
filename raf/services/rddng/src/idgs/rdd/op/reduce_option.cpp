/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "reduce_option.h"

namespace idgs {
namespace rdd {
namespace op {

void CountReduceOperation::reduce(idgs::expr::ExpressionContext* ctx, ReduceOption& option) {
  auto& outvalue = (* ctx->getOutputValue());

  protobuf::PbVariant value;
  helper.getMessageValue(outvalue.get(), field, value);

  if (!distinct || !option.isDistinctValue(value)) {
    if (option.value.is_null) {
      option.value = (size_t) 1;
    } else {
      option.value = (size_t) option.value + 1;
    }
  }

  helper.setMessageValue(outvalue.get(), field, option.value);
}

void CountReduceOperation::aggregate(idgs::expr::ExpressionContext* ctx, ReduceOption& option) {
  auto& outvalue = (* ctx->getOutputValue());

  protobuf::PbVariant value;
  helper.getMessageValue(outvalue.get(), field, value);

  size_t count = (size_t) value;
  if (option.aggrValue.is_null) {
    option.aggrValue = count;
  } else {
    option.aggrValue = (size_t) option.aggrValue + count;
  }

  helper.setMessageValue(outvalue.get(), field, option.aggrValue);
}

void SumReduceOperation::reduce(idgs::expr::ExpressionContext* ctx, ReduceOption& option) {
  auto& outvalue = (* ctx->getOutputValue());

  protobuf::PbVariant value;
  helper.getMessageValue(outvalue.get(), field, value);

  if (!distinct || !option.isDistinctValue(value)) {
    if (option.value.is_null) {
      option.value = value;
    } else {
      option.value = (double) option.value + (double) value;
    }
  }

  helper.setMessageValue(outvalue.get(), field, option.value);
}

void SumReduceOperation::aggregate(idgs::expr::ExpressionContext* ctx, ReduceOption& option) {
  auto& outvalue = (* ctx->getOutputValue());

  protobuf::PbVariant value;
  helper.getMessageValue(outvalue.get(), field, value);

  double sum = (double) value;
  if (option.aggrValue.is_null) {
    option.aggrValue = sum;
  } else {
    option.aggrValue = (double) option.aggrValue + sum;
  }

  helper.setMessageValue(outvalue.get(), field, option.aggrValue);
}

void MaxReduceOperation::reduce(idgs::expr::ExpressionContext* ctx, ReduceOption& option) {
  auto& outvalue = (* ctx->getOutputValue());

  protobuf::PbVariant value;
  helper.getMessageValue(outvalue.get(), field, value);

  if (!distinct || !option.isDistinctValue(value)) {
    if (option.value.is_null) {
      option.value = value;
    } else {
      if (option.value >= value) {
        return;
      }
    }
  }

  helper.setMessageValue(outvalue.get(), field, option.value);
}

void MaxReduceOperation::aggregate(idgs::expr::ExpressionContext* ctx, ReduceOption& option) {
  auto& outvalue = (* ctx->getOutputValue());

  protobuf::PbVariant value;
  helper.getMessageValue(outvalue.get(), field, value);

  if (option.aggrValue.is_null) {
    option.aggrValue = value;
  } else {
    if (option.aggrValue >= value) {
      return;
    }
  }

  helper.setMessageValue(outvalue.get(), field, option.aggrValue);
}

void MinReduceOperation::reduce(idgs::expr::ExpressionContext* ctx, ReduceOption& option) {
  auto& outvalue = (* ctx->getOutputValue());

  protobuf::PbVariant value;
  helper.getMessageValue(outvalue.get(), field, value);

  if (!distinct || !option.isDistinctValue(value)) {
    if (option.value.is_null) {
     option.value = value;
    } else {
      if (option.value <= value) {
        return;
      }
    }
  }

  helper.setMessageValue(outvalue.get(), field, option.value);
}

void MinReduceOperation::aggregate(idgs::expr::ExpressionContext* ctx, ReduceOption& option) {
  auto& outvalue = (* ctx->getOutputValue());

  protobuf::PbVariant value;
  helper.getMessageValue(outvalue.get(), field, value);

  if (option.aggrValue.is_null) {
    option.aggrValue = value;
  } else {
    if (option.aggrValue <= value) {
      return;
    }
  }

  helper.setMessageValue(outvalue.get(), field, option.aggrValue);
}

void AvgReduceOperation::reduce(idgs::expr::ExpressionContext* ctx, ReduceOption& option) {
  auto& outvalue = (* ctx->getOutputValue());

  protobuf::PbVariant value;
  helper.getMessageValue(outvalue.get(), field, value);

  if (!distinct || !option.isDistinctValue(value)) {
    if (option.value.is_null) {
      option.value = value;
      option.reduceCount = 1;
    } else {
      option.value = (double) option.value + (double) value;
      option.reduceCount += 1;
    }
  }

  protobuf::PbVariant var = ((double) option.value / option.reduceCount);
  helper.setMessageValue(outvalue.get(), field, var);
}

void AvgReduceOperation::aggregate(idgs::expr::ExpressionContext* ctx, ReduceOption& option) {
  LOG(ERROR) << "Not supported yet.";
}

void ExprReduceOperation::reduce(idgs::expr::ExpressionContext* ctx, ReduceOption& option) {
  if (!option.value.is_null) {
    return;
  }

  auto& outvalue = (* ctx->getOutputValue());

  protobuf::PbVariant value;
  helper.getMessageValue(outvalue.get(), field, value);
  option.value = value;
}

void ExprReduceOperation::aggregate(idgs::expr::ExpressionContext* ctx, ReduceOption& option) {
  if (!option.value.is_null) {
    return;
  }

  auto& outvalue = (* ctx->getOutputValue());

  protobuf::PbVariant value;
  helper.getMessageValue(outvalue.get(), field, value);
  option.value = value;
}

ReduceOperation* ReduceOperationFactory::getOption() {
  return getOption("EXPR");
}

ReduceOperation* ReduceOperationFactory::getOption(const std::string& name) {
  if (name == "EXPR") {
    return new ExprReduceOperation;
  } else if (name == "COUNT") {
    return new CountReduceOperation;
  } else if (name == "SUM") {
    return new SumReduceOperation;
  } else if (name == "MAX") {
    return new MaxReduceOperation;
  } else if (name == "MIN") {
    return new MinReduceOperation;
  } else if (name == "AVG") {
    return new AvgReduceOperation;
  } else {
    return NULL;
  }
}

} /* namespace op */
} /* namespace rdd */
} /* namespace idgs */
