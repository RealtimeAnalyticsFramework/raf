/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "action_context.h"

#include "idgs/rdd/pb/rdd_svc.pb.h"
#include "idgs/expr/expression_factory.h"

using namespace idgs::actor;

namespace idgs {
namespace rdd {
namespace action {

ActionContext::ActionContext() : actionMsg(NULL), keyTemplate(NULL), valueTemplate(NULL),
    expression(NULL), filterExpr(NULL), limit(-1), serdesMode(protobuf::PB_BINARY) {
}

ActionContext::~ActionContext() {
  if (expression) {
    delete expression;
    expression = NULL;
  }

  if (filterExpr) {
    delete filterExpr;
    filterExpr = NULL;
  }

  if (!partitionResults.empty()) {
    partitionResults.clear();
  }

  if (!aggrResults.empty()) {
    auto it = aggrResults.begin();
    for (; it != aggrResults.end(); ++ it) {
      it->clear();
    }
    aggrResults.clear();
  }
}

void ActionContext::initContext(const idgs::actor::ActorMessagePtr* actionMessage) {
  actionMsg = actionMessage;
  pb::ActionRequest* request = dynamic_cast<pb::ActionRequest*>((* actionMsg)->getPayload().get());
  if (request->has_limit()) {
    limit = request->limit();
  } else {
    limit = -1;
  }

  auto serdesType = (* actionMsg)->getSerdesType();
  serdesMode = static_cast<protobuf::SerdesMode>(serdesType);
}

bool ActionContext::initContext(const ActorMessagePtr* actionMessage, const PbMessagePtr* key, const PbMessagePtr* value) {
  pb::ActionRequest* request = dynamic_cast<pb::ActionRequest*>((* actionMessage)->getPayload().get());
  if (request->has_expression()) {
    if (idgs::expr::ExpressionFactory::build(&expression, request->expression(), * key, * value) != RC_SUCCESS) {
      LOG(ERROR) << "invalid expression.";
      return false;
    }
  }

  if (request->has_filter()) {
    if (idgs::expr::ExpressionFactory::build(&filterExpr, request->filter(), * key, * value) != RC_SUCCESS) {
      LOG(ERROR) << "invalid filter expression.";
      return false;
    }
  }

  actionMsg = actionMessage;
  keyTemplate = key;
  valueTemplate = value;
  if (request->has_limit()) {
    limit = request->limit();
  } else {
    limit = -1;
  }

  auto serdesType = (* actionMsg)->getSerdesType();
  serdesMode = static_cast<protobuf::SerdesMode>(serdesType);

  return true;
}

bool ActionContext::getActionParam(const std::string& name, google::protobuf::Message* param) {
  if (!param) {
    return false;
  }

  return (* actionMsg)->parseAttachment(name, param);
}

const protobuf::SerdesMode& ActionContext::getSerdesMode() const {
  return serdesMode;
}

const PbMessagePtr& ActionContext::getKeyTemplate() const {
  return * keyTemplate;
}

const PbMessagePtr& ActionContext::getValueTemplate() const {
  return * valueTemplate;
}

protobuf::PbVariant ActionContext::evaluateExpr() {
  protobuf::PbVariant var;
  if (expression) {
    var = expression->evaluate(getExpressionContext());
  }
  return var;
}

bool ActionContext::evaluateFilterExpr() {
  if (!filterExpr) {
    return true;
  }
  return filterExpr->evaluate(getExpressionContext());
}

bool ActionContext::hasFilterExpr() {
  return filterExpr != NULL;
}

void ActionContext::setKeyValue(const PbMessagePtr* key, const PbMessagePtr* value) {
  getExpressionContext()->setKeyValue(key, value);
}

void ActionContext::setActionResult(const PbMessagePtr& result) {
  actionResult = result;
}

const PbMessagePtr& ActionContext::getActionResult() const {
  return actionResult;
}

void ActionContext::addPartitionResult(const protobuf::PbVariant& result) {
  partitionResults.push_back(const_cast<protobuf::PbVariant&>(result));
}

const std::vector<protobuf::PbVariant>& ActionContext::getPartitionResult() const {
  return partitionResults;
}

void ActionContext::setAggregateResult(const std::vector<std::vector<std::string>>& result) {
  aggrResults = result;
}

const std::vector<std::vector<std::string>>& ActionContext::getAggregateResult() const {
  return aggrResults;
}

int64_t ActionContext::getLimit() {
  return limit;
}

} /* namespace action */
} /* namespace rdd */
} /* namespace idgs */
