
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "reducebykey_transformer.h"
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

ReduceByKeyTransformer::ReduceByKeyTransformer() {

}

ReduceByKeyTransformer::~ReduceByKeyTransformer() {
}

RddResultCode ReduceByKeyTransformer::parseOperations(const ActorMessagePtr& msg, const BaseRddPartition* input,
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

void ReduceByKeyTransformer::handleReduceResult(vector<ReduceOperation*>& operations, const PbMessagePtr& key,
    RddPartition* output, bool reuse_key) {
  PbMessagePtr new_key;
  PbMessagePtr new_value(output->getValueTemplate()->New());
  if (reuse_key) {
    new_key = key;
    for (auto it = operations.begin(); it != operations.end(); ++it) {
      if ((*it)->key) {
        continue;
      }
      protobuf::MessageHelper().setMessageValue(new_value.get(), (*it)->descriptor, (*it)->getResult());
      (*it)->reset(); /// clear reduce value for next group reduce loop
    }
  } else {
    new_key.reset(output->getKeyTemplate()->New());
    for (auto it = operations.begin(); it != operations.end(); ++it) {
      if ((*it)->key) {
        protobuf::MessageHelper().setMessageValue(new_key.get(), (*it)->descriptor, (*it)->getResult());
      } else {
        protobuf::MessageHelper().setMessageValue(new_value.get(), (*it)->descriptor, (*it)->getResult());
      }
      (*it)->reset(); /// clear reduce value for next group reduce loop
    }
  }
  output->putLocal(new_key, new_value);
}

RddResultCode ReduceByKeyTransformer::transform(const ActorMessagePtr& msg, const vector<BaseRddPartition*>& input,
    RddPartition* output) {
  if (input.size() != 1) {
    LOG(ERROR)<<"reduce value transformer need one input RDD";
    return RRC_INVALID_RDD_INPUT;
  }

  if (input[0]->empty()) {
    return RRC_SUCCESS;
  }

  if(!input[0]->getKeyTemplate().get()) {
    LOG(ERROR) << "KEY Template is empty";
    return RRC_INVALID_KEY;
  }

  if(!input[0]->getValueTemplate().get()) {
    LOG(ERROR) << "VALUE Template is empty";
    return RRC_INVALID_VALUE;
  }

  vector<ReduceOperation*> operations;
  RddResultCode code = parseOperations(msg, input[0], output, operations);
  if(RRC_SUCCESS != code) {
    LOG(ERROR) << "parse reduce operation error, code = " << code;
    return code;
  }

  auto filterExpr = output->getFilterExpression(0);
  ExpressionContext ctx;
  input[0]->foreachGroup([this, input, output, &operations, filterExpr, &code, &ctx] (const PbMessagePtr& key, const vector<PbMessagePtr>& values) {
        if(!key.get()) { /// key is empty
          LOG(ERROR) << "reduce value transformer error, key is empty";
          return RRC_INVALID_KEY;
        }
        for(auto value = values.begin(); value != values.end(); ++value) { /// loop values
          if(!(*value).get()) {
            LOG_FIRST_N(ERROR, 10) << "value is null, ignored";
            continue;
          }
          try {
            if (filterExpr) {
              ctx.setKeyValue(&key, &(*value));
              PbVariant var = filterExpr->evaluate(&ctx);
              if (!(bool) var) {
                continue;
              }
            }
          } catch (RddResultCode& err) {
            LOG(ERROR) << "evaluate error, caused by code " << err;
            code = err;
          }

          for(auto op = operations.begin(); op != operations.end(); ++op) {
            (*op)->reduce(key, *value);
          }
        } /// end loop values
        bool reuse_key = input[0]->getKeyTemplate()->GetDescriptor()->full_name() == output->getKeyTemplate()->GetDescriptor()->full_name();
        handleReduceResult(operations, key, output, reuse_key);
      });

  return code;
}

} // namespace transform
} // namespace rdd
} // namespace idgs

