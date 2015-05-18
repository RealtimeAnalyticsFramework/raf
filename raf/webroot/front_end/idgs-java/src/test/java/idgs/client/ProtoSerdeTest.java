
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.client;

import idgs.client.pb.PbClientConfig.ClientConfig;
import idgs.client.util.JsonUtil;
import idgs.client.util.ProtoSerde;
import idgs.client.util.ProtoSerdeFactory;

import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class ProtoSerdeTest extends BaseClientTest {
  
  private static Log log = LogFactory.getLog(ProtoSerdeTest.class);

	/**
	 * 1. json => pb1, pb1 => json, json => pb2
	 * 2. pb2 => binary, binary => pb3
	 * 3. pb3 => text, text => pb4
	 * assert pb1 == pb2 == pb3 == pb4
	 */
	public void testSerde() throws IOException {
	  log.info("test json <=> pb, binary <=> pb, text <=> pb serilaze/deserialize");
	  // json serde
	  // String jsonFile = getClass().getResource("/client.conf").getPath();
	  String jsonFile = "conf/client.conf";
	  String home = System.getenv("IDGS_HOME");
	  if (home != null) {
	    jsonFile = home + "/" + jsonFile;
	  } else {
	    jsonFile = "../../conf/client.conf";
	  }
	  final String expectString = JsonUtil.getJsonFromFile(jsonFile).toString();
	  ProtoSerde serde = ProtoSerdeFactory.createSerde(1);
	  ClientConfig.Builder builder = ClientConfig.newBuilder();
	  serde.deserializeFromByteArray(builder, expectString.getBytes());
	  ClientConfig cfg1 = builder.build();
	  byte[] array = serde.serializeToByteArray(cfg1);
	  final String actualString = new String(array);
	  builder = ClientConfig.newBuilder();
	  serde.deserializeFromByteArray(builder, actualString.getBytes());
	  ClientConfig cfg2 = builder.build();
	  
	  // binary serde
	  serde = ProtoSerdeFactory.createSerde();
	  array = serde.serializeToByteArray(cfg1);
	  builder = ClientConfig.newBuilder();
	  serde.deserializeFromByteArray(builder, array);
	  ClientConfig cfg3 = builder.build();
	  
	  // text serde
	  serde = ProtoSerdeFactory.createSerde(2);
	  array = serde.serializeToByteArray(cfg2);
	  builder = ClientConfig.newBuilder();
	  serde.deserializeFromByteArray(builder, array);
	  ClientConfig cfg4 = builder.build();
	  
	  assertEquals(cfg1, cfg2);
	  assertEquals(cfg2, cfg3);
	  assertEquals(cfg3, cfg4);
	  
	  log.debug(cfg4.toString());
	  
	}
}
