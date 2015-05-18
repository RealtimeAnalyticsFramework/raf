
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "protobuf/protobuf_json.h"
#include <fstream>
#include <algorithm>
#include "idgs/util/utillity.h"

#define MAX_INDENT 64
#define SPACES_PER_INDENT 2

namespace protobuf {
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                         Encode
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

static inline void charToHex(unsigned char c, char * hexBuf) {
  const static char * hexchar = "0123456789ABCDEF";
  hexBuf[0] = hexchar[c >> 4];
  hexBuf[1] = hexchar[c & 0x0F];
}

void json_string_escape(std::ostream& os,
//    char* dst,
//    int* dst_len,
    const unsigned char * str,
    size_t len,
    int escape_solidus = 0) {
  size_t beg = 0;
  size_t end = 0;
  char hexBuf[7];
  hexBuf[0] = '\\'; hexBuf[1] = 'u'; hexBuf[2] = '0'; hexBuf[3] = '0';
  hexBuf[6] = 0;
//  dst_len = 0;

  while (end < len) {
    const char * escaped = NULL;
    switch (str[end]) {
    case '\r': escaped = "\\r"; break;
    case '\n': escaped = "\\n"; break;
    case '\\': escaped = "\\\\"; break;
    /* it is not required to escape a solidus in JSON:
     * read sec. 2.5: http://www.ietf.org/rfc/rfc4627.txt
     * specifically, this production from the grammar:
     *   unescaped = %x20-21 / %x23-5B / %x5D-10FFFF
     */
    case '/': if (escape_solidus) escaped = "\\/"; break;
    case '"': escaped = "\\\""; break;
    case '\f': escaped = "\\f"; break;
    case '\b': escaped = "\\b"; break;
    case '\t': escaped = "\\t"; break;
    default:
      if ((unsigned char) str[end] < 32) {
        charToHex(str[end], hexBuf + 4);
        escaped = hexBuf;
      }
      break;
    }
    if (escaped != NULL) {
//      memcpy(dst + *dst_len, (const char *) (str + beg), end - beg);
//      *dst_len += (end - beg);
      os.write((const char *) (str + beg), end - beg);
//      memcpy(dst + *dst_len, escaped, (unsigned int)strlen(escaped));
//      *dst_len += (unsigned int)strlen(escaped);
      os.write(escaped, (unsigned int)strlen(escaped));
      beg = ++end;
    } else {
      ++end;
    }
  }
//  memcpy(dst + *dst_len, (const char *) (str + beg), end - beg);
//  *dst_len += (end - beg);
  os.write((const char *) (str + beg), end - beg);
}



inline std::ostream& printIndent(std::ostream& os, bool format, int indent) {
  const static std::string spaces(MAX_INDENT * SPACES_PER_INDENT, ' ');
  if (format) {
    if(indent > MAX_INDENT) {
      indent = MAX_INDENT;
    }
    return os.write(spaces.data(), indent * SPACES_PER_INDENT);
  }
  return os;
}

template <typename T>
inline std::ostream& printValue(std::ostream& os, T v, bool /*format*/, int /*indent*/) {
  return (os << v);
}

template <>
inline std::ostream& printValue<const std::string>(std::ostream& os, const std::string v, bool /*format*/, int /*indent*/) {
  os << '"';
  json_string_escape(os, (const unsigned char*)v.data(), v.size());
  os << '"';
  return os;
}

template <>
inline std::ostream& printValue<const google::protobuf::EnumValueDescriptor*>(std::ostream& os, const google::protobuf::EnumValueDescriptor* v, bool /*format*/, int /*indent*/) {
  return os << '"' << v->name() << '"';
}

template <>
inline std::ostream& printValue<const google::protobuf::Message&>(std::ostream& os, const google::protobuf::Message& v, bool format, int indent) {
  return ProtobufJson::toJsonStream(os, &v, format, indent);
}

inline std::ostream& printFieldName(std::ostream& os, const std::string& name, bool format) {
  os << '"';
  json_string_escape(os, (const unsigned char*)name.data(), name.size());
  os << "\":";
  if (format) {
    os << ' ';
  }
  return os;
}

// T (const google::protobuf::Reflection:: *GM) (google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field) const,
// T (const google::protobuf::Reflection:: *GRM) (google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field, int index) const
template <typename T, typename GM, typename GRM>
inline std::ostream& fieldToJsonStream(std::ostream& os, bool format, int indent,
    const google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field,
    const google::protobuf::Reflection* reflection,
    GM getMethod,
    GRM getRepeatedMethod
    ) {
  if (field->is_repeated()) {
    int size = reflection->FieldSize(*message, field);
    int childIndent = indent + 1;
    os << '[';
    for (int i = 0; i < size; ++i) {
      T value = (reflection->*getRepeatedMethod)(*message, field, i);
      if(i > 0) {
        os << ',';
      }
      if (format) {
        os << std::endl;
      }
      printIndent(os, format, childIndent);
      printValue<T>(os, value, format, childIndent);
    }
    if (format) {
      os << std::endl;
    }
    printIndent(os, format, indent);
    os << ']';
  } else {
    T value = (reflection->*getMethod)(*message, field);
    printValue<T>(os, value, format, indent);
  }
  return os;
}

typedef const google::protobuf::Message& (google::protobuf::Reflection:: * MGM) (const google::protobuf::Message & message, const google::protobuf::FieldDescriptor* field, google::protobuf::MessageFactory* factory) const;
typedef const google::protobuf::Message& (google::protobuf::Reflection:: * MGRM) (const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field, int index) const;
template <>
inline std::ostream& fieldToJsonStream<const google::protobuf::Message&, MGM, MGRM>(std::ostream& os, bool format, int indent,
    const google::protobuf::Message* message,
    const google::protobuf::FieldDescriptor* field,
    const google::protobuf::Reflection* reflection,
    MGM getMethod,
    MGRM getRepeatedMethod
    ) {
  if (field->is_repeated()) {
    int size = reflection->FieldSize(*message, field);
    int childIndent = indent + 1;
    os << '[';
    for (int i = 0; i < size; ++i) {
      const google::protobuf::Message& value = (reflection->*getRepeatedMethod)(*message, field, i);
      if(i > 0) {
        os << ',';
      }
      if (format) {
        os << std::endl;
      }
      printIndent(os, format, childIndent);
      printValue<const google::protobuf::Message&>(os, value, format, childIndent);
    }
    if (format) {
      os << std::endl;
    }
    printIndent(os, format, indent);
    os << ']';
  } else {
    /// NOTE: following is the ONLY line which is different than common template function, because there is a optional parameter.
    const google::protobuf::Message& value = (reflection->*getMethod)(*message, field, NULL);
    printValue<const google::protobuf::Message&>(os, value, format, indent);
  }
  return os;
}

} // anonymous namespace

