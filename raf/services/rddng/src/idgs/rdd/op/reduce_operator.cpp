/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "reduce_operator.h"

#include "idgs/rdd/pb/rdd_transform.pb.h"

namespace idgs {
namespace rdd {
namespace op {

ReduceOperator::ReduceOperator() {
}

ReduceOperator::~ReduceOperator() {
  auto it = options.begin();
  for (; it != options.end(); ++ it) {
    if (* it) {
      delete (* it);
      (* it ) = NULL;
    }
  }
  options.clear();
}

bool ReduceOperator::parse(const pb::InRddInfo& inRddInfo, const pb::OutRddInfo& outRddInfo,
      RddLocal* inRddLocal, RddLocal* outRddLocal) {
  auto& msg = outRddLocal->getTransformerMsg();
  pb::CreateRddRequest* request = dynamic_cast<pb::CreateRddRequest*>(msg->getPayload().get());
  if (request->in_rdd_size() != 1) {
    LOG(ERROR) << "RDD with reduce or reduce by key transformer must only have one dependent RDD.";
    return false;
  }

  if (request->transformer_op_name() == REDUCE_TRANSFORMER) {
    if (outRddInfo.key_type_name() != "idgs.pb.Integer") {
      LOG(ERROR) << "Key of reduce transformer must be 'idgs.pb.Integer'.";
      return false;
    }
  }

  if (!mapOperator.parse(inRddInfo, outRddInfo, inRddLocal, outRddLocal)) {
    return false;
  }

  pb::ReduceByKeyRequest reduce;
  if (!msg->parseAttachment(TRANSFORMER_PARAM, &reduce)) {
    LOG(ERROR)<< "Not exists attachment: ReduceValueRequest";
    return false;
  }

  outRddLocal->setPersistType(pb::ORDERED);

  std::map<std::string, pb::ReduceByKeyField> reduce_value_map; /// Key: field_alias, Value: ReduceValue
  for (auto it = reduce.fields().begin(); it != reduce.fields().end(); ++it) {
    reduce_value_map.insert(std::pair<std::string, pb::ReduceByKeyField>(it->field_alias(), * it));
  }

  auto& valueFields = mapOperator.outMsg.value.getFields();
  for (int32_t i = 0; i < valueFields.size(); ++i) {
    ReduceOperation* op = NULL;
    std::string alias = valueFields[i]->name();
    auto reduce_it = reduce_value_map.find(alias);
    if (reduce_it != reduce_value_map.end()) { /// reduce value field
      op = ReduceOperationFactory::getOption(reduce_it->second.type()); /// which reduce type, e.g. count, sum, ... etc
      op->distinct = reduce_it->second.distinct(); /// whether including distinct, e.g. count(distinct), avg(distinct)
    } else { /// not reduce value field
      op = ReduceOperationFactory::getOption();
    }

    op->field = valueFields[i];

    options.push_back(op);
  }

  return true;
}

bool ReduceOperator::evaluate(idgs::expr::ExpressionContext* ctx) {
  return mapOperator.evaluate(ctx);
}


} /* namespace op */
} /* namespace rdd */
} /* namespace idgs */
