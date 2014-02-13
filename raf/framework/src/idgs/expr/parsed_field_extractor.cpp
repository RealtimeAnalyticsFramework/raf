
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "parsed_field_extractor.h"

#include "idgs/util/utillity.h"

using namespace protobuf;
using namespace idgs::actor;
using namespace google::protobuf;

namespace idgs {
namespace expr {

ParsedFieldExtractor::ParsedFieldExtractor() :
    fieldType(FieldDescriptor::TYPE_STRING), target(T_VALUE) {
}

ParsedFieldExtractor::~ParsedFieldExtractor() {
  parsedFields.clear();
}

bool ParsedFieldExtractor::parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key,
    const idgs::actor::PbMessagePtr& value) {
  if (entryExp.expression_size()) {
    LOG(ERROR)<< "Invalid subexpression: " << entryExp.type();
    return false;
  }

  if (!entryExp.has_value()) {
    LOG(ERROR) << "FIELD Expression must have field path into value.";
    return false;
  }

  PbMessagePtr message;
  vector<string> fields;
  path = entryExp.value();
  str::split(path, ".", fields);
  if (key->GetDescriptor()->FindFieldByName(fields[0])) {
    target = T_KEY;
    message = key;
  } else if (value->GetDescriptor()->FindFieldByName(fields[0])) {
    target = T_VALUE;
    message = value;
  } else {
    LOG(ERROR) << "field path " << path << " is not found in " << key->GetDescriptor()->full_name() << " or " << value->GetDescriptor()->full_name();
    return false;
  }

  auto descriptor = message->GetDescriptor();
  auto reflection = message->GetReflection();

  Message* msg = message.get();
  for (int32_t i = 0; i < fields.size(); ++ i) {
    auto field = descriptor->FindFieldByName(fields[i]);
    if (!field) {
      LOG(ERROR) << "Field " << fields[i] << " of " << descriptor->full_name() << " not found.";
      return false;
    }

    ParsedField pField;
    pField.path = fields[i];
    pField.descriptor = descriptor;
    pField.fieldDescriptor = field;
    pField.reflection = reflection;
    parsedFields.push_back(pField);

    if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      msg = reflection->MutableMessage(msg, field);
      descriptor = msg->GetDescriptor();
      reflection = msg->GetReflection();
    }

    if (i == fields.size() - 1) {
      fieldType = field->type();
    }
  }

  return true;
}

protobuf::PbVariant ParsedFieldExtractor::evaluate(ExpressionContext* ctx) const {
  PbVariant var;
  bool is_key = (target == T_KEY);
  Message* msg = is_key ? (*ctx->getKey()).get() : (*ctx->getValue()).get();
  if (!msg) {
    LOG(ERROR)<< (is_key ? "Key message is null." : "Value message is null");
    return var;
  }

  for (int32_t i = 0; i < parsedFields.size(); ++i) {
    auto field = parsedFields[i].fieldDescriptor;
    auto reflection = parsedFields[i].reflection;

    switch (field->cpp_type()) {
      case FieldDescriptor::CPPTYPE_UINT64: {
        var = reflection->GetUInt64(*msg, field);
        break;
      }
        // handle type int64
      case FieldDescriptor::CPPTYPE_INT64: {
        var = reflection->GetInt64(*msg, field);
        break;
      }
        // handle type unsigned int32
      case FieldDescriptor::CPPTYPE_UINT32: {
        var = reflection->GetUInt32(*msg, field);
        break;
      }
        // handle type int32
      case FieldDescriptor::CPPTYPE_INT32: {
        var = reflection->GetInt32(*msg, field);
        break;
      }
        // handle type string
      case FieldDescriptor::CPPTYPE_STRING: {
        std::string s;
        const std::string& ref = reflection->GetStringReference(*msg, field, &s);
        var = ref;
//        std::string s;
//        auto ref = reflection->GetStringReference(* msg, field, &s);
//        var = new std::string(ref.data(), ref.size());
        break;
      }
        // handle type double
      case FieldDescriptor::CPPTYPE_DOUBLE: {
        var = reflection->GetDouble(*msg, field);
        break;
      }
        // handle type float
      case FieldDescriptor::CPPTYPE_FLOAT: {
        var = reflection->GetFloat(*msg, field);
        break;
      }
        // handle type bool
      case FieldDescriptor::CPPTYPE_BOOL: {
        var = reflection->GetBool(*msg, field);
        break;
      }
        // handle type enum
      case FieldDescriptor::CPPTYPE_ENUM: {
        var = reflection->GetEnum(*msg, field)->number();
        break;
      }
      case FieldDescriptor::CPPTYPE_MESSAGE: {
        msg = reflection->MutableMessage(msg, field);
        break;
      }
      default: {
        LOG(ERROR)<< "This type of protobuf message is not supported yet.";
        break;
      }
    }
  }

  return var;
}

const google::protobuf::FieldDescriptor::Type& ParsedFieldExtractor::getFieldType() const {
  return fieldType;
}

} // namespace expr
} // namespace idgs
