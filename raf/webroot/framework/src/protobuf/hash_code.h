
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <google/protobuf/message.h>

typedef size_t hashcode_t;

namespace protobuf {

/// Calculate hash code. <br>
class HashCode {
public:

  /// @brief  Hash code of protobuf Message value.
  /// @param  value protobuf Message value.
  /// @return Hash code.
  /// @code
  /// // Example
  /// Employee * emp = new Employee; // Employee is protobuf type
  /// // set properties.
  /// int32_t hashcode = HashCode::hashcode(value);
  /// delete emp;
  /// @endcode
  static hashcode_t hashcode(const google::protobuf::Message* value);

  inline static hashcode_t hashcode(const google::protobuf::Message& value) {
    return hashcode(&value);
  }

private:
  static hashcode_t hashcodeByField(const google::protobuf::Message* msg,
      const google::protobuf::FieldDescriptor* field);
  static hashcode_t hashcodeByRepeatedField(const google::protobuf::Message* msg,
      const google::protobuf::FieldDescriptor* field);
  static hashcode_t hashcodeByUnRepeatedField(const google::protobuf::Message* msg,
      const google::protobuf::FieldDescriptor* field);

};
// class HashCode

// private
inline hashcode_t HashCode::hashcodeByField(const google::protobuf::Message* msg,
    const google::protobuf::FieldDescriptor* field) {
  if (field->is_repeated()) {
    return hashcodeByRepeatedField(msg, field);
  } else {
    return hashcodeByUnRepeatedField(msg, field);
  }
}

} // namespace protobuf

template<typename T>
inline hashcode_t stl_hashcode(const T& v) {
  static std::hash<T> h;
  return h(v);
}

template<>
inline hashcode_t stl_hashcode<google::protobuf::Message>(const google::protobuf::Message& v) {
  return protobuf::HashCode::hashcode(v);
}

typedef google::protobuf::Message* GoogleProtobufMessagePtr;
template<>
inline hashcode_t stl_hashcode<GoogleProtobufMessagePtr>(const GoogleProtobufMessagePtr& v) {
  return protobuf::HashCode::hashcode(v);
}

