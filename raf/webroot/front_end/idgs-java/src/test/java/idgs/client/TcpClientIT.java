
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.client;

import idgs.client.ClientActorMessage;
import idgs.client.TcpClient;
import idgs.client.TcpClientInterface;
import idgs.store.pb.PbStoreService.InsertResponse;
import idgs.store.pb.PbStoreService.StoreResultCode;

import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class TcpClientIT extends BaseClientTest {

  private static Log log = LogFactory.getLog(TcpClientIT.class);
  
  /**
   * client send cfg file to server, server response it to client again
   */
  public void testSendRecv() {
    log.info("test client to connect with server, then insert a customer into store");
    ClientActorMessage requestActorMsg = createActorMessage();
    TcpClientInterface client = null;
    try {
      TcpClient tc = new TcpClient("127.0.0.1", 7700);
      tc.connect(true, 5, 3);
      if(tc.isConnected()) {
        client = new TcpClientInterface(tc);
      }
    } catch (IOException e) {
      log.info("possible reason: idgs server has not started... ");
      log.error(e.getMessage(),e);
    }
    if(client != null) {
      try {
        log.debug("-------------------request msg----------------------");
        log.debug(requestActorMsg.toString());
        ClientActorMessage responseActorMsg = client.sendRecv(requestActorMsg);
        InsertResponse.Builder builder = InsertResponse.newBuilder();
        responseActorMsg.parsePayload(builder);
        log.debug("-------------------response msg----------------------");
        log.debug(responseActorMsg.toString());
        InsertResponse response = builder.build();
        log.debug(response.toString());
        assertEquals(response.getResultCode(), StoreResultCode.SRC_SUCCESS);
      } catch(Exception e) {
        log.error(e.getMessage(), e);
      } finally {
        client.close();
      }
    }
  }
}
