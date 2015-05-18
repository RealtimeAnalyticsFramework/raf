
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <memory>
#include "protobuf/hash_code.h"
#include "protobuf/pbvariant.h"

namespace protobuf {

/// A comparison functors. <br>
/// Compare two protobuf messages with message1 less then message2. <br>
struct less {
public:
  /// @brief  Compare two protobuf messages.
  /// @param  msg1 To be compared message1.
  /// @param  msg2 To be compared message2.
  /// @return Whether msg1 less the msg2.
  bool operator()(const google::protobuf::Message* msg1, const google::protobuf::Message* msg2) const;
};
// struct less

struct sharedless {
public:
  /// @brief  Compare two protobuf messages.
  /// @param  msg1 To be compared message1.
  /// @param  msg2 To be compared message2.
  /// @return Whether msg1 less the msg2.
  bool operator()(const std::shared_ptr<google::protobuf::Message>& msg1,
      const std::shared_ptr<google::protobuf::Message>& msg2) const;
};
// struct less

/// A comparison functors. <br>
/// Compare two protobuf messages with message1 equals to message2. <br>
struct equals_to {
public:
  /// @brief  Compare two protobuf messages.
  /// @param  msg1 To be compared message1.
  /// @param  msg2 To be compared message2.
  /// @return Whether msg1 equals to msg2.
  bool operator()(const google::protobuf::Message* msg1, const google::protobuf::Message* msg2) const;
};
// struct equals_to

/// A comparison functors. <br>
/// Compare two protobuf messages. <br>
struct compare_to {
public:
  /// @brief  Compare two protobuf messages.
  /// @param  msg1 To be compared message1.
  /// @param  msg2 To be compared message2.
  /// @return If msg1 less then msg2, return value < 0.
  ///         If msg1 equals to msg2, return value = 0.
  ///         If msg1 greater then msg2, return value > 0.
  int32_t operator()(const google::protobuf::Message* msg1, const google::protobuf::Message* msg2) const;
};
// struct compare_to

/// A comparison functors. <br>
/// Calculate the hash code of a protobuf message. <br>
struct hash_code {
public:
  /// @brief  Calculate the hash code of the given protobuf message.
  /// @param  msg To calculate message.
  /// @return The hash code.
  size_t operator()(const google::protobuf::Message* msg) const {
    return HashCode::hashcode(msg);
  }
};
// struct hash_code

/// A comparison functors. <br>
/// Calculate the hash code of a protobuf message. <br>
struct shared_hash_code {
public:
  /// @brief  Calculate the hash code of the given protobuf message.
  /// @param  msg To calculate message.
  /// @return The hash code.
  size_t operator()(const std::shared_ptr<google::protobuf::Message> msg) const {
    return HashCode::hashcode(msg.get());
  }
};
// struct hash_code

struct orderless {
public:

  orderless(const bool& isdesc = false);
  orderless(const std::vector<bool> isdesc);
  /// @brief  Compare two protobuf messages.
  /// @param  msg1 To be compared message1.
  /// @param  msg2 To be compared message2.
  /// @return Whether msg1 less the msg2.
  bool operator()(const std::shared_ptr<google::protobuf::Message> msg1, const std::shared_ptr<google::protobuf::Message> msg2) const;

private:
  std::vector<bool> fieldOrder;
};

/// @brief  Inline method, Compare two protobuf messages.
/// @param  msg1 To be compared message1.
/// @param  msg2 To be compared message2.
/// @return If msg1 less then msg2, return value < 0.
///         If msg1 equals to msg2, return value = 0.
///         If msg1 greater then msg2, return value > 0.
int32_t compare(const google::protobuf::Message* msg1, const google::protobuf::Message* msg2);

/// @brief  Inline method, Compare repeated field of two protobuf messages.
/// @param  msg1 To be compared message1.
/// @param  msg2 To be compared message2.
/// @param  field To be compared field of the two message.
/// @return If msg1 less then msg2, return value < 0.
///         If msg1 equals to msg2, return value = 0.
///         If msg1 greater then msg2, return value > 0.
int32_t compareRepeatedFieldValue(const google::protobuf::Message* msg1, const google::protobuf::Message* msg2,
    const google::protobuf::FieldDescriptor* field1, const google::protobuf::FieldDescriptor* field2);

/// @brief  Inline method, Compare unrepeated field of two protobuf messages.
/// @param  msg1 To be compared message1.
/// @param  msg2 To be compared message2.
/// @param  field To be compared field of the two message.
/// @return If msg1 less then msg2, return value < 0.
///         If msg1 equals to msg2, return value = 0.
///         If msg1 greater then msg2, return value > 0.
int32_t compareUnRepeatedFieldValue(const google::protobuf::Message* msg1, const google::protobuf::Message* msg2,
    const google::protobuf::FieldDescriptor* field1, const google::protobuf::FieldDescriptor* field2);

/// @brief  Inline method, Compare each field of two protobuf messages.
/// @param  msg1 To be compared message1.
/// @param  msg2 To be compared message2.
/// @param  field To be compared field of the two message.
/// @return If msg1 less then msg2, return value < 0.
///         If msg1 equals to msg2, return value = 0.
///         If msg1 greater then msg2, return value > 0.
inline int32_t compareFieldValue(const google::protobuf::Message* msg1, const google::protobuf::Message* msg2,
    const google::protobuf::FieldDescriptor* field1, const google::protobuf::FieldDescriptor* field2) {
  if (field1->is_repeated() && field2->is_repeated()) {
    return compareRepeatedFieldValue(msg1, msg2, field1, field2);
  } else if (!field1->is_repeated() && field2->is_repeated()) {
    return -1;
  } else if (field1->is_repeated() && !field2->is_repeated()) {
    return 1;
  } else {
    return compareUnRepeatedFieldValue(msg1, msg2, field1, field2);
  }
}

PbVariant getFieldValue(const google::protobuf::Message* msg, const google::protobuf::FieldDescriptor* field, const int64_t& index = 0);

} // namespace protobuf;
