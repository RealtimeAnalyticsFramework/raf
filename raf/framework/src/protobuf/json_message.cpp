
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "protobuf/json_message.h"

#include <fstream>
#include "idgs/util/utillity.h"

using namespace idgs;
using namespace google::protobuf;

namespace protobuf {

std::string JsonMessage::toPrettyJsonString(const google::protobuf::Message* message) {
  return toIndentJsonString(toJsonString(message));
}

std::string JsonMessage::toIndentJsonString(const std::string& json) {
  std::stringstream result;
  size_t pos = 0;
  const std::string& indent_str = "  "; /// two space
  string prev_char = ""; /// character before current character 'ch'
  bool is_out_quote = true; /// if character is "
  for (size_t i = 0, size = json.length(); i < size; ++i) {
    string ch = json.substr(i, 1); // sub string one character
    if (ch == "\"" && prev_char != "\\") {
      is_out_quote = !is_out_quote;
    } else if ((ch == "}" || ch == "]") && is_out_quote) {
      result << std::endl;
      --pos;
      for (size_t j = 0; j < pos; ++j) {
        result << indent_str;
      }
    }
    result << ch;
    if ((ch == "," || ch == "{" || ch == "[") && is_out_quote) {
      result << std::endl;
      if (ch == "{" || ch == "[") {
        ++pos;
      }
      for (size_t j = 0; j < pos; ++j) {
        result << indent_str;
      }
    }
    prev_char = ch;
  }
  return result.str();
}

string JsonMessage::toJsonString(const Message* message) {

  assert(message != NULL);

  string json;
  const Descriptor* descriptor = message->GetDescriptor();

  bool first = true;
  json.append("{");
  for (int32_t i = 0; i < descriptor->field_count(); ++i) {
    const FieldDescriptor* field = descriptor->field(i);
    // append field to json
    auto fs = fieldToJson(message, field);
    if(!fs.empty()) {
      if(first) {
        first = false;
      } else {
        json.append(",");
      }
      json.append(fs);
    }
  }

  json.append("}");

  return json;
}

ResultCode JsonMessage::parseJsonFromString(Message* message, const string& json) {
  ResultCode resultCode = RC_SUCCESS;
  if (!message) {
    resultCode = RC_INVALID_MESSAGE;
  } else {
    char err[255];
    // parse json string
    yajl_val root = yajl_tree_parse(json.c_str(), err, 255);

    // whether occur error
    if (strcmp(err, "") != 0) {
      DLOG(ERROR)<< "Parsing json error : " << err;
      resultCode = RC_JSON_PARSE_ERROR;
    } else {
      // set value of each field from yajl struct
      resultCode = jsonToField(message, root);
    }

    // free
    yajl_tree_free(root);
  }

  return resultCode;
}

ResultCode JsonMessage::parseJsonFromFile(Message* message, const string& filePath) {
  // open json file
  ifstream ifs;
  ifs.open(filePath.c_str(), ifstream::in);

  // whether file not found
  if (!ifs.is_open()) {
    return RC_FILE_NOT_FOUND;
  }

  // get file length
  ifs.seekg(0, ios::end);
  int length = ifs.tellg();
  ifs.seekg(0, ios::beg);

  // read file content
  string buff;
  buff.resize(length);
  ifs.read(const_cast<char*>(buff.data()), length);
  ifs.close();

  // parse
  return parseJsonFromString(message, buff);
}

// define repeated or unrepeated field to json string
#define FIELD_TO_JSON(TYPE, METHOD, FIELD_VALUE_EXPR) \
  if (field->is_repeated()) { \
    int32_t size = reflection->FieldSize(* message, field); \
    if (size > 0) { \
      fieldJson.append("\"").append(field->name()).append("\":"); \
      fieldJson.append("["); \
      for (int32_t i = 0; i < size; i ++) { \
        TYPE value = reflection->GetRepeated##METHOD(* message, field, i); \
        fieldJson.append((i > 0) ? "," : "").append(FIELD_VALUE_EXPR); \
      } \
      fieldJson.append("]"); \
    } \
  } else { \
    if (reflection->HasField(* message, field)) { \
      TYPE value = reflection->Get##METHOD(* message, field); \
      fieldJson.append("\"").append(field->name()).append("\":"); \
      fieldJson.append(FIELD_VALUE_EXPR); \
    } \
  }

std::string JsonMessage::filteString(const std::string& value) {
  char first = value.at(0);
  if (first == '{' || first == '[') {
    return value;
  } else {
    return "\"" + value + "\"";
  }
}

string JsonMessage::fieldToJson(const Message* message, const FieldDescriptor* field) {
  string json, fieldJson;
  auto reflection = message->GetReflection();

  // switch field type and get field value.
  switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32: {
      FIELD_TO_JSON(int32_t, Int32, to_string(value));
      break;
    }
    case FieldDescriptor::CPPTYPE_INT64: {
      FIELD_TO_JSON(int64_t, Int64, to_string(value));
      break;
    }
    case FieldDescriptor::CPPTYPE_UINT32: {
      FIELD_TO_JSON(uint32_t, UInt32, to_string(value));
      break;
    }
    case FieldDescriptor::CPPTYPE_UINT64: {
      FIELD_TO_JSON(uint64_t, UInt64, to_string(value));
      break;
    }
    case FieldDescriptor::CPPTYPE_DOUBLE: {
      FIELD_TO_JSON(double, Double, to_string(value));
      break;
    }
    case FieldDescriptor::CPPTYPE_FLOAT: {
      FIELD_TO_JSON(float, Float, to_string(value));
      break;
    }
    case FieldDescriptor::CPPTYPE_BOOL: {
      FIELD_TO_JSON(bool, Bool, (value) ? "true" : "false");
      break;
    }
    case FieldDescriptor::CPPTYPE_ENUM: {
      FIELD_TO_JSON(const EnumValueDescriptor*, Enum, "\"" + value->name() + "\"");
      break;
    }
    case FieldDescriptor::CPPTYPE_STRING: {
      /// @fixme the string must be escaped. e.g. if value is "hello \n\"world", the result will be illegal.
      //FIELD_TO_JSON(string, String, "\"" + value + "\"");
      FIELD_TO_JSON(string, String, filteString(value));
      break;
    }
    case FieldDescriptor::CPPTYPE_MESSAGE: {
      FIELD_TO_JSON(const Message&, Message, toJsonString(&value));
      break;
    }
    default: {
      LOG(ERROR)<< "The type of protobuf is not supported";
      break;
    }
  }

