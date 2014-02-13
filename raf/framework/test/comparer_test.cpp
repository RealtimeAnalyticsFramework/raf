
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "gtest/gtest.h"

#include "idgs/util/singleton.h"
#include "idgs/idgslogging.h"
#include "employee.pb.cc"
#include "protobuf/msg_comparer.h"

using namespace idgs::framework::test::pb;

TEST(Comparer, less) {
  Employee * emp1 = new Employee;
  emp1->set_id(10000);
  emp1->set_name("Tom");

  Employee * emp2 = new Employee;
  emp2->set_id(10000);
  emp2->set_name("Tom");

  ASSERT_FALSE(::idgs::util::singleton<protobuf::less>::getInstance().operator ()(emp1, emp2));

  Employee * emp3 = new Employee;
  emp3->set_id(50000);
  emp3->set_name("Tom");
  emp3->set_age(30);

  ASSERT_TRUE(::idgs::util::singleton<protobuf::less>::getInstance().operator ()(emp1, emp3));

  delete emp1;
  delete emp2;
  delete emp3;
}

TEST(Comparer, equals_to) {
  Employee * emp1 = new Employee;
  emp1->set_id(10000);
  emp1->set_name("Tom");

  Employee * emp2 = new Employee;
  emp2->set_id(10000);
  emp2->set_name("Tom");

  ASSERT_TRUE(::idgs::util::singleton<protobuf::equals_to>::getInstance().operator ()(emp1, emp2));

  Employee * emp3 = new Employee;
  emp3->set_id(50000);
  emp3->set_name("Tom");
  emp3->set_age(30);

  ASSERT_FALSE(::idgs::util::singleton<protobuf::equals_to>::getInstance().operator ()(emp1, emp3));

  delete emp1;
  delete emp2;
  delete emp3;
}

TEST(Comparer, compare_to) {
  Employee * emp1 = new Employee;
  emp1->set_id(10000);
  emp1->set_name("Tom");

  Employee * emp2 = new Employee;
  emp2->set_id(10000);
  emp2->set_name("Tom");

  ASSERT_EQ(0, ::idgs::util::singleton<protobuf::compare_to>::getInstance().operator ()(emp1, emp2));

  Employee * emp3 = new Employee;
  emp3->set_id(50000);
  emp3->set_name("Tom");
  emp3->set_age(30);

  ASSERT_LT(::idgs::util::singleton<protobuf::compare_to>::getInstance().operator ()(emp1, emp3), 0);

  Employee * emp4 = new Employee;
  emp4->set_id(1000);
  emp4->set_name("Tom");

  ASSERT_GT(::idgs::util::singleton<protobuf::compare_to>::getInstance().operator ()(emp1, emp4), 0);

  delete emp1;
  delete emp2;
  delete emp3;
  delete emp4;
}

TEST(Comparer, hash_code) {
  Employee * emp1 = new Employee;
  emp1->set_id(10000);
  emp1->set_name("Tom");

  Employee * emp2 = new Employee;
  emp2->set_id(10000);
  emp2->set_name("Tom");

  int32_t hashcode1 = ::idgs::util::singleton<protobuf::hash_code>::getInstance().operator ()(emp1);
  int32_t hashcode2 = ::idgs::util::singleton<protobuf::hash_code>::getInstance().operator ()(emp2);

  ASSERT_EQ(hashcode1, hashcode2);

  Employee * emp3 = new Employee;
  emp3->set_id(50000);
  emp3->set_name("Tom");
  emp3->set_age(30);

  hashcode2 = ::idgs::util::singleton<protobuf::hash_code>::getInstance().operator ()(emp3);

  ASSERT_NE(hashcode1, hashcode2);

  delete emp1;
  delete emp2;
  delete emp3;
}

//
// it return 0, should be 1
//
TEST(Comparer, compare_float) {
  LOG(INFO) << "float compare: " << (int)(0.1f - 0.09f);
}

