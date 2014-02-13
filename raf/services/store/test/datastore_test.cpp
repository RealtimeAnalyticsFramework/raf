
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

#include "idgs/store/data_store.h"
#include "protobuf/message_helper.h"
#include "idgs/cluster/cluster_framework.h"

using namespace idgs;
using namespace idgs::sample::tpch::pb;
using namespace idgs::store;
using namespace google::protobuf;
using namespace protobuf;

TEST(DataStore, loadCfgFile) {
  ResultCode status = ::idgs::util::singleton<DataStore>::getInstance().loadCfgFile("services/store/test/test_data_store.conf");
  ASSERT_TRUE(status == RC_SUCCESS);

  shared_ptr<StoreConfigWrapper> storeConfigWrapper;
  status = ::idgs::util::singleton<DataStore>::getInstance().loadStoreConfig("Customer", storeConfigWrapper);
  ASSERT_TRUE(status == RC_SUCCESS);

  ASSERT_EQ("Customer", storeConfigWrapper->getStoreConfig().name());
  ASSERT_EQ(idgs::store::pb::ORDERED, storeConfigWrapper->getStoreConfig().store_type());
  ASSERT_EQ(idgs::store::pb::PARTITION_TABLE, storeConfigWrapper->getStoreConfig().partition_type());
  ASSERT_EQ("idgs.sample.tpch.pb.CustomerKey", storeConfigWrapper->getStoreConfig().key_type());
  ASSERT_EQ("idgs.sample.tpch.pb.Customer", storeConfigWrapper->getStoreConfig().value_type());

//  ASSERT_EQ(storeConfigWrapper->getStoreConfig().listener_configs_size(), 2);

//  idgs::store::pb::ListenerConfig listener_config;
//  status = storeConfigWrapper->getListenerConfig("listener1", listener_config);
//  ASSERT_TRUE(status == RC_SUCCESS);

//  ASSERT_EQ("index", listener_config.type());
//  ASSERT_EQ(1, listener_config.params_size());

//  string param;
//  storeConfigWrapper->getListenerParam("listener1", "cols", param);
//  ASSERT_EQ("id0,name0", param);

  status = ::idgs::util::singleton<DataStore>::getInstance().loadStoreConfig("PartSupp", storeConfigWrapper);
  ASSERT_TRUE(status == RC_SUCCESS);

  status = ::idgs::util::singleton<DataStore>::getInstance().loadStoreConfig("Number", storeConfigWrapper);
  ASSERT_TRUE(status != RC_SUCCESS);
}

TEST(DataStore, initialize) {
  ::idgs::util::singleton<idgs::cluster::ClusterFramework>::getInstance().loadCfgFile("framework/conf/cluster.conf");
  size_t partSize = ::idgs::util::singleton<idgs::cluster::ClusterFramework>::getInstance().getPartitionCount();

  for (int32_t i = 0; i < partSize; ++ i) {
    ::idgs::util::singleton<DataStore>::getInstance().migrateData(i, 0, -1, 0);
  }

  StoreKey<Message> key(shared_ptr<Customer>(new Customer));
  StoreValue<Message> value(shared_ptr<Customer>(new Customer));

  ResultCode status = ::idgs::util::singleton<DataStore>::getInstance().getData("Customer", key, value);
  ASSERT_TRUE(status != RC_STORE_NOT_FOUND);

  status = ::idgs::util::singleton<DataStore>::getInstance().getData("Company", key, value);
  ASSERT_TRUE(status == RC_STORE_NOT_FOUND);
}

