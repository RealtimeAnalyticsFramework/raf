
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "reduce_transformer.h"
#include "protobuf/message_helper.h"

using namespace protobuf;
using namespace idgs::actor;
using namespace idgs::rdd::pb;
using namespace idgs::expr;
using namespace idgs::expr;
using namespace google::protobuf;

using namespace idgs::rdd::transform::reduce;

namespace idgs {
namespace rdd {
namespace transform {

ReduceTransformer::ReduceTransformer() {

}

ReduceTransformer::~ReduceTransformer() {
}

RddResultCode ReduceTransformer::parseOperations(const ActorMessagePtr& msg, const BaseRddPartition* input,
    RddPartition* output, vector<ReduceOperation*>& operations) {
  ReduceByKeyRequest reduce_request;
  if (!msg->parseAttachment(TRANSFORMER_PARAM, &reduce_request)) {
    LOG(ERROR)<< "Not exists attachment: ReduceValueRequest";
    return RRC_INVALID_TRANSFORMER_PARAM;
  }
  std::map<std::string, ReduceByKeyField> reduce_value_map; /// Key: field_alias, Value: ReduceValue
  for (auto it = reduce_request.fields().begin(); it != reduce_request.fields().end(); ++it) {
    reduce_value_map.insert(std::pair<std::string, ReduceByKeyField>(it->field_alias(), *it));
  }

  auto outMsg = output->getOutMessage(0);
  auto& keyFields = outMsg.key.getFields();
  auto& keyExprs = outMsg.key.getExpressions();
  for (int32_t i = 0; i < keyFields.size(); ++i) {
    ReduceOperation* op = NULL;
    std::string alias = keyFields[i]->name();
    auto reduce_it = reduce_value_map.find(alias);
    if (reduce_it != reduce_value_map.end()) { /// reduce value field
      op = ReduceOperationFactory::create(reduce_it->second.type()); /// which reduce type, e.g. count, sum, ... etc
      op->is_distinct = reduce_it->second.distinct(); /// whether including distinct, e.g. count(distinct), avg(distinct)
    } else { /// not reduce value field
      op = ReduceOperationFactory::create();
    }

    op->expr = keyExprs[i];
    op->descriptor = keyFields[i];
    op->key = true;

    operations.push_back(op);
  }

  auto& valueFields = outMsg.value.getFields();
  auto& valueExprs = outMsg.value.getExpressions();
  for (int32_t i = 0; i < valueFields.size(); ++i) {
    ReduceOperation* op = NULL;
    std::string alias = valueFields[i]->name();
    auto reduce_it = reduce_value_map.find(alias);
    if (reduce_it != reduce_value_map.end()) { /// reduce value field
      op = ReduceOperationFactory::create(reduce_it->second.type()); /// which reduce type, e.g. count, sum, ... etc
      op->is_distinct = reduce_it->second.distinct(); /// whether including distinct, e.g. count(distinct), avg(distinct)
    } else { /// not reduce value field
      op = ReduceOperationFactory::create();
    }

    op->expr = valueExprs[i];
    op->descriptor = valueFields[i];
    op->key = false;

    operations.push_back(op);
  }

  return RRC_SUCCESS;
}

RddResultCode ReduceTransformer::transform(const ActorMessagePtr& msg, const vector<BaseRddPartition*>& input, RddPartition* output) {
  if (input.size() != 1) {
    LOG(ERROR)<<"reduce value transformer need one input RDD";
    return RRC_INVALID_RDD_INPUT;
  }

  if (output->getKeyTemplate()->GetDescriptor()->full_name() != "idgs.pb.Integer") {
    LOG(ERROR) << "key message type of reduce transformer must be 'idgs.pb.Integer', but '" << output->getKeyTemplate()->GetDescriptor()->full_name() << "'";
    return RRC_INVALID_KEY;
  }

  vector<ReduceOperation*> operations;
  RddResultCode code = parseOperations(msg, input[0], output, operations);
  if(RRC_SUCCESS != code) {
    LOG(ERROR) << "parse reduce operation error, code = " << code;
    return code;
  }

  auto filterExpr = output->getFilterExpression(0);
  ExpressionContext ctx;
  input[0]->foreach([this, output, &operations, filterExpr, &code, &ctx] (const PbMessagePtr& key, const PbMessagePtr& value) {
      try {
        if (filterExpr) {
          ctx.setKeyValue(&key, &value);
          PbVariant var = filterExpr->evaluate(&ctx);
          if (!(bool) var) {
            return;
          }
        }
      } catch (RddResultCode& err) {
        LOG(ERROR) << "evaluate error, caused by code " << err;
        code = err;
      }

      for(auto op = operations.begin(); op != operations.end(); ++op) {
        (*op)->reduce(key, value);
      }
  });

  std::shared_ptr<idgs::pb::Integer> out_key = std::make_shared<idgs::pb::Integer>();
  out_key->set_value(1);
  PbMessagePtr out_value(output->getValueTemplate()->New());

  for (auto it = operations.begin(); it != operations.end(); ++it) {
    protobuf::MessageHelper().setMessageValue(out_value.get(), (*it)->descriptor, (*it)->getResult());
    (*it)->reset();
  }

  output->put(out_key, out_value);

  return code;
}

} // namespace transform
} // namespace rdd
} // namespace idgs

