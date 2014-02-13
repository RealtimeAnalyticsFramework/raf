
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "gtest/gtest.h"

#include "protobuf/pb_serdes.h"
#include "employee.pb.cc"


using namespace protobuf;
using namespace google::protobuf;
using namespace idgs::framework::test::pb;

TEST(pb_serdes, unknown) {
  string json;
  Employee emp;
  try {
    ProtoSerdes<100>::deserialize(json, &emp);
  } catch (std::exception& e) {
    LOG(INFO) << e.what();
  }
  try {
    ProtoSerdes<100>::serialize(&emp, &json);
  } catch (std::exception& e) {
    LOG(INFO) << e.what();
  }

}


TEST(pb_serdes, json) {
  string json;
  json.append("{\"id\":\"234000\",\"name\":\"Tom\",\"age\":\"14\",\"gender\":\"male\",");
  json.append("\"role\":[\"role1\",\"role2\",\"role3\"],");
  json.append("\"dept\":{\"id\":\"19999\",\"name\":\"12345\"},");
  json.append("\"edu\":[{\"when\":\"2000-2004\",\"school\":\"ncut\"},");
  json.append("{\"when\":\"2004-2008\",\"school\":\"bj\"}]}");
  DVLOG(2) << "json string : " << json;

  Employee emp;
  ProtoSerdes<PB_JSON>::deserialize(json, &emp);

  string s2;
  ProtoSerdes<PB_JSON>::serialize(&emp, &s2);

  Employee emp2;
  ProtoSerdes<PB_JSON>::deserialize(s2, &emp2);

  string s3;
  ProtoSerdes<PB_JSON>::serialize(&emp2, &s3);

  EXPECT_EQ(s2, s3);
  LOG(INFO) << s2;
}

TEST(pb_serdes, binary) {
  string json;
  json.append("{\"id\":\"234000\",\"name\":\"Tom\",\"age\":\"14\",\"gender\":\"male\",");
  json.append("\"role\":[\"role1\",\"role2\",\"role3\"],");
  json.append("\"dept\":{\"id\":\"19999\",\"name\":\"12345\"},");
  json.append("\"edu\":[{\"when\":\"2000-2004\",\"school\":\"ncut\"},");
  json.append("{\"when\":\"2004-2008\",\"school\":\"bj\"}]}");
  DVLOG(2) << "json string : " << json;

  Employee emp;
  ProtoSerdes<PB_JSON>::deserialize(json, &emp);

  string s2;
  ProtoSerdes<PB_BINARY>::serialize(&emp, &s2);

  Employee emp2;
  ProtoSerdes<PB_BINARY>::deserialize(s2, &emp2);

  string s3;
  ProtoSerdes<PB_BINARY>::serialize(&emp2, &s3);

  EXPECT_EQ(s2, s3);
  LOG(INFO) << s2;
}

TEST(pb_serdes, text) {
  string json;
  json.append("{\"id\":\"234000\",\"name\":\"Tom\",\"age\":\"14\",\"gender\":\"male\",");
  json.append("\"role\":[\"role1\",\"role2\",\"role3\"],");
  json.append("\"dept\":{\"id\":\"19999\",\"name\":\"12345\"},");
  json.append("\"edu\":[{\"when\":\"2000-2004\",\"school\":\"ncut\"},");
  json.append("{\"when\":\"2004-2008\",\"school\":\"bj\"}]}");
  DVLOG(2) << "json string : " << json;

  Employee emp;
  ProtoSerdes<PB_JSON>::deserialize(json, &emp);

  string s2;
  ProtoSerdes<PB_TEXT>::serialize(&emp, &s2);

  Employee emp2;
  ProtoSerdes<PB_TEXT>::deserialize(s2, &emp2);

  string s3;
  ProtoSerdes<PB_TEXT>::serialize(&emp2, &s3);

  EXPECT_EQ(s2, s3);
  LOG(INFO) << s2;
}