TEST(DataStore, insertData) {
  ResultCode status;

  shared_ptr<CustomerKey> customerKey1(new CustomerKey);
  customerKey1->set_c_custkey(11000);

  shared_ptr<Customer> customer1(new Customer);
  customer1->set_c_name("Tom1");
  customer1->set_c_nationkey(20);
  customer1->set_c_phone("13800138000");

  StoreKey<Message> key1(customerKey1);
  StoreValue<Message> value1(customer1);

  status = ::idgs::util::singleton<DataStore>::getInstance().insertData("Customer", key1, value1);
  ASSERT_TRUE(status == RC_SUCCESS);

  shared_ptr<CustomerKey> customerKey2(new CustomerKey);
  customerKey2->set_c_custkey(12010);
  shared_ptr<Customer> customer2(new Customer);
  customer2->set_c_name("Tom2");
  customer2->set_c_nationkey(25);
  customer2->set_c_phone("13800138000");

  StoreKey<Message> key2(customerKey2);
  StoreValue<Message> value2(customer2);

  status = ::idgs::util::singleton<DataStore>::getInstance().insertData("Customer", key2, value2);
  ASSERT_TRUE(status == RC_SUCCESS);

  shared_ptr<CustomerKey> customerKey3(new CustomerKey);
  customerKey3->set_c_custkey(987654321);
  shared_ptr<Customer> customer3(new Customer);
  customer3->set_c_name("Jerry");
  customer3->set_c_nationkey(40);
  customer3->set_c_phone("13800138000");

  StoreKey<Message> key3(customerKey3);
  StoreValue<Message> value3(customer3);

  status = ::idgs::util::singleton<DataStore>::getInstance().insertData("Customer", key3, value3);
  ASSERT_TRUE(status == RC_SUCCESS);
}

TEST(DataStore, getData) {
  ResultCode status;

  shared_ptr<CustomerKey> customerKey1(new CustomerKey);
  customerKey1->set_c_custkey(11000);

  shared_ptr<Customer> customer1(new Customer);

  StoreKey<Message> key1(customerKey1);
  StoreValue<Message> value1(customer1);

  status = ::idgs::util::singleton<DataStore>::getInstance().getData("Customer", key1, value1);
  ASSERT_TRUE(status == RC_SUCCESS);

  Customer* result = (Customer *) value1.get().get();
  ASSERT_EQ("Tom1", result->c_name());
  ASSERT_EQ(20, result->c_nationkey());
  ASSERT_EQ("13800138000", result->c_phone());

  shared_ptr<CustomerKey> customerKey2(new CustomerKey);
  customerKey2->set_c_custkey(12010);

  shared_ptr<Customer> customer2(new Customer);

  StoreKey<Message> key2(customerKey2);
  StoreValue<Message> value2(customer2);

  status = ::idgs::util::singleton<DataStore>::getInstance().getData("Customer", key2, value2);
  ASSERT_TRUE(status == RC_SUCCESS);

  shared_ptr<CustomerKey> customerKey3(new CustomerKey);
  customerKey3->set_c_custkey(20000);

  StoreKey<Message> key3(customerKey3);
  StoreValue<Message> value3(shared_ptr<Customer>(new Customer));

  status = ::idgs::util::singleton<DataStore>::getInstance().getData("Customer", key3, value3);
  ASSERT_TRUE(status == RC_DATA_NOT_FOUND);
}

TEST(DataStore, updateData) {
  ResultCode status;

  shared_ptr<CustomerKey> customerKey1(new CustomerKey);
  customerKey1->set_c_custkey(11000);

  shared_ptr<Customer> customer1(new Customer);
  customer1->set_c_name("Kate");
  customer1->set_c_nationkey(50);
  customer1->set_c_phone("13800138000");

  StoreKey<Message> key1(customerKey1);
  StoreValue<Message> value1(customer1);

  status = ::idgs::util::singleton<DataStore>::getInstance().updateData("Customer", key1, value1);
  ASSERT_TRUE(status == RC_SUCCESS);

  shared_ptr<CustomerKey> customerKey2(new CustomerKey);
  customerKey2->set_c_custkey(11000);

  shared_ptr<Customer> customer2(new Customer);

  StoreKey<Message> key2(customerKey2);
  StoreValue<Message> value2(customer2);

  status = ::idgs::util::singleton<DataStore>::getInstance().getData("Customer", key2, value2);
  ASSERT_TRUE(status == RC_SUCCESS);

  ASSERT_TRUE(value2.get() != NULL);

  Customer* result = (Customer *) value2.get().get();
  ASSERT_EQ("Kate", result->c_name());
  ASSERT_EQ(50, result->c_nationkey());
  ASSERT_EQ("13800138000", result->c_phone());
}