TEST(Comparer, orderless) {
  Department * dept1 = new Department;
  dept1->set_id(30000);
  dept1->set_name("Tom");

  Department * dept2 = new Department;
  dept2->set_id(10000);
  dept2->set_name("Jarry");

  Department * dept3 = new Department;
  dept3->set_id(50000);
  dept3->set_name("Lucy");

  Department * dept4 = new Department;
  dept4->set_id(30000);
  dept4->set_name("Candy");

  std::shared_ptr<google::protobuf::Message> data1(dept1);
  std::shared_ptr<google::protobuf::Message> data2(dept2);
  std::shared_ptr<google::protobuf::Message> data3(dept3);
  std::shared_ptr<google::protobuf::Message> data4(dept4);

  LOG(INFO) << "order by id asc, name asc";
  protobuf::orderless order1(std::vector<bool>{false, false});
  std::map<std::shared_ptr<google::protobuf::Message>, std::shared_ptr<google::protobuf::Message>, protobuf::orderless> ordermap1(order1);
  ordermap1[data1] = data1;
  ordermap1[data2] = data2;
  ordermap1[data3] = data3;
  ordermap1[data4] = data4;

  for (auto l = ordermap1.begin(); l != ordermap1.end(); ++ l) {
    Department* dept = dynamic_cast<Department*>(l->first.get());
    LOG(INFO) << "id = " << dept->id() << ", name = " << dept->name();
  }

  auto it = ordermap1.begin();
  Department* ordered_dept1 = dynamic_cast<Department*>(it->first.get());
  ++ it;
  Department* ordered_dept2 = dynamic_cast<Department*>(it->first.get());
  ++ it;
  Department* ordered_dept3 = dynamic_cast<Department*>(it->first.get());
  ++ it;
  Department* ordered_dept4 = dynamic_cast<Department*>(it->first.get());

  EXPECT_EQ(10000, ordered_dept1->id());
  EXPECT_EQ("Jarry", ordered_dept1->name());
  EXPECT_EQ(30000, ordered_dept2->id());
  EXPECT_EQ("Candy", ordered_dept2->name());
  EXPECT_EQ(30000, ordered_dept3->id());
  EXPECT_EQ("Tom", ordered_dept3->name());
  EXPECT_EQ(50000, ordered_dept4->id());
  EXPECT_EQ("Lucy", ordered_dept4->name());

  LOG(INFO) << "order by id desc, name asc";
  protobuf::orderless order2(std::vector<bool>{true, false});
  std::map<std::shared_ptr<google::protobuf::Message>, std::shared_ptr<google::protobuf::Message>, protobuf::orderless> ordermap2(order2);
  ordermap2[data1] = data1;
  ordermap2[data2] = data2;
  ordermap2[data3] = data3;
  ordermap2[data4] = data4;

  for (auto l = ordermap2.begin(); l != ordermap2.end(); ++ l) {
    Department* dept = dynamic_cast<Department*>(l->first.get());
    LOG(INFO) << "id = " << dept->id() << ", name = " << dept->name();
  }

  it = ordermap2.begin();
  ordered_dept1 = dynamic_cast<Department*>(it->first.get());
  ++ it;
  ordered_dept2 = dynamic_cast<Department*>(it->first.get());
  ++ it;
  ordered_dept3 = dynamic_cast<Department*>(it->first.get());
  ++ it;
  ordered_dept4 = dynamic_cast<Department*>(it->first.get());

  EXPECT_EQ(50000, ordered_dept1->id());
  EXPECT_EQ("Lucy", ordered_dept1->name());
  EXPECT_EQ(30000, ordered_dept2->id());
  EXPECT_EQ("Candy", ordered_dept2->name());
  EXPECT_EQ(30000, ordered_dept3->id());
  EXPECT_EQ("Tom", ordered_dept3->name());
  EXPECT_EQ(10000, ordered_dept4->id());
  EXPECT_EQ("Jarry", ordered_dept4->name());

  LOG(INFO) << "order by id asc, name desc";
  protobuf::orderless order3(std::vector<bool>{false, true});
  std::map<std::shared_ptr<google::protobuf::Message>, std::shared_ptr<google::protobuf::Message>, protobuf::orderless> ordermap3(order3);
  ordermap3[data1] = data1;
  ordermap3[data2] = data2;
  ordermap3[data3] = data3;
  ordermap3[data4] = data4;

  for (auto l = ordermap3.begin(); l != ordermap3.end(); ++ l) {
    Department* dept = dynamic_cast<Department*>(l->first.get());
    LOG(INFO) << "id = " << dept->id() << ", name = " << dept->name();
  }

  it = ordermap3.begin();
  ordered_dept1 = dynamic_cast<Department*>(it->first.get());
  ++ it;
  ordered_dept2 = dynamic_cast<Department*>(it->first.get());
  ++ it;
  ordered_dept3 = dynamic_cast<Department*>(it->first.get());
  ++ it;
  ordered_dept4 = dynamic_cast<Department*>(it->first.get());

  EXPECT_EQ(10000, ordered_dept1->id());
  EXPECT_EQ("Jarry", ordered_dept1->name());
  EXPECT_EQ(30000, ordered_dept2->id());
  EXPECT_EQ("Tom", ordered_dept2->name());
  EXPECT_EQ(30000, ordered_dept3->id());
  EXPECT_EQ("Candy", ordered_dept3->name());
  EXPECT_EQ(50000, ordered_dept4->id());
  EXPECT_EQ("Lucy", ordered_dept4->name());

  LOG(INFO) << "order by id desc, name desc";
  protobuf::orderless order4(std::vector<bool>{true, true});
  std::map<std::shared_ptr<google::protobuf::Message>, std::shared_ptr<google::protobuf::Message>, protobuf::orderless> ordermap4(order4);
  ordermap4[data1] = data1;
  ordermap4[data2] = data2;
  ordermap4[data3] = data3;
  ordermap4[data4] = data4;

  for (auto l = ordermap4.begin(); l != ordermap4.end(); ++ l) {
    Department* dept = dynamic_cast<Department*>(l->first.get());
    LOG(INFO) << "id = " << dept->id() << ", name = " << dept->name();
  }

  it = ordermap4.begin();
  ordered_dept1 = dynamic_cast<Department*>(it->first.get());
  ++ it;
  ordered_dept2 = dynamic_cast<Department*>(it->first.get());
  ++ it;
  ordered_dept3 = dynamic_cast<Department*>(it->first.get());
  ++ it;
  ordered_dept4 = dynamic_cast<Department*>(it->first.get());

  EXPECT_EQ(50000, ordered_dept1->id());
  EXPECT_EQ("Lucy", ordered_dept1->name());
  EXPECT_EQ(30000, ordered_dept2->id());
  EXPECT_EQ("Tom", ordered_dept2->name());
  EXPECT_EQ(30000, ordered_dept3->id());
  EXPECT_EQ("Candy", ordered_dept3->name());
  EXPECT_EQ(10000, ordered_dept4->id());
  EXPECT_EQ("Jarry", ordered_dept4->name());
}
