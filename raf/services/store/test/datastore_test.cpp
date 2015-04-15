
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h"
#endif // GNUC_ $

#include <gtest/gtest.h>

#include "customer.pb.cc"

#include "idgs/application.h"
#include "idgs/store/data_store.h"

using namespace idgs;
using namespace idgs::sample::tpch::pb;
using namespace idgs::store;
using namespace google::protobuf;

namespace idgs {
namespace store_test {
  DataStore datastore;
  size_t partSize;
}
}

TEST(DataStore, init) {
  auto cluster = idgs_application()->getClusterFramework();
  auto code = cluster->loadCfgFile("conf/cluster.conf");
  ASSERT_TRUE(code == RC_SUCCESS);

  idgs::store_test::partSize = cluster->getPartitionCount();

  code = idgs::store_test::datastore.loadCfgFile("services/store/test/test_data_store.conf");
  ASSERT_TRUE(code == RC_SUCCESS);

  code = idgs::store_test::datastore.start();
  ASSERT_TRUE(code == RC_SUCCESS);
}

TEST(DataStore, loadCfgFile) {
  auto store = idgs::store_test::datastore.getStore("Customer");
  ASSERT_TRUE(store.get() != NULL);
  auto& storeConfigWrapper = store->getStoreConfigWrapper();
  ASSERT_TRUE(storeConfigWrapper.get() != NULL);

  ASSERT_EQ("Customer", storeConfigWrapper->getStoreConfig().name());
  ASSERT_EQ(idgs::store::pb::ORDERED, storeConfigWrapper->getStoreConfig().store_type());
  ASSERT_EQ(idgs::store::pb::PARTITION_TABLE, storeConfigWrapper->getStoreConfig().partition_type());
  ASSERT_EQ("idgs.sample.tpch.pb.CustomerKey", storeConfigWrapper->getStoreConfig().key_type());
  ASSERT_EQ("idgs.sample.tpch.pb.Customer", storeConfigWrapper->getStoreConfig().value_type());

  auto storePartSupp = idgs::store_test::datastore.getStore("PartSupp");
  ASSERT_TRUE(storePartSupp.get() != NULL);

  auto storeNumber = idgs::store_test::datastore.getStore("Number");
  ASSERT_TRUE(storeNumber.get() == NULL);
}

TEST(DataStore, insertData) {
  ResultCode status;
  PartitionInfo ps;
  hashcode_t hash;

  shared_ptr<CustomerKey> customerKey1 = make_shared<CustomerKey>();
  customerKey1->set_c_custkey(11000);

  shared_ptr<Customer> customer1 = make_shared<Customer>();
  customer1->set_c_name("Tom1");
  customer1->set_c_nationkey(20);
  customer1->set_c_phone("13800138000");

  StoreKey<Message> key1(customerKey1);
  StoreValue<Message> value1(customer1);

  hash = protobuf::HashCode::hashcode(key1.get());
  ps.partitionId = (hash % idgs::store_test::partSize);
  auto store = idgs::store_test::datastore.getStore("Customer");

  status = store->setData(key1, value1, &ps);
  ASSERT_EQ(RC_SUCCESS, status);

  shared_ptr<CustomerKey> customerKey2 = make_shared<CustomerKey>();
  customerKey2->set_c_custkey(12010);
  shared_ptr<Customer> customer2 = make_shared<Customer>();
  customer2->set_c_name("Tom2");
  customer2->set_c_nationkey(25);
  customer2->set_c_phone("13800138000");

  StoreKey<Message> key2(customerKey2);
  StoreValue<Message> value2(customer2);

  hash = protobuf::HashCode::hashcode(key2.get());
  ps.partitionId = (hash % idgs::store_test::partSize);
  status = store->setData(key2, value2, &ps);
  ASSERT_EQ(RC_SUCCESS, status);

  shared_ptr<CustomerKey> customerKey3 = make_shared<CustomerKey>();
  customerKey3->set_c_custkey(987654321);
  shared_ptr<Customer> customer3 = make_shared<Customer>();
  customer3->set_c_name("Jerry");
  customer3->set_c_nationkey(40);
  customer3->set_c_phone("13800138000");

  StoreKey<Message> key3(customerKey3);
  StoreValue<Message> value3(customer3);

  hash = protobuf::HashCode::hashcode(key3.get());
  ps.partitionId = (hash % idgs::store_test::partSize);
  status = store->setData(key3, value3, &ps);
  ASSERT_EQ(RC_SUCCESS, status);
}