TEST(DataStore, removeData) {
  ResultCode status;

  shared_ptr<CustomerKey> customerKey1(new CustomerKey);
  customerKey1->set_c_custkey(11000);

  shared_ptr<Customer> customer1(new Customer);

  StoreKey<Message> key1(customerKey1);
  StoreValue<Message> value1(customer1);

  status = ::idgs::util::singleton<DataStore>::getInstance().removeData("Customer", key1, value1);
  ASSERT_TRUE(status == RC_SUCCESS);

  shared_ptr<CustomerKey> customerKey2(new CustomerKey);
  customerKey2->set_c_custkey(11000);

  shared_ptr<Customer> customer2(new Customer);

  StoreKey<Message> key2(customerKey2);
  StoreValue<Message> value2(customer2);

  status = ::idgs::util::singleton<DataStore>::getInstance().getData("Customer", key2, value2);
  ASSERT_TRUE(status == RC_DATA_NOT_FOUND);
}

TEST(DataStore, dynamicMessage) {
  ResultCode status;

  auto SupplierKey = ::idgs::util::singleton<MessageHelper>::getInstance().createMessage("idgs.sample.tpch.pb.SupplierKey");
  const Reflection* keyReflection = SupplierKey->GetReflection();
  const Descriptor* keyDescriptor = SupplierKey->GetDescriptor();
  const FieldDescriptor* field = NULL;

  field = keyDescriptor->FindFieldByName("s_suppkey");
  keyReflection->SetInt64(SupplierKey.get(), field, 5000);

  auto Supplier = ::idgs::util::singleton<MessageHelper>::getInstance().createMessage("idgs.sample.tpch.pb.Supplier");
  const Reflection* valueReflection = Supplier->GetReflection();
  const Descriptor* valueDescriptor = Supplier->GetDescriptor();

  valueReflection->SetString(Supplier.get(), valueDescriptor->FindFieldByName("s_name"), "Jerry");
  valueReflection->SetInt64(Supplier.get(), valueDescriptor->FindFieldByName("s_nationkey"), 16);
  valueReflection->SetString(Supplier.get(), valueDescriptor->FindFieldByName("s_comment"), "comment");

  StoreKey<Message> key1(SupplierKey);
  StoreValue<Message> value1(Supplier);

  status = ::idgs::util::singleton<DataStore>::getInstance().insertData("Supplier", key1, value1);
  ASSERT_TRUE(status == RC_SUCCESS);

  Supplier = ::idgs::util::singleton<MessageHelper>::getInstance().createMessage("idgs.sample.tpch.pb.Supplier");
  StoreValue<Message> value2(Supplier);
  status = ::idgs::util::singleton<DataStore>::getInstance().getData("Supplier", key1, value2);
  ASSERT_TRUE(status == RC_SUCCESS);

  ASSERT_EQ("Jerry", valueReflection->GetString(* value2.get(), valueDescriptor->FindFieldByName("s_name")));
  ASSERT_EQ(16, valueReflection->GetInt64(* value2.get(), valueDescriptor->FindFieldByName("s_nationkey")));
  ASSERT_EQ("comment", valueReflection->GetString(* value2.get(), valueDescriptor->FindFieldByName("s_comment")));

  Supplier = ::idgs::util::singleton<MessageHelper>::getInstance().createMessage("idgs.sample.tpch.pb.Supplier");
  StoreValue<Message> value3(Supplier);
  status = ::idgs::util::singleton<DataStore>::getInstance().removeData("Supplier", key1, value3);
  ASSERT_TRUE(status == RC_SUCCESS);

  ASSERT_EQ("Jerry", valueReflection->GetString(* value3.get(), valueDescriptor->FindFieldByName("s_name")));
  ASSERT_EQ(16, valueReflection->GetInt64(* value3.get(), valueDescriptor->FindFieldByName("s_nationkey")));
  ASSERT_EQ("comment", valueReflection->GetString(* value3.get(), valueDescriptor->FindFieldByName("s_comment")));
}

TEST(DataStore, stop) {
  ResultCode code = ::idgs::util::singleton<DataStore>::getInstance().stop();
  ASSERT_EQ(RC_SUCCESS, code);
}
