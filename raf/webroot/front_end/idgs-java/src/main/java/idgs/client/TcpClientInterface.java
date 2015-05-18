
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.client;

import java.io.IOException;

public class TcpClientInterface {

  private TcpClient client;

  /**
   * friendly constructor
   * @param client
   */
  TcpClientInterface(TcpClient client) {
    this.client = client;
  }

  TcpClient getClient() {
    return this.client;
  }

  public int getId() {
    return this.client.getId();
  }

  public void close() {
    TcpClientPool.getInstance().putBack(client);
    client = null; // better for GC
  }

  public ResultCode send(ClientActorMessage msg) throws IOException {
    if (client == null) {
      return ResultCode.RC_ERROR;
    }
    SocketChannelHandlerContext ctx = new SocketChannelHandlerContext();
    ctx.setAttachment(msg);
    SocketChannelHandler handler = new SocketChannelHandler(ctx);
    client.setHandler(handler);
    client.write();
    return ResultCode.RC_OK;
  }

  public ClientActorMessage sendRecv(ClientActorMessage msg) throws IOException {
    return sendRecv(msg, -1);
  }
  
  public ClientActorMessage sendRecv(ClientActorMessage msg, long timeout) throws IOException {
    if (client == null) {
      return null;
    }
    client.setTimeout(timeout);
    send(msg);
    return receive();
  }

  public ClientActorMessage receive() throws IOException {
    if (client == null) {
      return null;
    }
    SocketChannelHandlerContext ctx = new SocketChannelHandlerContext();
    SocketChannelHandler handler = new SocketChannelHandler(ctx);
    client.setHandler(handler);
    client.read();
    ClientActorMessage msg = (ClientActorMessage) ctx.getAttachment();
    return msg;
  }
  
  public String toString() {
    return client.toString();
  }
}
