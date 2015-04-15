
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "protobuf/msg_comparer.h"

#include "idgs/idgslogging.h"

using namespace google::protobuf;

namespace protobuf {

bool less::operator()(const Message* msg1, const Message* msg2) const {
  return compare(msg1, msg2) < 0;
}

bool sharedless::operator()(const std::shared_ptr<google::protobuf::Message>& msg1,
    const std::shared_ptr<google::protobuf::Message>& msg2) const {
  return compare(msg1.get(), msg2.get()) < 0;
}

bool equals_to::operator()(const Message* msg1, const Message* msg2) const {
  return (compare(msg1, msg2) == 0);
}

orderless::orderless(const bool& isdesc) {
  fieldOrder.push_back(isdesc);
}

orderless::orderless(const std::vector<bool> isdesc) {
  fieldOrder = isdesc;
}

bool orderless::operator() (const std::shared_ptr<google::protobuf::Message> msg1, const std::shared_ptr<google::protobuf::Message> msg2) const {
  if (!msg1 && !msg2) {
    return 0;
  } else if (!msg1) {
    return -1;
  } else if (!msg2) {
    return 1;
  }

  const Descriptor * desc1 = msg1->GetDescriptor();
  const Descriptor * desc2 = msg2->GetDescriptor();

  if (desc1->field_count() != desc2->field_count()) {
    return false;
  }

  for (size_t i = 0; i < (size_t)desc1->field_count(); i++) {
    auto fld2 = desc2->field(i);
    if (desc1->field(i)->type() != fld2->type()) {
      return false;
    }
    int32_t cmp = compareFieldValue(msg1.get(), msg2.get(), desc1->field(i), fld2);
    if (cmp == 0) {
      continue;
    } else {
      bool isdesc = false;
      if (i < fieldOrder.size()) {
        isdesc = fieldOrder[i];
      }

      return ((isdesc ? -1 : 1) * cmp) < 0;
    }
  }

  return 0;
}

int32_t compare_to::operator()(const Message* msg1, const Message* msg2) const {
  return compare(msg1, msg2);
}

int32_t compare(const Message* msg1, const Message* msg2) {
  if (!msg1 && !msg2) {
    return 0;
  } else if (!msg1) {
    return -1;
  } else if (!msg2) {
    return 1;
  }

  const Descriptor * desc1 = msg1->GetDescriptor();
  const Descriptor * desc2 = msg2->GetDescriptor();

  if (desc1->field_count() != desc2->field_count()) {
    return 1;
  }

  for (int32_t i = 0; i < desc1->field_count(); i++) {
    auto fld2 = desc2->field(i);
    if (!fld2) {
      return 1;
    }
    int32_t cmp = compareFieldValue(msg1, msg2, desc1->field(i), fld2);
    if (cmp == 0) {
      continue;
    } else {
      return cmp;
    }
  }

  return 0;
}

int32_t compareUnRepeatedFieldValue(const Message * msg1, const Message * msg2, const FieldDescriptor * field1, const FieldDescriptor * field2) {
  if (field1->cpp_type() != field2->cpp_type()) {
    if (field1->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE || field2->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      return 1;
    }

    PbVariant lvar = getFieldValue(msg1, field1);
    PbVariant rvar = getFieldValue(msg2, field2);

    if (lvar == rvar) {
      return 0;
    } else if (lvar < rvar) {
      return -1;
    } else {
      return 1;
    }
  }

  const Reflection* ref1 = msg1->GetReflection();
  const Reflection* ref2 = msg2->GetReflection();
  switch (field1->cpp_type()) {
    case FieldDescriptor::CPPTYPE_UINT64:
      return ref1->GetUInt64(*msg1, field1) - ref2->GetUInt64(*msg2, field2);
    case FieldDescriptor::CPPTYPE_INT64:
      return ref1->GetInt64(*msg1, field1) - ref2->GetInt64(*msg2, field2);
    case FieldDescriptor::CPPTYPE_UINT32:
      return ref1->GetUInt32(*msg1, field1) - ref2->GetUInt32(*msg2, field2);
    case FieldDescriptor::CPPTYPE_INT32:
      return ref1->GetInt32(*msg1, field1) - ref2->GetInt32(*msg2, field2);
    case FieldDescriptor::CPPTYPE_STRING:
      return ref1->GetString(*msg1, field1).compare(ref2->GetString(*msg2, field2));
    case FieldDescriptor::CPPTYPE_DOUBLE: {
      double lhs = ref1->GetDouble(*msg1, field1);
      double rhs = ref2->GetDouble(*msg2, field2);
      /// @todo float or double should not be compared directly. correct one should like: abs(lhs - rhs) < 0.0000001
      return (lhs == rhs) ? 0 : ((lhs > rhs) ? 2 : -2);
    }
    case FieldDescriptor::CPPTYPE_FLOAT: {
      float lhs = ref1->GetFloat(*msg1, field1);
      float rhs = ref2->GetFloat(*msg2, field2);
      /// @todo float or double should not be compared directly. correct one should like: abs(lhs - rhs) < 0.0000001
      return (lhs == rhs) ? 0 : ((lhs > rhs) ? 2 : -2);
    }
    case FieldDescriptor::CPPTYPE_BOOL:
      return ref1->GetBool(*msg1, field1) - ref2->GetBool(*msg2, field2);
    case FieldDescriptor::CPPTYPE_ENUM:
      return ref1->GetEnum(*msg1, field1)->number() - ref2->GetEnum(*msg2, field2)->number();
    case FieldDescriptor::CPPTYPE_MESSAGE:
      return compare(&ref1->GetMessage(*msg1, field1), &ref2->GetMessage(*msg2, field2));
    default:
      LOG(ERROR)<< "The type of protobuf is not supported";
      return 0;
  }
}

#define COMPARE_ARRAY_FIELD(msg1, msg2, field1, field2, method) \
    for (int i = 0; i < size && result == 0; ++ i) { \
      result = (ref1->GetRepeated ##method(* msg1, field1, i)) - (ref2->GetRepeated ##method(* msg2, field2, i)); \
    }

int32_t compareRepeatedFieldValue(const Message* msg1, const Message* msg2, const FieldDescriptor* field1, const FieldDescriptor * field2) {
  int32_t result = 0;
  const Reflection* ref1 = msg1->GetReflection();
  const Reflection* ref2 = msg2->GetReflection();
  int size1 = ref1->FieldSize(* msg1, field1);
  int size2 = ref2->FieldSize(* msg2, field2);
  int size = std::min(size1, size2);

  if (field1->cpp_type() != field2->cpp_type()) {
    if (field1->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE || field2->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      return 1;
    }

    for (int32_t i = 0; i < size && result == 0; ++ i) {
      PbVariant lvar = getFieldValue(msg1, field1, i);
      PbVariant rvar = getFieldValue(msg2, field2, i);

      if (lvar == rvar) {
        result = 0;
      } else if (lvar < rvar) {
        result = -1;
      } else {
        result = 1;
      }
    }
  } else {
    switch (field1->cpp_type()) {
    case FieldDescriptor::CPPTYPE_UINT64:
      COMPARE_ARRAY_FIELD(msg1, msg2, field1, field2, UInt64);
      break;
    case FieldDescriptor::CPPTYPE_INT64:
      COMPARE_ARRAY_FIELD(msg1, msg2, field1, field2, Int64);
      break;
    case FieldDescriptor::CPPTYPE_UINT32:
      COMPARE_ARRAY_FIELD(msg1, msg2, field1, field2, UInt32);
      break;
    case FieldDescriptor::CPPTYPE_INT32:
      COMPARE_ARRAY_FIELD(msg1, msg2, field1, field2, Int32);
      break;
    case FieldDescriptor::CPPTYPE_STRING:
      for (int i = 0; i < size && result == 0; ++i) {
        result = ref1->GetRepeatedString(*msg1, field1, i).compare(ref2->GetRepeatedString(*msg2, field2, i));
      }
      break;
    case FieldDescriptor::CPPTYPE_DOUBLE:
      for (int i = 0; i < size && result == 0; ++i) {
        auto v1 = ref1->GetRepeatedDouble(*msg1, field1, i);
        auto v2 = ref2->GetRepeatedDouble(*msg2, field2, i);
        /// @todo compare double
        if (v1 < v2) {
          result = -1;
        } else if (v1 > v2) {
          result = 1;
        }
      }
      break;
    case FieldDescriptor::CPPTYPE_FLOAT:
      for (int i = 0; i < size && result == 0; ++i) {
        auto v1 = ref1->GetRepeatedFloat(*msg1, field1, i);
        auto v2 = ref2->GetRepeatedFloat(*msg2, field2, i);
        /// @todo compare double
        if (v1 < v2) {
          result = -1;
        } else if (v1 > v2) {
          result = 1;
        }
      }
      break;
    case FieldDescriptor::CPPTYPE_BOOL:
      COMPARE_ARRAY_FIELD(msg1, msg2, field1, field2, Bool);
      break;
    case FieldDescriptor::CPPTYPE_ENUM:
      COMPARE_ARRAY_FIELD(msg1, msg2, field1, field2, Enum);
      break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
      for (int i = 0; i < size && result == 0; ++ i) {
        result = compare(&ref1->GetMessage(*msg1, field1), &ref2->GetMessage(*msg2, field2));
      }
      break;
    default:
      LOG(ERROR)<< "The type of protobuf is not supported";
      result = 0;
      break;
    }
  }

  if (result == 0) {
    if (size1 < size2) {
      return -1;
    } else if (size1 > size2) {
      return 1;
    }
  }

  return result;
}

PbVariant getFieldValue(const google::protobuf::Message* msg, const google::protobuf::FieldDescriptor* field, const int64_t& index) {
  switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_UINT64:
      return PbVariant((field->is_repeated()) ? msg->GetReflection()->GetRepeatedUInt64(*msg, field, index) : msg->GetReflection()->GetUInt64(*msg, field));
    case FieldDescriptor::CPPTYPE_INT64:
      return PbVariant((field->is_repeated()) ? msg->GetReflection()->GetRepeatedInt64(*msg, field, index) : msg->GetReflection()->GetInt64(*msg, field));
    case FieldDescriptor::CPPTYPE_UINT32:
      return PbVariant((field->is_repeated()) ? msg->GetReflection()->GetRepeatedUInt32(*msg, field, index) : msg->GetReflection()->GetUInt32(*msg, field));
    case FieldDescriptor::CPPTYPE_INT32:
      return PbVariant((field->is_repeated()) ? msg->GetReflection()->GetRepeatedInt32(*msg, field, index) : msg->GetReflection()->GetInt32(*msg, field));
    case FieldDescriptor::CPPTYPE_STRING:
      return PbVariant((field->is_repeated()) ? msg->GetReflection()->GetRepeatedString(*msg, field, index) : msg->GetReflection()->GetString(*msg, field));
    case FieldDescriptor::CPPTYPE_DOUBLE:
      return PbVariant((field->is_repeated()) ? msg->GetReflection()->GetRepeatedDouble(*msg, field, index) : msg->GetReflection()->GetDouble(*msg, field));
    case FieldDescriptor::CPPTYPE_FLOAT:
      return PbVariant((field->is_repeated()) ? msg->GetReflection()->GetRepeatedFloat(*msg, field, index) : msg->GetReflection()->GetFloat(*msg, field));
    case FieldDescriptor::CPPTYPE_BOOL:
      return PbVariant((field->is_repeated()) ? msg->GetReflection()->GetRepeatedBool(*msg, field, index) : msg->GetReflection()->GetBool(*msg, field));
    case FieldDescriptor::CPPTYPE_ENUM:
      return PbVariant((field->is_repeated()) ? msg->GetReflection()->GetRepeatedEnum(*msg, field, index)->number() : msg->GetReflection()->GetEnum(*msg, field)->number());
    case FieldDescriptor::CPPTYPE_MESSAGE:
      return PbVariant();
    default:
      LOG(ERROR)<< "The type of protobuf is not supported";
      return PbVariant();
  }
}

} // namespace idgs
