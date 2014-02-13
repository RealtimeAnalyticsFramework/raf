
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "gtest/gtest.h"

#include "protobuf/hash_code.h"
#include "idgs/idgslogging.h"
#include "employee.pb.cc"

using namespace protobuf;
using namespace idgs::framework::test::pb;

TEST(HashCode, message) {
  Employee * emp1 = new Employee;
  emp1->set_id(12345);
  emp1->set_name("abcde");
  emp1->set_gender("male");
  emp1->set_age(18);

  emp1->add_role("role1");
  emp1->add_role("role2");
  emp1->add_role("role3");

  Employee * emp2 = new Employee;
  emp2->CopyFrom(* emp1);

  int32_t hashcode1 = HashCode::hashcode(emp1);
  int32_t hashcode2 = HashCode::hashcode(emp2);

  DVLOG(2) << "------------ test hashcode ------------";
  DVLOG(2) << "employee 1 hash code : " << hashcode1;
  DVLOG(2) << "employee 2 hash code : " << hashcode2;

  ASSERT_EQ(hashcode1, hashcode2);

  emp2->add_role("role3");
  hashcode2 = HashCode::hashcode(emp2);

  DVLOG(2) << "update employee 2 hash code : " << hashcode2;
  DVLOG(2) << "---------------------------------------";

  ASSERT_NE(hashcode1, hashcode2);

  delete emp1;
  delete emp2;
}
