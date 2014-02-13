
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $

#include "top_n_action.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/expr/expression_factory.h"
#include "idgs/expr/parsed_field_extractor.h"
#include "idgs/rdd/pb/rdd_action.pb.h"
#include "idgs/util/utillity.h"
#include "protobuf/message_helper.h"
#include "protobuf/type_composer.h"
#include "idgs/store/data_map.h"

using namespace std;
using namespace idgs::actor;
using namespace idgs::util;
using namespace idgs::expr;
using namespace idgs::rdd::pb;
using namespace protobuf;

namespace idgs {
namespace rdd {
namespace action {

TopNAction::TopNAction() {
}

TopNAction::~TopNAction() {
}

RddResultCode TopNAction::action(const ActorMessagePtr& msg, const BaseRddPartition* input, vector<PbVariant>& output) {
  ActionRequest* payload = dynamic_cast<ActionRequest*>(msg->getPayload().get());

  if (input->empty()) {
    return RRC_SUCCESS;
  }

  Expression* filterExpr = NULL;
  if (payload->has_filter()) {
    auto rc = ExpressionFactory::build(&filterExpr, payload->filter(), input->getKeyTemplate(),
        input->getValueTemplate());
    if (idgs::RC_SUCCESS != rc) {
      LOG(ERROR)<< "RDD \"" << input->getRddName() << "\" partition[" << input->getPartition() << "] parsed filter expression error, caused by " << idgs::getErrorDescription(rc);
      return RRC_NOT_SUPPORT;
    }
  }

  TopNActionRequest request;
  if (!msg->parseAttachment(ACTION_PARAM, &request)) {
    LOG(ERROR)<< "RDD \"" << input->getRddName() << "\" partition[" << input->getPartition() << "] parse top N expression error";
    return RRC_INVALID_ACTION_PARAM;
  }

  int32_t fieldSize = request.order_field_size();
  vector<FieldInfo> fields;
  vector<bool> fieldDesc;
  fields.resize(fieldSize);
  fieldDesc.resize(fieldSize);
  string typeName = input->getRddName();
  for (int32_t i = 0; i < fieldSize; ++i) {
    auto& field = request.order_field(i);
    auto rc = ExpressionFactory::build(&fields[i].expr, field.expr(), input->getKeyTemplate(),
        input->getValueTemplate());
    if (idgs::RC_SUCCESS != rc) {
      LOG(ERROR)<< "RDD \"" << input->getRddName() << "\" partition[" << input->getPartition() << "] parsed filter expression error, caused by " << idgs::getErrorDescription(rc);
      return RRC_NOT_SUPPORT;
    }

    if (field.has_field_name()) {
      fields[i].name = field.field_name();
    } else if (field.expr().type() == idgs::pb::FIELD) {
      fields[i].name = field.expr().value();
    } else {
      fields[i].name = "column" + to_string(i);
    }

    if (field.has_field_type()) {
      fields[i].type = field.field_type();
    } else if (field.expr().type() == idgs::pb::FIELD) {
      ParsedFieldExtractor* fieldExt = dynamic_cast<ParsedFieldExtractor*>(fields[i].expr);
      auto fldType = fieldExt->getFieldType();
      fields[i].type = static_cast<idgs::pb::DataType>(fldType);
    } else {
      fields[i].type = idgs::pb::STRING;
    }

    fieldDesc[i] = field.desc();
    typeName.append("_").append(fields[i].name);
  }

  string fullTypeName = "idgs.rdd.pb." + typeName;

  auto& helper = singleton<MessageHelper>::getInstance();

  if (!helper.isMessageRegistered(fullTypeName)) {
    // cppcheck-suppress unreadVariable
    std::lock_guard<std::mutex> lockGuard(lock);
    if (!helper.isMessageRegistered(fullTypeName)) {
      registerTempKeyMessage(input->getPartition(), typeName, fields);
    }
  }

  PbMessagePtr tmpKeyTemp = helper.createMessage(fullTypeName);
  if (!tmpKeyTemp) {
    LOG(ERROR)<< "RDD \"" << input->getRddName() << "\" partition[" << input->getPartition() << "] generate dynamic key error.";
    return RRC_INVALID_KEY;
  }

  orderless compare(fieldDesc);
  multimap<PbMessagePtr, KeyValueMessagePair, orderless> tmpMap(compare);
  ExpressionContext ctx;
  auto tmpDescriptor = tmpKeyTemp->GetDescriptor();

  uint64_t topN = 0, start = 1, dataSize = 0;
  bool hasTop = request.has_top_n();
  if (request.has_start()) {
    start = request.start();
  }

  if (request.has_top_n()) {
    topN = request.top_n();
    dataSize = topN + start - 1;
  }

  input->foreach(
      [filterExpr, &ctx, &tmpMap, fields, tmpKeyTemp, tmpDescriptor, dataSize, hasTop] (const PbMessagePtr& key, const PbMessagePtr& value) {
        ctx.setKeyValue(&key, &value);
        if (filterExpr) {
          if (!(bool)filterExpr->evaluate(&ctx)) {
            return;
          }
        }

        PbMessagePtr tmpKey(tmpKeyTemp->New());
        for (int32_t i = 0; i < tmpDescriptor->field_count(); ++ i) {
          auto field = tmpDescriptor->field(i);
          PbVariant var = fields[i].expr->evaluate(&ctx);
          singleton<MessageHelper>::getInstance().setMessageValue(tmpKey.get(), field, var);
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

    auto itfst = tmpMap.begin();
    auto itlst = tmpMap.end();
    for (; itfst != itlst; ++ itfst) {
      auto pair = data.add_data();
      ProtoSerdes<DEFAULT_PB_SERDES>::serialize(itfst->first.get(), pair->mutable_key());
      ProtoSerdes<DEFAULT_PB_SERDES>::serialize(itfst->second.key->get(), pair->mutable_pair()->mutable_key());
      ProtoSerdes<DEFAULT_PB_SERDES>::serialize(itfst->second.value->get(), pair->mutable_pair()->mutable_value());
    }

    string var;
    ProtoSerdes<DEFAULT_PB_SERDES>::serialize(&data, &var);
    output.push_back(var);
  }

  return RRC_SUCCESS;
}

RddResultCode TopNAction::aggregate(const idgs::actor::ActorMessagePtr& actionRequest, idgs::actor::ActorMessagePtr& actionResponse, const vector<vector<string>>& input) {
  TopNActionRequest request;
  if (!actionRequest->parseAttachment(ACTION_PARAM, &request)) {
    return RRC_INVALID_ACTION_PARAM;
  }

  int32_t fieldSize = request.order_field_size();
  vector<bool> fieldDesc;
  fieldDesc.resize(fieldSize);
  for (int32_t i = 0; i < fieldSize; ++i) {
    auto& field = request.order_field(i);
    fieldDesc[i] = field.desc();
  }

  uint64_t start = (request.has_start()) ? request.start() : 1;
  bool hasTop = request.has_top_n();
  uint64_t dataSize = (hasTop) ? request.top_n() + start - 1 : 0;

  auto& helper = singleton<MessageHelper>::getInstance();
  orderless compare(fieldDesc);
  multimap<PbMessagePtr, KeyValuePair, orderless> tmpMap(compare);

  TopNActionData data;
  auto it = input.begin();
  for (; it != input.end(); ++it) {
    if (!it->empty()) {
      ProtoSerdes<DEFAULT_PB_SERDES>::deserialize((*it)[0], &data);

      for (int32_t i = 0; i < data.data_size(); ++i) {
        auto tmpKey = helper.createMessage(data.key_name());
        auto pair = data.data(i);
        ProtoSerdes<DEFAULT_PB_SERDES>::deserialize(pair.key(), tmpKey.get());

        KeyValuePair kvPair = pair.pair();
        tmpMap.insert(std::pair<PbMessagePtr, KeyValuePair>(tmpKey, kvPair));

        if (hasTop && tmpMap.size() > dataSize) {
          auto itTmp = tmpMap.end();
          --itTmp;
          tmpMap.erase(itTmp);
        }
      }
    }
  }

  shared_ptr<TopNActionResult> result(new TopNActionResult);
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

  actionResponse->setAttachment(ACTION_RESULT, result);

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
  for (; it != fields.end(); ++it) {
    DynamicField dyField = DynamicField("required", idgs::str::toLower(DataType_Name(it->type)), it->name);
    dmKey.addField(dyField);
  }

  composer.addMessage(dmKey);
  string fileName = RDD_DYNAMIC_PROTO_PATH + typeName + "_" + to_string(partition) + ".proto";
  composer.saveFile(fileName);

  auto code = singleton<MessageHelper>::getInstance().registerDynamicMessage(fileName);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "Invalid message type, caused by " << getErrorDescription(code);
  }

  remove(fileName.c_str());
}

} /* namespace op */
} /* namespace rdd */
} /* namespace idgs */
