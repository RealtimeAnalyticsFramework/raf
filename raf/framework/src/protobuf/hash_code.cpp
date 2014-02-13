
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "protobuf/hash_code.h"
#include "idgs/idgslogging.h"

using namespace google::protobuf;

namespace protobuf {

const hashcode_t PRIME = 99989;

// class HashCode
hashcode_t HashCode::hashcode(const Message* value) {
  if (!value) {
    return 0;
  }

  hashcode_t hashcode = 41;
  const Descriptor* desc = value->GetDescriptor();

  int i;
  int size = desc->field_count();
  for (i = 0; i < size; ++i) {
    hashcode = (hashcode * PRIME) + hashcodeByField(value, desc->field(i));
  }

  return hashcode;
}

#define HASH_ARRAY_FIELD(msg, field, method) \
    for (int i = 0; i < size; ++i) { \
      result = (result * PRIME) + stl_hashcode(reflect->GetRepeated ##method (* msg, field, i)); \
    } \
    return result;

hashcode_t HashCode::hashcodeByRepeatedField(const Message* msg, const FieldDescriptor* field) {
  hashcode_t result = 0;
  auto reflect = msg->GetReflection();
  int size = reflect->FieldSize(*msg, field);

  switch (field->cpp_type()) {
  case FieldDescriptor::CPPTYPE_UINT64:
    HASH_ARRAY_FIELD(msg, field, UInt64)
    ;
    break;
  case FieldDescriptor::CPPTYPE_INT64:
    HASH_ARRAY_FIELD(msg, field, Int64)
    ;
    break;
  case FieldDescriptor::CPPTYPE_UINT32:
    HASH_ARRAY_FIELD(msg, field, UInt32)
    ;
    break;
  case FieldDescriptor::CPPTYPE_INT32:
    HASH_ARRAY_FIELD(msg, field, Int32)
    ;
    break;
  case FieldDescriptor::CPPTYPE_STRING:
    HASH_ARRAY_FIELD(msg, field, String)
    ;
    break;
  case FieldDescriptor::CPPTYPE_DOUBLE:
    HASH_ARRAY_FIELD(msg, field, Double)
    ;
    break;
  case FieldDescriptor::CPPTYPE_FLOAT:
    HASH_ARRAY_FIELD(msg, field, Float)
    ;
    break;
  case FieldDescriptor::CPPTYPE_BOOL:
    HASH_ARRAY_FIELD(msg, field, Bool)
    ;
    break;
  case FieldDescriptor::CPPTYPE_ENUM:
    HASH_ARRAY_FIELD(msg, field, Enum)
    ;
    break;
  case FieldDescriptor::CPPTYPE_MESSAGE:
    HASH_ARRAY_FIELD(msg, field, Message)
    ;
    break;
  default:
    LOG(ERROR)<< "The type of protobuf is not supported";
    break;
  }

  return result;
}

hashcode_t HashCode::hashcodeByUnRepeatedField(const Message* msg, const FieldDescriptor* field) {
  auto reflect = msg->GetReflection();
  switch (field->cpp_type()) {
  case FieldDescriptor::CPPTYPE_UINT64:
    return stl_hashcode(reflect->GetUInt64(*msg, field));
  case FieldDescriptor::CPPTYPE_INT64:
    return stl_hashcode(reflect->GetInt64(*msg, field));
  case FieldDescriptor::CPPTYPE_UINT32:
    return stl_hashcode(reflect->GetUInt32(*msg, field));
  case FieldDescriptor::CPPTYPE_INT32:
    return stl_hashcode(reflect->GetInt32(*msg, field));
  case FieldDescriptor::CPPTYPE_STRING:
    return stl_hashcode(reflect->GetString(*msg, field));
  case FieldDescriptor::CPPTYPE_DOUBLE:
    return stl_hashcode(reflect->GetDouble(*msg, field));
  case FieldDescriptor::CPPTYPE_FLOAT:
    return stl_hashcode(reflect->GetFloat(*msg, field));
  case FieldDescriptor::CPPTYPE_BOOL:
    return stl_hashcode(reflect->GetBool(*msg, field));
  case FieldDescriptor::CPPTYPE_ENUM:
    return stl_hashcode(reflect->GetEnum(*msg, field)->number());
  case FieldDescriptor::CPPTYPE_MESSAGE:
    return stl_hashcode(reflect->GetMessage(*msg, field));
  default:
    LOG(ERROR)<< "The type of protobuf is not supported";
    return 0;
  }
}
// class HashCode

}// namespace idgs
