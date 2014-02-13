/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include <gtest/gtest.h>
#include "protobuf/message_helper.h"
#include "protobuf/pb_serdes.h"

using namespace protobuf;
using namespace google::protobuf;

namespace idgs {
namespace pair_serde_test {
  ///serde by string
  void check_serde_str(const Message* src, Message* dest) {
    std::string serde_str;
    ProtoSerdes<PB_BINARY>::serialize(src, &serde_str);
    DVLOG(5) << dumpBinaryBuffer2(serde_str.c_str(), serde_str.length());
    ProtoSerdes<PB_BINARY>::deserialize(serde_str, dest);
    if(src->DebugString().compare(dest->DebugString())) {
      LOG(FATAL) << "\nexpect: " << src->DebugString() << "\nactual: " << dest->DebugString();
    }
    DVLOG(5) << dest->DebugString();
  }
  /// serde by array
  void check_serde_array(const Message* src, Message* dest) {
    size_t size = src->ByteSize();
    char* array = new char[size];
    ProtoSerdes<PB_BINARY>::serialize(src, array, size);
    DVLOG(5) << dumpBinaryBuffer2(array, size);
    ProtoSerdes<PB_BINARY>::deserializeFromArray(array, size, dest);
    delete array;
    if(src->DebugString().compare(dest->DebugString())) {
      LOG(FATAL) << "\nexpect: " << src->DebugString() << "\nactual: " << dest->DebugString();
    }
    DVLOG(5) << dest->DebugString();
  }
  /// ser to string, de by array
  void check_serstr_dearray(const Message* src, Message* dest) {
    std::string serde_str;
    ProtoSerdes<PB_BINARY>::serialize(src, &serde_str);
    DVLOG(5) << dumpBinaryBuffer2(serde_str.c_str(), serde_str.length());
    ProtoSerdes<PB_BINARY>::deserializeFromArray(serde_str.c_str(), serde_str.length(), dest);
    if(src->DebugString().compare(dest->DebugString())) {
      LOG(FATAL) << "\nexpect: " << src->DebugString() << "\nactual: " << dest->DebugString();
    }
    DVLOG(5) << dest->DebugString();
  }
  /// ser to array, de by string
  void check_serarray_destr(const Message* src, Message* dest) {
    size_t size = src->ByteSize();
    char* array = new char[size];
    ProtoSerdes<PB_BINARY>::serialize(src, array, size);
    DVLOG(5) << dumpBinaryBuffer2(array, size);
    ProtoSerdes<PB_BINARY>::deserialize(std::string(array, size), dest);
    delete array;
    if(src->DebugString().compare(dest->DebugString())) {
      LOG(FATAL) << "\nexpect: " << src->DebugString() << "\nactual: " << dest->DebugString();
    }
    DVLOG(0) << dest->DebugString();
  }

  void test_serde() {
    // dynamic create message.
    MessageHelper helper;
    FileDescriptorProto file_proto;
    file_proto.set_name("test.proo");
    DescriptorProto *message_proto = file_proto.add_message_type();
    message_proto->set_name("Pair");
    auto field = message_proto->add_field();
    field->set_name("key");
    field->set_label(FieldDescriptorProto_Label_LABEL_REQUIRED);
    field->set_type(FieldDescriptorProto_Type_TYPE_STRING);
    field->set_number(1);

    field = message_proto->add_field();
    field->set_name("value");
    field->set_label(FieldDescriptorProto_Label_LABEL_REQUIRED);
    field->set_type(FieldDescriptorProto_Type_TYPE_BYTES);
    field->set_number(2);
    helper.registerDynamicMessage(file_proto);
    auto src = helper.createMessage("Pair");
    auto ref = src->GetReflection();
    string key("scott");
    string value("tiger");
    ref->SetString(src.get(), src->GetDescriptor()->FindFieldByName("key"), key);
    ref->SetString(src.get(), src->GetDescriptor()->FindFieldByName("value"), value);

    // test serde
    check_serde_str(src.get(), helper.createMessage("Pair").get());
    check_serde_array(src.get(), helper.createMessage("Pair").get());
    check_serstr_dearray(src.get(), helper.createMessage("Pair").get());
    check_serarray_destr(src.get(), helper.createMessage("Pair").get());
  }
}
}

TEST(pair_serde_test, string_bytes_serde) {
  using namespace idgs::pair_serde_test;
  test_serde();
}
