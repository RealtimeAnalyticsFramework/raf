
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/

#include "gtest/gtest.h"
#include "protobuf/message_helper.h"

/// Case:
/// Message 'Person' has nested repeated Message 'Pet'
TEST(pb_dynamic_test, createDynamicMsg) {
  using namespace ::google::protobuf;

  FileDescriptorProto file_proto;
  file_proto.set_name("dynamic.proto");

  const std::string& msg_name = "Person";
  const std::string& nested_msg_name = "Pet";

  DescriptorProto *message_proto = file_proto.add_message_type();
  message_proto->set_name(nested_msg_name);
  auto field = message_proto->add_field();
  field->set_name("type");
  field->set_label(FieldDescriptorProto_Label_LABEL_REQUIRED);
  field->set_type(FieldDescriptorProto_Type_TYPE_STRING);
  field->set_number(1);

  message_proto = file_proto.add_message_type();
  message_proto->set_name(msg_name);
  int32_t i = 0;
  field = message_proto->add_field();
  field->set_name("name");
  field->set_label(FieldDescriptorProto_Label_LABEL_REQUIRED);
  field->set_type(FieldDescriptorProto_Type_TYPE_STRING);
  field->set_number(++i);

  field = message_proto->add_field();
  field->set_name("gender");
  field->set_label(FieldDescriptorProto_Label_LABEL_OPTIONAL);
  field->set_type(FieldDescriptorProto_Type_TYPE_STRING);
  field->set_number(++i);

  field = message_proto->add_field();
  field->set_name("age");
  field->set_label(FieldDescriptorProto_Label_LABEL_OPTIONAL);
  field->set_type(FieldDescriptorProto_Type_TYPE_INT32);
  field->set_number(++i);

  // nested type
  field = message_proto->add_field();
  field->set_name("pet");
  field->set_label(FieldDescriptorProto_Label_LABEL_REPEATED);
  field->set_type(FieldDescriptorProto_Type_TYPE_MESSAGE);
  field->set_type_name(nested_msg_name);
  field->set_number(++i);

  protobuf::MessageHelper helper;
  // register file
  helper.registerDynamicMessage(file_proto);

  auto pet1 = helper.createMessage(nested_msg_name);
  auto ref = pet1->GetReflection();
  ref->SetString(pet1.get(), pet1->GetDescriptor()->FindFieldByName("type"), "cat");
  ASSERT_EQ("cat", ref->GetString(*pet1, pet1->GetDescriptor()->FindFieldByName("type")));
  DVLOG(0) << pet1->DebugString();

  auto pet2 = helper.createMessage(nested_msg_name);
  ref = pet2->GetReflection();
  ref->SetString(pet2.get(), pet2->GetDescriptor()->FindFieldByName("type"), "dog");
  ASSERT_EQ("dog", ref->GetString(*pet2, pet2->GetDescriptor()->FindFieldByName("type")));
  DVLOG(0) << pet2->DebugString();

  // new a registered message
  auto person1 = helper.createMessage(msg_name);
  DVLOG(0) << person1;
  ref = person1->GetReflection();
  ref->SetString(person1.get(), person1->GetDescriptor()->FindFieldByName("name"), "tom");
  ref->SetString(person1.get(), person1->GetDescriptor()->FindFieldByName("gender"), "male");
  ref->SetInt32(person1.get(), person1->GetDescriptor()->FindFieldByName("age"), 20);
  auto nested_msg = ref->AddMessage(person1.get(), person1->GetDescriptor()->FindFieldByName("pet"));
  nested_msg->CopyFrom(*pet1);

  ASSERT_EQ("tom", ref->GetString(*person1, person1->GetDescriptor()->FindFieldByName("name")));
  ASSERT_EQ("male", ref->GetString(*person1, person1->GetDescriptor()->FindFieldByName("gender")));
  ASSERT_EQ(20, ref->GetInt32(*person1, person1->GetDescriptor()->FindFieldByName("age")));
  DVLOG(0) << person1->DebugString();

  auto person2 = helper.createMessage(msg_name);
  ref = person2->GetReflection();
  ref->SetString(person2.get(), person2->GetDescriptor()->FindFieldByName("name"), "jerry");
  ref->SetString(person2.get(), person2->GetDescriptor()->FindFieldByName("gender"), "female");
  ref->SetInt32(person2.get(), person2->GetDescriptor()->FindFieldByName("age"), 10);
  nested_msg = ref->AddMessage(person2.get(), person2->GetDescriptor()->FindFieldByName("pet"));
  nested_msg->CopyFrom(*pet1);
  nested_msg = ref->AddMessage(person2.get(), person2->GetDescriptor()->FindFieldByName("pet"));
  nested_msg->CopyFrom(*pet2);

  ASSERT_EQ("jerry", ref->GetString(*person2, person2->GetDescriptor()->FindFieldByName("name")));
  ASSERT_EQ("female", ref->GetString(*person2, person2->GetDescriptor()->FindFieldByName("gender")));
  ASSERT_EQ(10, ref->GetInt32(*person2, person2->GetDescriptor()->FindFieldByName("age")));
  DVLOG(0) << person2->DebugString();

  ASSERT_TRUE(true);
}

