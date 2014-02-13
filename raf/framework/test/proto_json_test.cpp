
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include <gtest/gtest.h>
#include "employee.pb.cc"
#include "protobuf/json_message.h"

using namespace protobuf;
using namespace idgs;
using namespace google::protobuf;
using namespace idgs::framework::test::pb;

TEST(json_proto, from_string) {
  string json;
  json.append("{\"id\":234000,\"name\":\"Tom\",\"age\":14,\"gender\":\"male\",");
  json.append("\"role\":[\"role1\",\"role2\",\"role3\"],");
  json.append("\"dept\":{\"id\":19999,\"name\":\"12345\"},");
  json.append("\"edu\":[{\"when\":\"2000-2004\",\"school\":\"ncut\"},");
  json.append("{\"when\":\"2004-2008\",\"school\":\"bj\"}]}");
  DVLOG(2) << "json string : " << json;

  Employee emp;
  ResultCode status = JsonMessage().parseJsonFromString(&emp, json);
  if (status != RC_SUCCESS) {
    DVLOG(2) << "Parse json error. Caused by " << getErrorDescription(status);
    return;
  }

  ASSERT_EQ(234000, emp.id());
  ASSERT_EQ("Tom", emp.name());
  ASSERT_EQ(14, emp.age());
  ASSERT_EQ("male", emp.gender());

  ASSERT_EQ("role1", emp.role(0));
  ASSERT_EQ("role2", emp.role(1));
  ASSERT_EQ("role3", emp.role(2));

  ASSERT_EQ(19999, emp.dept().id());
  ASSERT_EQ("12345", emp.dept().name());

  ASSERT_EQ("2000-2004", emp.edu(0).when());
  ASSERT_EQ("ncut", emp.edu(0).school());
  ASSERT_EQ("2004-2008", emp.edu(1).when());
  ASSERT_EQ("bj", emp.edu(1).school());

  string toJson = JsonMessage().toJsonString(&emp);
  ASSERT_EQ(json, toJson);

  Employee emp1;
  status = JsonMessage().parseJsonFromString(&emp1, toJson);
  if (status != RC_SUCCESS) {
    DVLOG(2) << "Parse json error. Caused by " << getErrorDescription(status);
    return;
  }

  ASSERT_EQ(emp1.id(), emp.id());
  ASSERT_EQ(emp1.name(), emp.name());
  ASSERT_EQ(emp1.age(), emp.age());
  ASSERT_EQ(emp1.gender(), emp.gender());

  ASSERT_EQ(emp1.role(0), emp.role(0));
  ASSERT_EQ(emp1.role(1), emp.role(1));
  ASSERT_EQ(emp1.role(2), emp.role(2));

  ASSERT_EQ(emp1.dept().id(), emp.dept().id());
  ASSERT_EQ(emp1.dept().name(), emp.dept().name());

  ASSERT_EQ(emp1.edu(0).when(), emp.edu(0).when());
  ASSERT_EQ(emp1.edu(0).school(), emp.edu(0).school());
  ASSERT_EQ(emp1.edu(1).when(), emp.edu(1).when());
  ASSERT_EQ(emp1.edu(1).school(), emp.edu(1).school());
}

TEST(json_proto, from_object) {
  Employee emp;
  emp.set_id(234000);
  emp.set_name("Tom");
  emp.set_age(14);
  emp.set_gender("male");

  emp.add_role("role1");
  emp.add_role("role2");
  emp.add_role("role3");

  emp.mutable_dept()->set_id(19990);
  emp.mutable_dept()->set_name("12345");

  Education* edu = emp.add_edu();
  edu->set_when("2000-2004");
  edu->set_school("ncut");

  string toJson = JsonMessage().toJsonString(&emp);

  Employee emp1;
  ResultCode status = JsonMessage().parseJsonFromString(&emp1, toJson);
  if (status != RC_SUCCESS) {
    DVLOG(2) << "Parse json error. Caused by " << getErrorDescription(status);
    return;
  }

  ASSERT_EQ(emp1.id(), emp.id());
  ASSERT_EQ(emp1.name(), emp.name());
  ASSERT_EQ(emp1.age(), emp.age());
  ASSERT_EQ(emp1.gender(), emp.gender());

  ASSERT_EQ(emp1.role(0), emp.role(0));
  ASSERT_EQ(emp1.role(1), emp.role(1));
  ASSERT_EQ(emp1.role(2), emp.role(2));

  ASSERT_EQ(emp1.dept().id(), emp.dept().id());
  ASSERT_EQ(emp1.dept().name(), emp.dept().name());

  ASSERT_EQ(emp1.edu(0).when(), emp.edu(0).when());
  ASSERT_EQ(emp1.edu(0).school(), emp.edu(0).school());
}

TEST(json_proto, from_file) {
  Employee emp;
  ResultCode status = JsonMessage().parseJsonFromFile(&emp, "framework/test/test.json");
  if (status != RC_SUCCESS) {
    DVLOG(2) << "Parse json error. Caused by " << getErrorDescription(status);
    return;
  }

  ASSERT_EQ(234000, emp.id());
  ASSERT_EQ("Tom", emp.name());
  ASSERT_EQ(14, emp.age());
  ASSERT_EQ("male", emp.gender());

  ASSERT_EQ("role1", emp.role(0));
  ASSERT_EQ("role2", emp.role(1));
  ASSERT_EQ("role3", emp.role(2));

  ASSERT_EQ(19999, emp.dept().id());
  ASSERT_EQ("12345", emp.dept().name());

  ASSERT_EQ("2000-2004", emp.edu(0).when());
  ASSERT_EQ("ncut", emp.edu(0).school());
  ASSERT_EQ("2004-2008", emp.edu(1).when());
  ASSERT_EQ("bj", emp.edu(1).school());
}

