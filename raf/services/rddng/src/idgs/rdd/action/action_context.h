/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/expr/expression.h"

namespace idgs {
namespace rdd {
namespace action {

class ActionContext {
public:
  ActionContext();
  virtual ~ActionContext();

public:
  void initContext(const idgs::actor::ActorMessagePtr* actionMessage);
  bool initContext(const idgs::actor::ActorMessagePtr* actionMessage, const idgs::actor::PbMessagePtr* key, const idgs::actor::PbMessagePtr* value);
  bool getActionParam(const std::string& name, google::protobuf::Message* param);
  const protobuf::SerdesMode& getSerdesMode() const;

  const idgs::actor::PbMessagePtr& getKeyTemplate() const;
  const idgs::actor::PbMessagePtr& getValueTemplate() const;

  protobuf::PbVariant evaluateExpr();
  bool evaluateFilterExpr();
  bool hasFilterExpr();

  void setKeyValue(const idgs::actor::PbMessagePtr* key, const idgs::actor::PbMessagePtr* value);
  idgs::expr::ExpressionContext* getExpressionContext();

  void setActionResult(const idgs::actor::PbMessagePtr& result);
  const idgs::actor::PbMessagePtr& getActionResult() const;

  void addPartitionResult(const protobuf::PbVariant& result);
  const std::vector<protobuf::PbVariant>& getPartitionResult() const;

  void setAggregateResult(const std::vector<std::vector<std::string>>& result);
  const std::vector<std::vector<std::string>>& getAggregateResult() const;

  int64_t getLimit();

private:
  const idgs::actor::ActorMessagePtr* actionMsg;
  const idgs::actor::PbMessagePtr* keyTemplate;
  const idgs::actor::PbMessagePtr* valueTemplate;
  idgs::expr::Expression* expression;
  idgs::expr::Expression* filterExpr;

  idgs::actor::PbMessagePtr actionResult;
  std::vector<protobuf::PbVariant> partitionResults;
  std::vector<std::vector<std::string>> aggrResults;
  int64_t limit;
  protobuf::SerdesMode serdesMode;
};

inline idgs::expr::ExpressionContext* ActionContext::getExpressionContext() {
  return &idgs::expr::getTlExpressionContext();
}

} /* namespace action */
} /* namespace rdd */
} /* namespace idgs */