TEST(DataStore, getData) {
  ResultCode status;
  PartitionInfo ps;
  hashcode_t hash;

  shared_ptr<CustomerKey> customerKey1 = make_shared<CustomerKey>();
  customerKey1->set_c_custkey(11000);

  shared_ptr<Customer> customer1 = make_shared<Customer>();

  StoreKey<Message> key1(customerKey1);
  StoreValue<Message> value1(customer1);

  auto store = idgs::store_test::datastore.getStore("Customer");

  hash = protobuf::HashCode::hashcode(key1.get());
  ps.partitionId = (hash % idgs::store_test::partSize);
  status = store->getData(key1, value1, &ps);
  ASSERT_EQ(RC_SUCCESS, status);

  Customer* result = (Customer *) value1.get().get();
  ASSERT_EQ("Tom1", result->c_name());
  ASSERT_EQ(20, result->c_nationkey());
  ASSERT_EQ("13800138000", result->c_phone());

  shared_ptr<CustomerKey> customerKey2 = make_shared<CustomerKey>();
  customerKey2->set_c_custkey(12010);

  shared_ptr<Customer> customer2 = make_shared<Customer>();

  StoreKey<Message> key2(customerKey2);
  StoreValue<Message> value2(customer2);

  hash = protobuf::HashCode::hashcode(key2.get());
  ps.partitionId = (hash % idgs::store_test::partSize);
  status = store->getData(key2, value2, &ps);
  ASSERT_EQ(RC_SUCCESS, status);

  shared_ptr<CustomerKey> customerKey3 = make_shared<CustomerKey>();
  customerKey3->set_c_custkey(20000);

  StoreKey<Message> key3(customerKey3);
  StoreValue<Message> value3(make_shared<Customer>());

  hash = protobuf::HashCode::hashcode(key3.get());
  ps.partitionId = (hash % idgs::store_test::partSize);
  status = store->getData(key3, value3, &ps);
  ASSERT_EQ(RC_DATA_NOT_FOUND, status);
}

TEST(DataStore, updateData) {
  ResultCode status;
  PartitionInfo ps;
  hashcode_t hash;

  shared_ptr<CustomerKey> customerKey1 = make_shared<CustomerKey>();
  customerKey1->set_c_custkey(11000);

  shared_ptr<Customer> customer1 = make_shared<Customer>();
  customer1->set_c_name("Kate");
  customer1->set_c_nationkey(50);
  customer1->set_c_phone("13800138000");

  StoreKey<Message> key1(customerKey1);
  StoreValue<Message> value1(customer1);

  auto store = idgs::store_test::datastore.getStore("Customer");

  hash = protobuf::HashCode::hashcode(key1.get());
  ps.partitionId = (hash % idgs::store_test::partSize);
  status = store->setData(key1, value1, &ps);
  ASSERT_EQ(RC_SUCCESS, status);

  shared_ptr<CustomerKey> customerKey2 = make_shared<CustomerKey>();
  customerKey2->set_c_custkey(11000);

  shared_ptr<Customer> customer2 = make_shared<Customer>();

  StoreKey<Message> key2(customerKey2);
  StoreValue<Message> value2(customer2);

  hash = protobuf::HashCode::hashcode(key2.get());
  ps.partitionId = (hash % idgs::store_test::partSize);
  status = store->setData(key2, value2, &ps);
  ASSERT_EQ(RC_SUCCESS, status);

  ASSERT_TRUE(value2.get() != NULL);

  Customer* result = (Customer *) value2.get().get();
  ASSERT_EQ("Kate", result->c_name());
  ASSERT_EQ(50, result->c_nationkey());
  ASSERT_EQ("13800138000", result->c_phone());
}

