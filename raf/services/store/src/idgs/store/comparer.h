
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/store/store_ptr.h"
#include "protobuf/msg_comparer.h"

namespace idgs {
namespace store {

/// A comparison functors. <br>
/// Compare two unique point of protobuf messages with message1 less then message2. <br>
struct less {
public:
  /// @brief  Compare two unique point of protobuf messages.
  /// @param  msg1 To be compared protobuf message wrapper.
  /// @param  msg2 To be compared protobuf message wrapper.
  /// @return Whether msg1 less the msg2.
  bool operator()(const StoreKey<google::protobuf::Message>& msg1,
      const StoreKey<google::protobuf::Message>& msg2) const;

};
// struct less

/// A comparison functors. <br>
/// Compare two unique point of protobuf messages with message1 equals to message2. <br>
struct equals_to {
public:
  /// @brief  Compare two unique point of protobuf messages.
  /// @param  msg1 To be compared protobuf message wrapper.
  /// @param  msg2 To be compared protobuf message wrapper.
  /// @return Whether msg1 equals to msg2.
  bool operator()(const StoreKey<google::protobuf::Message>& msg1,
      const StoreKey<google::protobuf::Message>& msg2) const;

};
// struct equals_to

/// A comparison functors. <br>
/// Compare two unique point of protobuf messages. <br>
struct compare_to {
public:
  /// @brief  Compare two unique point of protobuf messages.
  /// @param  msg1 To be compared protobuf message wrapper.
  /// @param  msg2 To be compared protobuf message wrapper.
  /// @return If msg1 less then msg2, return value < 0.
  ///         If msg1 equals to msg2, return value = 0.
  ///         If msg1 greater then msg2, return value > 0.
  int32_t operator()(const StoreKey<google::protobuf::Message>& msg1,
      const StoreKey<google::protobuf::Message>& msg2) const;

};
// struct compare_to

/// A comparison functors. <br>
/// Calculate the hash code of a unique point of protobuf message. <br>
struct hash_code {
public:
  /// @brief  Calculate the hash code of the given unique point of protobuf message.
  /// @param  msg To be calculated protobuf message wrapper.
  /// @return The hash code.
  size_t operator()(const StoreKey<google::protobuf::Message>& msg) const {
    return op(msg.get());
  }
private:
  protobuf::hash_code op;

};
// struct hash_code

inline bool less::operator()(const StoreKey<google::protobuf::Message>& msg1,
    const StoreKey<google::protobuf::Message>& msg2) const {
  static protobuf::less less;
  return less.operator ()(msg1.get(), msg2.get());
}

inline bool equals_to::operator()(const StoreKey<google::protobuf::Message>& msg1,
    const StoreKey<google::protobuf::Message>& msg2) const {
  static protobuf::equals_to eq;
  return eq.operator ()(msg1.get(), msg2.get());
}

inline int32_t compare_to::operator()(const StoreKey<google::protobuf::Message>& msg1,
    const StoreKey<google::protobuf::Message>& msg2) const {
  static protobuf::compare_to compare;
  return compare.operator ()(msg1.get(), msg2.get());
}
} //namespace store
} // namespace idgs