  // append fieldJson to json string
  json.append(fieldJson);
  return json;
}

// define set or add number value to field from node of json
#define JSON_TO_NUMBER_FIELD(NODE, TYPE, METHOD) TYPE value = 0; \
  if (YAJL_IS_STRING(NODE)) { \
    resultCode = idgs::sys::convert<TYPE>(string(NODE->u.string), value); \
  } else if (YAJL_IS_NUMBER(NODE)) { \
    resultCode = idgs::sys::convert<TYPE>(string(NODE->u.number.r), value); \
  } else { \
    resultCode = RC_JSON_MESSAGE_NOT_MATCH; \
  } \
  if (resultCode != RC_SUCCESS) { \
    LOG(ERROR) << "field " << field->name() << " is not matched json."; \
    return resultCode; \
  } \
  reflection->METHOD(message, field, value);

// define set or add bool value to field from node of json
#define JSON_TO_BOOL_FIELD(NODE, TYPE, METHOD) TYPE value = false; \
  if (YAJL_IS_STRING(NODE)) { \
    string nodeValue(NODE->u.string); \
    if (idgs::str::toUpper(nodeValue) == "TRUE") { \
      value = true; \
    } else if (idgs::str::toUpper(nodeValue) == "FALSE") { \
      value = false; \
    } else { \
      resultCode = idgs::sys::convert<bool>(nodeValue, value); \
    } \
  } else if (YAJL_IS_NUMBER(NODE)) { \
    value = NODE->u.number.i != 0; \
  } else if (YAJL_IS_TRUE(NODE)) { \
    value = true; \
  } else if (YAJL_IS_FALSE(NODE)) { \
    value = false; \
  } else { \
    resultCode = RC_JSON_MESSAGE_NOT_MATCH; \
  } \
  if (resultCode != RC_SUCCESS) { \
    LOG(ERROR) << "field " << field->name() << " is not matched json."; \
    return resultCode; \
  } \
  reflection->METHOD(message, field, value);

// define set or add enum value to field from node of json
#define JSON_TO_ENUM_FIELD(NODE, TYPE, METHOD) TYPE value = NULL; \
  if (YAJL_IS_STRING(NODE)) { \
    value = field->enum_type()->FindValueByName(string(NODE->u.string)); \
  } else if (YAJL_IS_NUMBER(NODE)) { \
    value = field->enum_type()->FindValueByNumber(NODE->u.number.i); \
  } else { \
    resultCode = RC_JSON_MESSAGE_NOT_MATCH; \
  } \
  if (resultCode != RC_SUCCESS) { \
    LOG(ERROR) << "field " << field->name() << " is not matched json."; \
    return resultCode; \
  } \
  reflection->METHOD(message, field, value);

