/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/rdd/op/rdd_operator.h"
#include "idgs/rdd/op/reduce_option.h"
#include "protobuf/msg_comparer.h"

namespace idgs {
namespace rdd {
class BaseRddPartition;

namespace transform {

class TransformerContext {
public:
  TransformerContext();
  virtual ~TransformerContext();

public:
  void setTransformerMsg(const idgs::actor::ActorMessagePtr& transformerMsg);
  bool getTransformerParam(const std::string& name, google::protobuf::Message* param);

  void setRddOperator(const idgs::rdd::op::RddOperator* rddOperator);
  idgs::rdd::op::RddOperator* getRddOperator() const;

  idgs::expr::ExpressionContext* getExpressionContext();

  void setParamRdds(const std::vector<idgs::rdd::BaseRddPartition*>& parameters);
  const std::vector<idgs::rdd::BaseRddPartition*>& getParamRdds() const;

  idgs::rdd::op::ReduceOptions& getReduceOption(const idgs::actor::PbMessagePtr& key, const int32_t& reduceSize);

private:
  idgs::actor::ActorMessagePtr msg;
  idgs::rdd::op::RddOperator* rddOp;
  std::vector<idgs::rdd::BaseRddPartition*> paramRdds;

  std::map<idgs::actor::PbMessagePtr, idgs::rdd::op::ReduceOptions, protobuf::sharedless> reduceOptions;
};

inline idgs::expr::ExpressionContext* TransformerContext::getExpressionContext() {
  return &idgs::expr::getTlExpressionContext();
}

} /* namespace transform */
} /* namespace rdd */
} /* namespace idgs */
