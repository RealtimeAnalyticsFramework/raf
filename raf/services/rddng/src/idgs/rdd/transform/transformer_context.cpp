/*
 * transformer_context.cpp
 *
 *  Created on: May 9, 2014
 *      Author: root
 */

#include "transformer_context.h"

namespace idgs {
namespace rdd {
namespace transform {

TransformerContext::TransformerContext() : msg(NULL), rddOp(NULL) {
}

TransformerContext::~TransformerContext() {
  paramRdds.clear();
  reduceOptions.clear();
}

void TransformerContext::setTransformerMsg(const idgs::actor::ActorMessagePtr& transformerMsg) {
  msg = const_cast<idgs::actor::ActorMessagePtr&>(transformerMsg);
}

bool TransformerContext::getTransformerParam(const std::string& name, google::protobuf::Message* param) {
  if (!param) {
    return false;
  }

  return msg->parseAttachment(name, param);
}

void TransformerContext::setRddOperator(const op::RddOperator* rddOperator) {
  rddOp = const_cast<op::RddOperator*>(rddOperator);
}

op::RddOperator* TransformerContext::getRddOperator() const {
  return rddOp;
}

void TransformerContext::setParamRdds(const std::vector<BaseRddPartition*>& parameters) {
  paramRdds = parameters;
}

const std::vector<BaseRddPartition*>& TransformerContext::getParamRdds() const {
  return paramRdds;
}

op::ReduceOptions& TransformerContext::getReduceOption(const idgs::actor::PbMessagePtr& key, const int32_t& reduceSize) {
  auto it = reduceOptions.find(key);
  if (it == reduceOptions.end()) {
    reduceOptions[key].resize(reduceSize);
    return reduceOptions[key];
  } else {
    return it->second;
  }
}

} /* namespace transform */
} /* namespace rdd */
} /* namespace idgs */
