
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $

#include "top_n_action.h"

#include "idgs/expr/expression_factory.h"
#include "idgs/expr/field_extractor.h"

#include "idgs/rdd/rdd_module.h"

#include "idgs/util/utillity.h"

#include "protobuf/type_composer.h"

using namespace std;
using namespace idgs::actor;
using namespace idgs::expr;
using namespace idgs::rdd::pb;
using namespace protobuf;

namespace idgs {
namespace rdd {
namespace action {

struct KeyValueMessagePair {
  const PbMessagePtr* key;
  const PbMessagePtr* value;
};

TopNAction::TopNAction() {
}

TopNAction::~TopNAction() {
}

RddResultCode TopNAction::action(ActionContext* ctx, const BaseRddPartition* input) {
  TopNActionRequest request;
  if (!ctx->getActionParam(ACTION_PARAM, &request)) {
    LOG(ERROR)<< input->getPartitionName() << " parse top N expression error";
    return RRC_INVALID_ACTION_PARAM;
  }

  int32_t fieldSize = request.order_field_size();
  vector<FieldInfo> fields;
  vector<bool> fieldDesc;
  fields.resize(fieldSize);
  fieldDesc.resize(fieldSize);
  string typeName = input->getRddName();
  for (int32_t i = 0; i < fieldSize; ++ i) {
    auto& field = request.order_field(i);
    auto rc = ExpressionFactory::build(&fields[i].expr, field.expr(), ctx->getKeyTemplate(), ctx->getValueTemplate());
    if (idgs::RC_SUCCESS != rc) {
      LOG(ERROR)<< input->getPartitionName() << " parsed filter expression error, caused by " << idgs::getErrorDescription(rc);
      return RRC_NOT_SUPPORT;
    }

    if (field.has_field_name()) {
      fields[i].name = field.field_name();
    } else if (field.expr().name() == "FIELD") {
      fields[i].name = field.expr().value();
    } else {
      fields[i].name = "column" + to_string(i);
    }

    if (field.has_field_type()) {
      fields[i].type = field.field_type();
    } else if (field.expr().name() == "FIELD") {
      FieldExtractor* fieldExt = dynamic_cast<FieldExtractor*>(fields[i].expr);
      auto fldType = fieldExt->getFieldType();
      fields[i].type = static_cast<idgs::pb::DataType>(fldType);
    } else {
      fields[i].type = idgs::pb::STRING;
    }

    fieldDesc[i] = field.desc();
    typeName.append("_").append(fields[i].name);
  }

  string fullTypeName = "idgs.rdd.pb." + typeName;

  auto helper = idgs_rdd_module()->getMessageHelper();
  if (!helper->isMessageRegistered(fullTypeName)) {
    std::lock_guard<std::mutex> lockGuard(lock);
    if (!helper->isMessageRegistered(fullTypeName)) {
      registerTempKeyMessage(input->getPartition(), typeName, fields);
    }
  }

  if (input->empty()) {
    return RRC_SUCCESS;
  }

  PbMessagePtr tmpKeyTemp = helper->createMessage(fullTypeName);
  if (!tmpKeyTemp) {
    LOG(ERROR)<< "RDD \"" << input->getRddName() << "\" partition[" << input->getPartition() << "] generate dynamic key error.";
    return RRC_INVALID_KEY;
  }

  orderless compare(fieldDesc);
  multimap<PbMessagePtr, KeyValueMessagePair, orderless> tmpMap(compare);
  auto tmpDescriptor = tmpKeyTemp->GetDescriptor();

  uint64_t start = 1, dataSize = 0;
  bool hasTop = request.has_top_n();
  if (request.has_start()) {
    start = request.start();
  }

  if (request.has_top_n()) {
    auto topN = request.top_n();
    dataSize = topN + start - 1;
  }

  input->foreach(
    [&ctx, &tmpMap, fields, tmpKeyTemp, tmpDescriptor, dataSize, hasTop, helper]
     (const PbMessagePtr& key, const PbMessagePtr& value) {
      ctx->setKeyValue(&key, &value);
      if (!ctx->evaluateFilterExpr()) {
        return;
      }

      PbMessagePtr tmpKey(tmpKeyTemp->New());
      for (int32_t i = 0; i < tmpDescriptor->field_count(); ++ i) {
        auto field = tmpDescriptor->field(i);
        PbVariant var = fields[i].expr->evaluate(ctx->getExpressionContext());
        helper->setMessageValue(tmpKey.get(), field, var);
      }

      KeyValueMessagePair pair;
      pair.key = &key;
      pair.value = &value;

      tmpMap.insert(std::pair<PbMessagePtr, KeyValueMessagePair>(tmpKey, pair));

      if (hasTop && tmpMap.size() > dataSize) {
        auto itTmp = tmpMap.end();
        -- itTmp;
        tmpMap.erase(itTmp);
      }
    });

  if (!tmpMap.empty()) {
    TopNActionData data;
    data.set_key_name(fullTypeName);

    auto serdesMode = ctx->getSerdesMode();
    auto itfst = tmpMap.begin();
    auto itlst = tmpMap.end();
    for (; itfst != itlst; ++ itfst) {
      auto pair = data.add_data();
      ProtoSerdesHelper::serialize(serdesMode, itfst->first.get(), pair->mutable_key());
      ProtoSerdesHelper::serialize(serdesMode, itfst->second.key->get(), pair->mutable_pair()->mutable_key());
      ProtoSerdesHelper::serialize(serdesMode, itfst->second.value->get(), pair->mutable_pair()->mutable_value());
    }

    string var;
    ProtoSerdesHelper::serialize(serdesMode, &data, &var);
    ctx->addPartitionResult(var);
  }
  return RRC_SUCCESS;
}

RddResultCode TopNAction::aggregate(ActionContext* ctx) {
  TopNActionRequest request;
  if (!ctx->getActionParam(ACTION_PARAM, &request)) {
    return RRC_INVALID_ACTION_PARAM;
  }

  int32_t fieldSize = request.order_field_size();
  vector<bool> fieldDesc;
  fieldDesc.resize(fieldSize);
  for (int32_t i = 0; i < fieldSize; ++ i) {
    auto& field = request.order_field(i);
    fieldDesc[i] = field.desc();
  }

  uint64_t start = (request.has_start()) ? request.start() : 1;
  bool hasTop = request.has_top_n();
  uint64_t dataSize = (hasTop) ? request.top_n() + start - 1 : 0;

  orderless compare(fieldDesc);
  multimap<PbMessagePtr, KeyValuePair, orderless> tmpMap(compare);

  auto helper = idgs_rdd_module()->getMessageHelper();
  TopNActionData data;
  auto& input = ctx->getAggregateResult();
  auto it = input.begin();
  auto serdesMode = ctx->getSerdesMode();
  for (; it != input.end(); ++ it) {
    if (!it->empty()) {
      ProtoSerdesHelper::deserialize(serdesMode, (*it)[0], &data);

      for (int32_t i = 0; i < data.data_size(); ++i) {
        auto tmpKey = helper->createMessage(data.key_name());
        auto pair = data.data(i);
        ProtoSerdesHelper::deserialize(serdesMode, pair.key(), tmpKey.get());

        KeyValuePair kvPair = pair.pair();
        tmpMap.insert(std::pair<PbMessagePtr, KeyValuePair>(tmpKey, kvPair));

        if (hasTop && tmpMap.size() > dataSize) {
          auto itTmp = tmpMap.end();
          -- itTmp;
          tmpMap.erase(itTmp);
        }
      }
    }
  }

  shared_ptr<TopNActionResult> result = make_shared<TopNActionResult>();
  if (!tmpMap.empty()) {
    auto itfst = tmpMap.begin();
    auto itlst = tmpMap.end();
    uint64_t index = 1;
    for (; itfst != itlst; ++ itfst, ++ index) {
      if (index >= start) {
        result->add_pair()->CopyFrom(itfst->second);
      }
    }
  }

  ctx->setActionResult(result);

  return RRC_SUCCESS;
}

void TopNAction::registerTempKeyMessage(const uint32_t& partition, const std::string& typeName,
    const std::vector<FieldInfo>& fields) {
  DynamicTypeComposer composer;
  composer.setPackage("idgs.rdd.pb");
  composer.setName(typeName);

  DynamicMessage dmKey;
  dmKey.name = typeName;
  auto it = fields.begin();
  for (; it != fields.end(); ++ it) {
    DynamicField dyField = DynamicField("required", idgs::str::toLower(DataType_Name(it->type)), it->name);
    dmKey.addField(dyField);
  }

  composer.addMessage(dmKey);
  string fileName = RDD_DYNAMIC_PROTO_PATH + typeName + "_" + to_string(partition) + ".proto";
  composer.saveFile(fileName);

  auto helper = idgs_rdd_module()->getMessageHelper();
  auto code = helper->registerDynamicMessage(fileName);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "Invalid message type, caused by " << getErrorDescription(code);
  }

  remove(fileName.c_str());
}

} /* namespace op */
} /* namespace rdd */
} /* namespace idgs */
