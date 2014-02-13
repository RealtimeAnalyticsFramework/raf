
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.client;

import idgs.client.pb.PbClientConfig.ClientConfig;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;


public class ProtobufTest extends BaseClientTest {
	
	private static Log log = LogFactory.getLog(ProtobufTest.class);

	public void testBuildMessage() {
	  log.info("test how to create a java protobuf message");
		ClientConfig.Builder builder = ClientConfig.newBuilder();
		ClientConfig cfg = null;
		
		builder.setPoolSize(30);
		// build
		cfg = builder.build();
		assertEquals(30, cfg.getPoolSize());
		assertEquals(1, cfg.getThreadCount()); // default
		log.debug(cfg.toString());
		
		builder.setThreadCount(7);// not build
		assertEquals(1, cfg.getThreadCount());
		assertEquals(30, cfg.getPoolSize());
		log.debug(cfg.toString());

		// build
		cfg = builder.build();
		assertEquals(7, cfg.getThreadCount());
		assertEquals(30, cfg.getPoolSize());
		log.debug(cfg.toString());
	}

}
