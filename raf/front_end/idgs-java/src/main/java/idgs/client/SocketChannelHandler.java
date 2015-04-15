
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.client;

import idgs.client.util.ProtoSerdeFactory;
import idgs.pb.PbRpcMessage.RpcMessage;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.util.Arrays;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class SocketChannelHandler {
  
  private static Log log = LogFactory.getLog(SocketChannelHandler.class);
  
  private SocketChannelHandlerContext context = new SocketChannelHandlerContext();
  
  public SocketChannelHandler() {
    
  }
  
  public SocketChannelHandler(SocketChannelHandlerContext ctx) {
    this.context = ctx;
  }
  
  public void setContext(SocketChannelHandlerContext context) {
    this.context = context;
  }
  
  public SocketChannelHandlerContext getContext() {
    return context;
  }
  
  /**
   * handle connect
   * @throws IOException
   */
  public void onConnected(SocketChannel channel) throws IOException {
    ClientLogin login = new ClientLogin();
    ByteBuffer buffer = login.toBuffer();
    buffer.flip();
    int writeClientLoginSize = channel.write(buffer);
    if(writeClientLoginSize > 0) {
      log.debug("writer client login content: " + new String(login.toString()));
      log.debug("byte size: " + writeClientLoginSize + ", order by " + RpcBuffer.DEFAULT_BYTE_ORDER.toString() + " bytes: " + Arrays.toString(buffer.array()));
    }
  }
  

  /**
   * handle read message
   * @param channel
   * @param ctx
   * @throws IOException
   */
  public void onRead(SocketChannel channel) throws IOException {
    RpcBuffer buffer = new RpcBuffer();
    buffer.decodeHeader(); // allocate default header size
    // read header
    int headerTotalBytes = 0, nBytes = 0;
    while (buffer.hasRemaining() && (nBytes = channel.read(buffer.getBody())) > 0) {
      headerTotalBytes += nBytes;
    }
    buffer.flip();
    if(headerTotalBytes > 0 ) {
      assert(headerTotalBytes == buffer.getBodyLength());
      int bodyLength = buffer.getBody().getInt();
      log.debug("read header content: " + bodyLength);
      log.debug("byte size: " + headerTotalBytes + ", order by " + buffer.getByteOrder().toString() + " bytes: " + Arrays.toString(buffer.array()));
      buffer.setBodyLength(bodyLength);
      buffer.decodeHeader(); // decode header
      int bodyTotalBytes = 0;
      while (buffer.hasRemaining()) {
        nBytes = channel.read(buffer.getBody());
        if(nBytes == -1) {
          log.error("read end of stream");
          throw new IOException("read end of stream");
        }
        bodyTotalBytes += nBytes;
      }
      buffer.flip();
      // read body
      if(bodyTotalBytes > 0) {
        assert(bodyTotalBytes == buffer.getBodyLength());
        RpcMessage.Builder builder = RpcMessage.newBuilder();
        byte[] bytes = buffer.array();
        ProtoSerdeFactory.createSerde(builder.getSerdesType().getNumber()).deserializeFromByteArray(builder, bytes);
//        log.debug("read body content: " + new String(bytes));
        log.debug("byte size: " + bodyTotalBytes + ", order by " + buffer.getByteOrder().toString() + " bytes: " + Arrays.toString(bytes));
        context.setAttachment(new ClientActorMessage(builder));
      } else {
        log.warn("read body nothing");
      }        
    } else {
      log.warn("read header nothing");
    } 
  }
  
  /**
   * handle write message
   * @param channel
   * @param ctx
   * @throws IOException
   */
  public void onWrite(SocketChannel channel) throws IOException {
    ClientActorMessage msg = (ClientActorMessage) context.getAttachment();
    if( msg != null) {
      // write header
      byte[] bodyBytes = msg.toBuffer();
      final int bodyBytesLen = bodyBytes.length;
      RpcBuffer buffer = new RpcBuffer();
      buffer.encodeHeader();
      buffer.setHeader(bodyBytesLen);
      buffer.flip();
      int writeHeaderBytes = channel.write(buffer.getBody());
      if(writeHeaderBytes > 0) {
        log.debug("writer header content: " + bodyBytesLen);
        log.debug("byte size: " + writeHeaderBytes + ", order by " + buffer.getByteOrder().toString() + " bytes: " + Arrays.toString(buffer.array()));
        buffer.setBodyLength(bodyBytesLen);
        buffer.encodeHeader();
        buffer.setBody(bodyBytes);
        buffer.flip();
        int writeBodyBytes = channel.write(buffer.getBody());
        if(writeBodyBytes > 0) {
          log.debug("writer body content: " + new String(bodyBytes));
          log.debug("byte size: " + bodyBytesLen + ", order by " + buffer.getByteOrder().toString() + " bytes: " + Arrays.toString(buffer.array()));
        } else {
          log.warn("write body nothing...");
        }
      } else {
        log.warn("write header nothing...");
      }
    }
  }
}
