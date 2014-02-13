
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.client;

import idgs.client.util.ProtoSerde;
import idgs.client.util.ProtoSerdeFactory;
import idgs.pb.PbRpcMessage.MemberIdConst;
import idgs.pb.PbRpcMessage.TransportChannel;
import idgs.sample.tpch.pb.Tpch.Customer;
import idgs.sample.tpch.pb.Tpch.CustomerKey;
import idgs.store.pb.PbStoreService.InsertRequest;
import junit.framework.TestCase;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class BaseClientTest extends TestCase {
  
  private static Log log = LogFactory.getLog(BaseClientTest.class);

  public void testClientBase() {
    assertTrue(true);
  }
  
  protected ClientActorMessage createActorMessage() {
// -----------------------copy from c, in order to compare----------------------    
//  std::shared_ptr<CustomerKey> key(new CustomerKey);
//  key->set_c_custkey(10000);
//
//  std::shared_ptr<Customer> customer(new Customer);
//  customer->set_c_name("Tom");
//  customer->set_c_nationkey(10);
//  customer->set_c_address("address");
//  customer->set_c_phone("13800138000");
//  customer->set_c_acctbal(100.123);
//  customer->set_c_comment("customer store test");
//-----------------------copy from c, in order to compare----------------------       
    // key
    CustomerKey.Builder kb = CustomerKey.newBuilder();
    kb.setCCustkey(10000);
    
    // value
    Customer.Builder vb = Customer.newBuilder();
    vb.setCName("Tom");
    vb.setCNationkey(10);
    vb.setCAddress("address");
    vb.setCPhone("13800138000");
    vb.setCAcctbal(100.123);
    vb.setCComment("customer store test");
    
    CustomerKey key = kb.build();
    Customer value = vb.build();

    ProtoSerde serde = ProtoSerdeFactory.createSerde(0);
    log.debug("serialized key: " + serde.serializeToByteArray(key));
    log.debug("serialized value: " + serde.serializeToByteArray(value));
    
    ClientActorMessage msg = new ClientActorMessage();
    msg.setOperationName("insert");
    
    msg.setSourceMemberId(MemberIdConst.CLIENT_MEMBER.getNumber());
    msg.setSourceActorId("client_actor_id");
    
    msg.setDestMemberId(MemberIdConst.ANY_MEMBER.getNumber());
    msg.setDestActorId("store.service");
    
    msg.setChannel(TransportChannel.TC_TCP);

    msg.setPayload(InsertRequest.newBuilder().setStoreName("Customer").build());
    msg.setAttachment("value", value);
    msg.setAttachment("key", key);
    
    return msg;
  }
}
