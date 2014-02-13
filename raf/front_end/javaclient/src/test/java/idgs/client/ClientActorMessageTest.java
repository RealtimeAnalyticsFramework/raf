
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.client;

import idgs.client.ClientActorMessage;
import idgs.client.util.ProtoSerde;
import idgs.client.util.ProtoSerdeFactory;
import idgs.pb.PbRpcMessage.RpcMessage;
import idgs.pb.PbRpcMessage.TransportChannel;
import idgs.sample.tpch.pb.Tpch.Customer;
import idgs.sample.tpch.pb.Tpch.CustomerKey;
import idgs.store.pb.PbStoreService.InsertRequest;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class ClientActorMessageTest extends BaseClientTest {
	
	private static Log log = LogFactory.getLog(ClientActorMessageTest.class);

	public void testConstruct() {
	  log.info("test construct a ClientActorMessage without parameters");
		ClientActorMessage msg = new ClientActorMessage();
		assertEquals(TransportChannel.TC_AUTO, msg.getChannel()); // default
	}
	
	public void testToBuffer() {
	  log.info("test serialize/deserialize a ClientActorMessage");
	  ClientActorMessage request = createActorMessage();
    // serialize
    byte[] array = request.toBuffer();
    String src = request.toString();
    // deserialize
    ProtoSerde serde = ProtoSerdeFactory.createSerde(request.getSerdesType().getNumber());
    RpcMessage.Builder builder = RpcMessage.newBuilder();
    serde.deserializeFromByteArray(builder, array);
    ClientActorMessage response = new ClientActorMessage(builder);
    response.parsePayload(InsertRequest.newBuilder());
    response.parseAttachment("key", CustomerKey.newBuilder());
    response.parseAttachment("value", Customer.newBuilder());
    String dst = response.toString();
    
    assertEquals(src, dst);
    log.debug(dst);
	}
}