TEST(DataStore, removeData) {
  ResultCode status;
  PartitionInfo ps;
  hashcode_t hash;

  shared_ptr<CustomerKey> customerKey1 = make_shared<CustomerKey>();
  customerKey1->set_c_custkey(11000);

  shared_ptr<Customer> customer1 = make_shared<Customer>();

  StoreKey<Message> key1(customerKey1);
  StoreValue<Message> value1(customer1);

  auto store = idgs::store_test::datastore.getStore("Customer");

  hash = protobuf::HashCode::hashcode(key1.get());
  ps.partitionId = (hash % idgs::store_test::partSize);
  status = store->removeData(key1, value1, &ps);
  ASSERT_EQ(RC_SUCCESS, status);

  shared_ptr<CustomerKey> customerKey2 = make_shared<CustomerKey>();
  customerKey2->set_c_custkey(11000);

  shared_ptr<Customer> customer2 = make_shared<Customer>();

  StoreKey<Message> key2(customerKey2);
  StoreValue<Message> value2(customer2);

  hash = protobuf::HashCode::hashcode(key2.get());
  ps.partitionId = (hash % idgs::store_test::partSize);
  status = store->getData(key2, value2, &ps);
  ASSERT_EQ(RC_DATA_NOT_FOUND, status);
}

TEST(DataStore, dynamicMessage) {
  ResultCode status;
  PartitionInfo ps;
  hashcode_t hash;

  auto store = idgs::store_test::datastore.getStore("Supplier");
  auto& storeConfigWrapper = store->getStoreConfigWrapper();

  auto SupplierKey = storeConfigWrapper->newKey();
  const Reflection* keyReflection = SupplierKey->GetReflection();
  const Descriptor* keyDescriptor = SupplierKey->GetDescriptor();
  const FieldDescriptor* field = NULL;

  field = keyDescriptor->FindFieldByName("s_suppkey");
  keyReflection->SetInt64(SupplierKey.get(), field, 5000);

  auto Supplier = storeConfigWrapper->newValue();
  const Reflection* valueReflection = Supplier->GetReflection();
  const Descriptor* valueDescriptor = Supplier->GetDescriptor();

  valueReflection->SetString(Supplier.get(), valueDescriptor->FindFieldByName("s_name"), "Jerry");
  valueReflection->SetInt64(Supplier.get(), valueDescriptor->FindFieldByName("s_nationkey"), 16);
  valueReflection->SetString(Supplier.get(), valueDescriptor->FindFieldByName("s_comment"), "comment");

  StoreKey<Message> key1(SupplierKey);
  StoreValue<Message> value1(Supplier);

  hash = protobuf::HashCode::hashcode(key1.get());
  ps.partitionId = (hash % idgs::store_test::partSize);
  status = store->setData(key1, value1, &ps);
  ASSERT_EQ(RC_SUCCESS, status);

  Supplier = storeConfigWrapper->newValue();
  StoreValue<Message> value2(Supplier);
  status = store->getData(key1, value2, &ps);
  ASSERT_EQ(RC_SUCCESS, status);

  ASSERT_EQ("Jerry", valueReflection->GetString(* value2.get(), valueDescriptor->FindFieldByName("s_name")));
  ASSERT_EQ(16, valueReflection->GetInt64(* value2.get(), valueDescriptor->FindFieldByName("s_nationkey")));
  ASSERT_EQ("comment", valueReflection->GetString(* value2.get(), valueDescriptor->FindFieldByName("s_comment")));

  Supplier = storeConfigWrapper->newValue();
  StoreValue<Message> value3(Supplier);
  status = store->removeData(key1, value3, &ps);
  ASSERT_TRUE(status == RC_SUCCESS);

  ASSERT_EQ("Jerry", valueReflection->GetString(* value3.get(), valueDescriptor->FindFieldByName("s_name")));
  ASSERT_EQ(16, valueReflection->GetInt64(* value3.get(), valueDescriptor->FindFieldByName("s_nationkey")));
  ASSERT_EQ("comment", valueReflection->GetString(* value3.get(), valueDescriptor->FindFieldByName("s_comment")));
}

TEST(DataStore, stop) {
  ResultCode code = idgs::store_test::datastore.stop();
  ASSERT_EQ(RC_SUCCESS, code);
}
