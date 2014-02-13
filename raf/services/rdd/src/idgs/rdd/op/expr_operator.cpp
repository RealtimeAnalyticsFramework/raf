/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "expr_operator.h"
#include "idgs/expr/expression_factory.h"

namespace idgs {
namespace rdd {
namespace op {

OutMessageInfo::OutMessageInfo() :
    templateMessage(NULL), reflection(NULL), reuse(REUSE_NONE) {
}

OutMessageInfo::~OutMessageInfo() {
}

void OutMessageInfo::destroy() {
  outFields.clear();
  for (int32_t i = 0; i < srcExprs.size(); ++i) {
    if (srcExprs[i]) {
      delete srcExprs[i];
      srcExprs[i] = NULL;
    }
  }

  srcExprs.clear();
}

void OutMessageInfo::setTemplateMessage(const google::protobuf::Message* templateMessage) {
  assert(templateMessage != NULL);
  this->templateMessage = templateMessage;
  reflection = templateMessage->GetReflection();
}

void OutMessageInfo::setReuse(const ReUseType& reuse) {
  this->reuse = reuse;
}

const ReUseType& OutMessageInfo::getReuse() const {
  return reuse;
}

bool OutMessageInfo::addField(const std::string& fieldName, idgs::expr::Expression* srcExpr) {
  assert(templateMessage != NULL);
  auto outField = templateMessage->GetDescriptor()->FindFieldByName(fieldName);
  auto d = templateMessage->GetDescriptor();
  if (outField == NULL) {
    LOG(ERROR)<< "error in adding field " << fieldName << " for " << templateMessage->GetDescriptor()->full_name();
    return false;
  }

  outFields.push_back(outField);
  srcExprs.push_back(srcExpr);

  return true;
}

void OutMessageInfo::fillMessage(idgs::actor::PbMessagePtr& output, idgs::expr::ExpressionContext& ctx) const {
  if (reuse == REUSE_KEY) {
    output = *ctx.getKey();
  } else if (reuse == REUSE_VALUE) {
    output = *ctx.getValue();
  } else {
    assert(templateMessage != NULL);
    assert(reflection != NULL);
    if (!output) {
      output.reset(templateMessage->New());
    }

    for (int32_t i = 0; i < outFields.size(); ++i) {
      assert(outFields[i] != NULL);
      assert(srcExprs[i] != NULL);

      auto field = outFields[i];
      auto var = srcExprs[i]->evaluate(&ctx);

      switch (field->cpp_type()) {
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT64: {
          reflection->SetUInt64(output.get(), field, (uint64_t) var);
          break;
        }
        case google::protobuf::FieldDescriptor::CPPTYPE_INT64: {
          reflection->SetInt64(output.get(), field, (int64_t) var);
          break;
        }
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT32: {
          reflection->SetUInt32(output.get(), field, (uint32_t) var);
          break;
        }
        case google::protobuf::FieldDescriptor::CPPTYPE_INT32: {
          reflection->SetInt32(output.get(), field, (int32_t) var);
          break;
        }
        case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
          reflection->SetString(output.get(), field, (std::string) var);
          break;
        }
        case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE: {
          reflection->SetDouble(output.get(), field, (double) var);
          break;
        }
        case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT: {
          reflection->SetFloat(output.get(), field, (float) var);
          break;
        }
        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL: {
          reflection->SetBool(output.get(), field, (bool) var);
          break;
        }
        default: {
          continue;
        }
      }
    }
  }
}

const std::vector<const google::protobuf::FieldDescriptor*>& OutMessageInfo::getFields() const {
  return outFields;
}

const std::vector<idgs::expr::Expression*>& OutMessageInfo::getExpressions() const {
  return srcExprs;
}

ExprMapOperator::ExprMapOperator(){

}
ExprMapOperator::~ExprMapOperator(){
  if (filterExpr) {
    delete filterExpr;
    filterExpr = NULL;
  }

  outMsg.key.destroy();
  outMsg.value.destroy();
}

bool ExprMapOperator::parse(const idgs::rdd::pb::InRddInfo& inputRddInfo, const idgs::rdd::pb::OutRddInfo& outputRddInfo, idgs::rdd::BaseRddPartition* inputPartition, idgs::rdd::BaseRddPartition* outputPartition) {
  if (inputRddInfo.has_filter_expr()) {
    auto rc = idgs::expr::ExpressionFactory::build(&filterExpr, inputRddInfo.filter_expr(), inputPartition->getKeyTemplate(), inputPartition->getValueTemplate());
    if (rc != idgs::RC_SUCCESS) {
      LOG(ERROR)<< "parse filter expression error, caused by " << (rc);
    }
  }

  std::map<std::string, idgs::expr::Expression*> exprMap;
  for (int32_t j = 0; j < inputRddInfo.out_fields_size(); ++j) {
    auto fld = inputRddInfo.out_fields(j);
    auto exprEntry = fld.expr();
    std::string alias;
    if (fld.has_field_alias()) {
      alias = fld.field_alias();
    } else {
      if (exprEntry.type() != idgs::pb::FIELD) {
        LOG(ERROR)<< "The expression with type is not FIELD, must has alias.";
        return false;
      } else {
        if (!exprEntry.has_value()) {
          return false;
        }
        alias = exprEntry.value();
      }
    }

    if (exprMap.find(alias) == exprMap.end()) {
      idgs::expr::Expression* expr = NULL;
      auto rc = idgs::expr::ExpressionFactory::build(&expr, exprEntry, inputPartition->getKeyTemplate(), inputPartition->getValueTemplate());
      if (rc != idgs::RC_SUCCESS) {
        return false;
      }

      exprMap[alias] = expr;
    }
  }

  auto reuseKey = idgs::rdd::op::REUSE_NONE;
  auto reuseValue = idgs::rdd::op::REUSE_NONE;
  if (inputPartition->getKeyTemplate()->GetDescriptor()->full_name() == outputPartition->getKeyTemplate()->GetDescriptor()->full_name()) {
    reuseKey = idgs::rdd::op::REUSE_KEY;
  }

  if (inputPartition->getValueTemplate()->GetDescriptor()->full_name() == outputPartition->getValueTemplate()->GetDescriptor()->full_name()) {
    reuseValue = idgs::rdd::op::REUSE_VALUE;
  }

  outMsg.key.setTemplateMessage(outputPartition->getKeyTemplate().get());
  outMsg.key.setReuse(reuseKey);

  for (int32_t j = 0; j < outputRddInfo.key_fields_size(); ++j) {
    auto it = exprMap.find(outputRddInfo.key_fields(j).field_name());
    if (it != exprMap.end()) {
      if (!outMsg.key.addField(outputRddInfo.key_fields(j).field_name(), it->second)) {
        return false;
      }
    }
  }

  outMsg.value.setTemplateMessage(outputPartition->getValueTemplate().get());
  outMsg.value.setReuse(reuseValue);
  for (int32_t j = 0; j < outputRddInfo.value_fields_size(); ++j) {
    auto it = exprMap.find(outputRddInfo.value_fields(j).field_name());
    if (it != exprMap.end()) {
      if (!outMsg.value.addField(outputRddInfo.value_fields(j).field_name(), it->second)) {
        return false;
      }
    }
  }

  exprMap.clear();

  return true;

}


void ExprMapOperator::evaluate(idgs::expr::ExpressionContext* ctx) {
  try {
    if (filterExpr) {
      auto var = filterExpr->evaluate(ctx);
      if (!(bool) var) {
        return;
      }
    }
  } catch (idgs::rdd::pb::RddResultCode& err) {
    LOG(ERROR) << "evaluate error, caused by code " << err;
//    code = err;
  }

  idgs::actor::PbMessagePtr outKey;
  idgs::actor::PbMessagePtr outValue;

  outMsg.key.fillMessage(outKey, *ctx);
  outMsg.value.fillMessage(outValue, *ctx);

  ctx->getOutPartition()->putLocal(outKey, outValue);

}


} /* namespace op */
} /* namespace rdd */
} /* namespace idgs */
