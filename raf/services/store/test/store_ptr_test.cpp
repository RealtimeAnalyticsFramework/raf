
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "gtest/gtest.h"

#include <unordered_map>
#include "customer.pb.cc"
#include "idgs/store/comparer.h"

using namespace idgs::store;
using namespace idgs::sample::tpch::pb;
using namespace google::protobuf;

TEST(StorePtr, TreeMap) {
  std::map<StoreKey<Message>, StoreValue<Message>, idgs::store::less> treemap;

  shared_ptr<CustomerKey> key1 = make_shared<CustomerKey>();
  key1->set_c_custkey(100000);
  StoreKey<Message> storeKey1(key1);

  shared_ptr<Customer> value1 = make_shared<Customer>();
  value1->set_c_name("Customer");
  StoreValue<Message> storeValue1(value1);

  treemap[storeKey1] = storeValue1;

  shared_ptr<CustomerKey> key2 = make_shared<CustomerKey>();
  key2->set_c_custkey(100000);
  StoreKey<Message> storeKey2(key2);

  shared_ptr<Customer> value2 = make_shared<Customer>();
  StoreValue<Message> storeValue2(value2);

  storeValue2 = treemap[storeKey2];

  Customer* result = (Customer*) storeValue2.get().get();
  ASSERT_EQ("Customer", result->c_name());
}

TEST(StorePtr, HashMap) {
  std::unordered_map<StoreKey<Message>, StoreValue<Message>, hash_code, equals_to> hashmap;

  shared_ptr<CustomerKey> key1 = make_shared<CustomerKey>();
  key1->set_c_custkey(100000);
  StoreKey<Message> storeKey1(key1);

  shared_ptr<Customer> value1 = make_shared<Customer>();
  value1->set_c_name("Customer");
  StoreValue<Message> storeValue1(value1);

  hashmap[storeKey1] = storeValue1;

  shared_ptr<CustomerKey> key2 = make_shared<CustomerKey>();
  key2->set_c_custkey(100000);
  StoreKey<Message> storeKey2(key2);

  shared_ptr<Customer> value2 = make_shared<Customer>();
  StoreValue<Message> storeValue2(value2);

  storeValue2 = hashmap[storeKey2];

  Customer* result = (Customer*) storeValue2.get().get();
  ASSERT_EQ("Customer", result->c_name());
}