#if 1
#define FIELD_TO_JSON_STREAM(os, format, indent, message, field, reflection, TYPE, METHOD) \
    fieldToJsonStream<TYPE>(os, format, indent, message, field, reflection, &google::protobuf::Reflection::Get##METHOD, &google::protobuf::Reflection::GetRepeated##METHOD)

#else
// MACRO version of template function fieldToJsonStream
#define FIELD_TO_JSON_STREAM(os, format, indent, message, field, reflection, TYPE, METHOD) \
  do { \
    if (field->is_repeated()) { \
      int size = reflection->FieldSize(*message, field); \
      int childIndent = indent + 1; \
      os << '['; \
      for (int i = 0; i < size; ++i) { \
        TYPE value = reflection->GetRepeated##METHOD(*message, field, i); \
        if(i > 0) { \
          os << ','; \
        } \
        if (format) { \
          os << std::endl; \
        } \
        printIndent(os, format, childIndent); \
        printValue<TYPE>(os, value, format, childIndent); \
      } \
      if (format) { \
        os << std::endl; \
      } \
      printIndent(os, format, indent); \
      os << ']'; \
    } else { \
      TYPE value = reflection->Get##METHOD(*message, field); \
      printValue<TYPE>(os, value, format, indent); \
    } \
  } while (0)
#endif

std::ostream& ProtobufJson::toJsonStream(std::ostream& os, const google::protobuf::Message* message, bool format, int indent) {
  assert(message != NULL);

  const google::protobuf::Descriptor* descriptor = message->GetDescriptor();

  bool first = true;
  ++indent;
  os << '{';
  for (int i = 0; i < descriptor->field_count(); ++i) {
    const google::protobuf::FieldDescriptor* fieldDescriptor = descriptor->field(i);
    auto reflection = message->GetReflection();
    if ((fieldDescriptor->is_repeated() && reflection->FieldSize(*message, fieldDescriptor) <= 0)
        || (!fieldDescriptor->is_repeated() && !reflection->HasField(*message, fieldDescriptor))) {
      continue;
    }

    if(first) {
      first = false;
    } else {
      os << ',';
    }
    if (format) {
      os << std::endl;
    }

    printIndent(os, format, indent);
    printFieldName(os, fieldDescriptor->name(), format);

    switch (fieldDescriptor->cpp_type()) {
      case google::protobuf::FieldDescriptor::CPPTYPE_INT32: {
        FIELD_TO_JSON_STREAM(os, format, indent, message, fieldDescriptor, reflection, int32_t, Int32);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_INT64: {
        FIELD_TO_JSON_STREAM(os, format, indent, message, fieldDescriptor, reflection, int64_t, Int64);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_UINT32: {
        FIELD_TO_JSON_STREAM(os, format, indent, message, fieldDescriptor, reflection, uint32_t, UInt32);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_UINT64: {
        FIELD_TO_JSON_STREAM(os, format, indent, message, fieldDescriptor, reflection, uint64_t, UInt64);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE: {
        FIELD_TO_JSON_STREAM(os, format, indent, message, fieldDescriptor, reflection, double, Double);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT: {
        FIELD_TO_JSON_STREAM(os, format, indent, message, fieldDescriptor, reflection, float, Float);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_BOOL: {
        FIELD_TO_JSON_STREAM(os, format, indent, message, fieldDescriptor, reflection, bool, Bool);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
        FIELD_TO_JSON_STREAM(os, format, indent, message, fieldDescriptor, reflection, const google::protobuf::EnumValueDescriptor*, Enum);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
        FIELD_TO_JSON_STREAM(os, format, indent, message, fieldDescriptor, reflection, const std::string, String);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
        FIELD_TO_JSON_STREAM(os, format, indent, message, fieldDescriptor, reflection, const google::protobuf::Message&, Message);
        break;
      }
      default: {
        LOG(ERROR)<< "The type of protobuf is not supported";
        break;
      }
    } // switch
  } // for
  if (format) {
    os << std::endl;
  }
  printIndent(os, format, --indent);
  os << '}';

  return os;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                         Decode
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////


int ProtobufJson::parseJsonFromString(google::protobuf::Message* message, const std::string& json) {
  int resultCode = 0;
  if (!message) {
    resultCode = 1;
  } else {
    char err[255];
    // parse json std::string
    yajl_val root = yajl_tree_parse(json.c_str(), err, 255);

    // whether occur error
    if (strcmp(err, "") != 0) {
      DLOG(ERROR)<< "Parsing json error : " << err;
      resultCode = 1;
    } else {
      // set value of each field from yajl struct
      resultCode = jsonToField(message, root);
    }

    // free
    yajl_tree_free(root);
  }

  return resultCode;
}

int ProtobufJson::parseJsonFromFile(google::protobuf::Message* message, const std::string& filePath) {
  // open json file
  std::ifstream ifs;
  ifs.open(filePath.c_str(), std::ifstream::in);

  // whether file not found
  if (!ifs.is_open()) {
    return 1;
  }

  // get file length
  ifs.seekg(0, std::ios::end);
  int length = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  // read file content
  std::string buff;
  buff.resize(length);
  ifs.read(const_cast<char*>(buff.data()), length);
  ifs.close();

  // parse
  return parseJsonFromString(message, buff);
}

namespace {
template <typename Target, typename Source>
inline Target simple_lexical_cast(const Source &arg) {
  std::stringstream ss;
  ss << arg;
  Target result;
  ss >> result;
  return result;
}
} // anonymous namespace

// define set or add number value to field from node of json
#define JSON_TO_NUMBER_FIELD(NODE, TYPE, METHOD) \
  TYPE value = 0; \
  if (YAJL_IS_INTEGER(NODE)) { \
    value = (TYPE) YAJL_GET_INTEGER(NODE); \
  } else if (YAJL_IS_DOUBLE(NODE)) { \
    value = (TYPE) YAJL_GET_DOUBLE(NODE); \
  } else if (YAJL_IS_NUMBER(NODE)) { \
    value = simple_lexical_cast<TYPE, char*>(NODE->u.number.r); \
  } else if (YAJL_IS_STRING(NODE)) { \
    value = simple_lexical_cast<TYPE, char*>(NODE->u.string); \
  } else { \
    resultCode = 1; \
  } \
  if (resultCode != 0) { \
    LOG(ERROR) << "JSON_TO_NUMBER_FIELD: field (" << field->name() << ") type " <<  #TYPE << " does not match JSON type (" << NODE->type << ")"; \
    return resultCode; \
  } \
  reflection->METHOD(message, field, value);

// define set or add bool value to field from node of json
#define JSON_TO_BOOL_FIELD(NODE, TYPE, METHOD) \
  TYPE value = false; \
  if (YAJL_IS_STRING(NODE)) { \
    std::string nodeValue(NODE->u.string); \
    std::transform(nodeValue.begin(), nodeValue.end(), nodeValue.begin(), ::toupper); \
    if (nodeValue == "TRUE" || nodeValue == "YES" || nodeValue == "1") { \
      value = true; \
    } else { \
      value = false; \
    } \
  } else if (YAJL_IS_INTEGER(NODE)) { \
    value = NODE->u.number.i != 0; \
  } else if (YAJL_IS_DOUBLE(NODE)) { \
    value = NODE->u.number.d != 0; \
  } else if (YAJL_IS_TRUE(NODE)) { \
    value = true; \
  } else if (YAJL_IS_FALSE(NODE)) { \
    value = false; \
  } else { \
    resultCode = 1; \
  } \
  if (resultCode != 0) { \
    LOG(ERROR) << "JSON_TO_BOOL_FIELD: field (" << field->name() << ") type " <<  #TYPE << " does not match JSON type (" << NODE->type << ")"; \
    return resultCode; \
  } \
  reflection->METHOD(message, field, value);

// define set or add enum value to field from node of json
#define JSON_TO_ENUM_FIELD(NODE, TYPE, METHOD) TYPE value = NULL; \
  if (YAJL_IS_STRING(NODE)) { \
    value = field->enum_type()->FindValueByName(std::string(NODE->u.string)); \
  } else if (YAJL_IS_NUMBER(NODE)) { \
    value = field->enum_type()->FindValueByNumber(NODE->u.number.i); \
  } else { \
    resultCode = 1; \
  } \
  if (resultCode != 0) { \
    LOG(ERROR) << "JSON_TO_ENUM_FIELD: field (" << field->name() << ") type " <<  #TYPE << " does not match JSON type (" << NODE->type << ")"; \
    return resultCode; \
  } \
  reflection->METHOD(message, field, value);

// define set or add std::string value to field from node of json
#define JSON_TO_STRING_FIELD(NODE, TYPE, METHOD) TYPE value; \
  if (YAJL_IS_STRING(NODE)) { \
    value = NODE->u.string; \
  } else { \
    resultCode = 1; \
  } \
  if (resultCode != 0) { \
    LOG(ERROR) << "JSON_TO_STRING_FIELD: field (" << field->name() << ") type " <<  #TYPE << " does not match JSON type (" << NODE->type << ")"; \
    return resultCode; \
  } \
  reflection->METHOD(message, field, value);

// define set or add message value to field from node of json
#define JSON_TO_MESSAGE_FIELD(NODE, TYPE, METHOD) TYPE value = NULL; \
  value = reflection->METHOD(message, field); \
  if ((resultCode = jsonToField(value, NODE)) != 0) { \
    return resultCode; \
  }

// define set or add value to field from node of json
#define JSON_TO_FIELD(NODETYPE, TYPE, SETMETHOD, ADDMETHOD) \
  if (field->is_repeated()) { \
    for (size_t i = 0; i < fieldNode->u.array.len; ++ i) { \
      auto nd = fieldNode->u.array.values[i]; \
      JSON_TO_##NODETYPE##_FIELD(nd, TYPE, ADDMETHOD); \
    } \
  } else { \
    JSON_TO_##NODETYPE##_FIELD(fieldNode, TYPE, SETMETHOD); \
  }

int ProtobufJson::jsonToField(google::protobuf::Message* message, const yajl_val node) {
  if (!YAJL_IS_OBJECT(node)) {
    return 1;
  }

  int resultCode = 0;
  const google::protobuf::Descriptor* descriptor = message->GetDescriptor();
  const google::protobuf::Reflection* reflection = message->GetReflection();

  for (size_t i = 0; i < node->u.object.len; ++i) {
    yajl_val fieldNode = node->u.object.values[i];
    const google::protobuf::FieldDescriptor* field = descriptor->FindFieldByName(node->u.object.keys[i]);
    if (field == NULL) {
      LOG(ERROR) << "field " << node->u.object.keys[i] << " isn't found in message " << descriptor->full_name() << ".";
      return 1;
    }

    if ((YAJL_IS_ARRAY(fieldNode) != field->is_repeated())) {
      LOG(ERROR) << "field " << node->u.object.keys[i] << " does not matched from json and proto.";
      return 1;
    }

    switch (field->cpp_type()) {
      case google::protobuf::FieldDescriptor::CPPTYPE_INT32: {
        JSON_TO_FIELD(NUMBER, int32_t, SetInt32, AddInt32);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_INT64: {
        JSON_TO_FIELD(NUMBER, int64_t, SetInt64, AddInt64);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_UINT32: {
        JSON_TO_FIELD(NUMBER, uint32_t, SetUInt32, AddUInt32);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_UINT64: {
        JSON_TO_FIELD(NUMBER, uint64_t, SetUInt64, AddUInt64);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE: {
        JSON_TO_FIELD(NUMBER, double, SetDouble, AddDouble);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT: {
        JSON_TO_FIELD(NUMBER, float, SetFloat, AddFloat);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_BOOL: {
        JSON_TO_FIELD(BOOL, bool, SetBool, AddBool);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
        JSON_TO_FIELD(ENUM, const google::protobuf::EnumValueDescriptor*, SetEnum, AddEnum);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
        JSON_TO_FIELD(STRING, std::string, SetString, AddString);
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
        JSON_TO_FIELD(MESSAGE, google::protobuf::Message*, MutableMessage, AddMessage);
        break;
      }
      default: {
        LOG(ERROR)<< "The type of protobuf is not supported";
        resultCode = 1;
        break;
      }
    }
  }

  return 0;
}

} /* namespace idgs */