// define set or add string value to field from node of json
#define JSON_TO_STRING_FIELD(NODE, TYPE, METHOD) TYPE value; \
  if (YAJL_IS_STRING(NODE)) { \
    value = NODE->u.string; \
  } else { \
    resultCode = RC_JSON_MESSAGE_NOT_MATCH; \
  } \
  if (resultCode != RC_SUCCESS) { \
    LOG(ERROR) << "field " << field->name() << " is not matched json."; \
    return resultCode; \
  } \
  reflection->METHOD(message, field, value);

// define set or add message value to field from node of json
#define JSON_TO_MESSAGE_FIELD(NODE, TYPE, METHOD) TYPE value = NULL; \
  value = reflection->METHOD(message, field); \
  if ((resultCode = jsonToField(value, NODE)) != RC_SUCCESS) { \
    return resultCode; \
  }

// define set or add value to field from node of json
#define JSON_TO_FIELD(NODETYPE, TYPE, SETMETHOD, ADDMETHOD) if (field->is_repeated()) { \
    for (size_t i = 0; i < fieldNode->u.array.len; ++ i) { \
      auto nd = fieldNode->u.array.values[i]; \
      JSON_TO_##NODETYPE##_FIELD(nd, TYPE, ADDMETHOD); \
    } \
  } else { \
    JSON_TO_##NODETYPE##_FIELD(fieldNode, TYPE, SETMETHOD); \
  }

ResultCode JsonMessage::jsonToField(Message* message, const yajl_val& node) {
  if (!YAJL_IS_OBJECT(node)) {
    return RC_JSON_MESSAGE_NOT_MATCH;
  }

  ResultCode resultCode = RC_SUCCESS;
  const Descriptor* descriptor = message->GetDescriptor();
  const Reflection* reflection = message->GetReflection();

  for (size_t i = 0; i < node->u.object.len; ++i) {
    yajl_val fieldNode = node->u.object.values[i];
    const FieldDescriptor* field = descriptor->FindFieldByName(node->u.object.keys[i]);
    if (field == NULL) {
      LOG(ERROR) << "field " << node->u.object.keys[i] << " is not found from message " << descriptor->full_name() << ".";
      return RC_JSON_MESSAGE_NOT_MATCH;
    }

    if ((YAJL_IS_ARRAY(fieldNode) && !field->is_repeated()) || (!YAJL_IS_ARRAY(fieldNode) && field->is_repeated())) {
      LOG(ERROR) << "field " << node->u.object.keys[i] << " is not matched from json and proto.";
      return RC_JSON_MESSAGE_NOT_MATCH;
    }

    switch (field->cpp_type()) {
      case FieldDescriptor::CPPTYPE_INT32: {
        JSON_TO_FIELD(NUMBER, int32_t, SetInt32, AddInt32);
        break;
      }
      case FieldDescriptor::CPPTYPE_INT64: {
        JSON_TO_FIELD(NUMBER, int64_t, SetInt64, AddInt64);
        break;
      }
      case FieldDescriptor::CPPTYPE_UINT32: {
        JSON_TO_FIELD(NUMBER, uint32_t, SetUInt32, AddUInt32);
        break;
      }
      case FieldDescriptor::CPPTYPE_UINT64: {
        JSON_TO_FIELD(NUMBER, uint64_t, SetUInt64, AddUInt64);
        break;
      }
      case FieldDescriptor::CPPTYPE_DOUBLE: {
        JSON_TO_FIELD(NUMBER, double, SetDouble, AddDouble);
        break;
      }
      case FieldDescriptor::CPPTYPE_FLOAT: {
        JSON_TO_FIELD(NUMBER, float, SetFloat, AddFloat);
        break;
      }
      case FieldDescriptor::CPPTYPE_BOOL: {
        JSON_TO_FIELD(BOOL, bool, SetBool, AddBool);
        break;
      }
      case FieldDescriptor::CPPTYPE_ENUM: {
        JSON_TO_FIELD(ENUM, const EnumValueDescriptor*, SetEnum, AddEnum);
        break;
      }
      case FieldDescriptor::CPPTYPE_STRING: {
        JSON_TO_FIELD(STRING, string, SetString, AddString);
        break;
      }
      case FieldDescriptor::CPPTYPE_MESSAGE: {
        JSON_TO_FIELD(MESSAGE, Message*, MutableMessage, AddMessage);
        break;
      }
      default: {
        LOG(ERROR)<< "The type of protobuf is not supported";
        resultCode = RC_NOT_SUPPORT;
        break;
      }
    }
  }

  return RC_SUCCESS;
}

} /* namespace idgs */
