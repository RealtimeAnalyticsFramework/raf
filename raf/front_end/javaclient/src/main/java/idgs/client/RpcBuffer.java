
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.client;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

class TcpHeader {
  
  public static final int DEFAULT_MSG_HEADER_LENGTH = 4;
  
  public int size = DEFAULT_MSG_HEADER_LENGTH;
  
}

class ClientLogin {
  
  public static final String IDGS_COOKIE = "INTC";
  
  private byte[] cookie = IDGS_COOKIE.getBytes();    // must be IDGS_COOKIE('INTC')
  
  private byte serdes = 0;             // 0: protobuf binary, 1: protobuf text, 2: json
  
  public String toString() {
    StringBuilder builder = new StringBuilder();
    builder.append(new String(cookie));
    builder.append(",").append(serdes);
    return builder.toString();
  }
  
  public ByteBuffer toBuffer() {
    ByteBuffer buffer = ByteBuffer.allocate(cookie.length + 1).order(RpcBuffer.DEFAULT_BYTE_ORDER);
    buffer.put(cookie);
    buffer.put(serdes);
    return buffer;
  }
  
  public void setCookie(String str) {
    setCookie(str.getBytes());
  }
  
  public void setCookie(byte[] array) {
    this.cookie = array;
  }
  
  public void setSerdes(byte serdeType) {
    if(serdeType < 0 || serdeType > 2) {
      throw new IllegalArgumentException("illegal serde type, currently support 0, 1 ,2");
    }
    this.serdes = serdeType;
  }
}

public class RpcBuffer {
  
  public static final ByteOrder DEFAULT_BYTE_ORDER = ByteOrder.LITTLE_ENDIAN;

  private TcpHeader header = new TcpHeader();

  private ByteBuffer body;
  
  private int bodyCapacity;
  
  private ByteOrder byteOrder = DEFAULT_BYTE_ORDER;
  
  public RpcBuffer() {
    
  }
  
  public RpcBuffer(ByteOrder byteOrder) {
   this.byteOrder = byteOrder; 
  }
  
  public void setByteOrder(ByteOrder byteOrder) {
    this.byteOrder = byteOrder;
  }
  
  public ByteOrder getByteOrder() {
    return byteOrder;
  }
  
  public void reserveBuffer() {
    bodyCapacity = header.size;
    body = ByteBuffer.allocate(bodyCapacity).order(byteOrder);
  }
  
  public ByteBuffer getBody() {
    return body;
  }
  
  public TcpHeader getHeader() {
    return header;
  }
  
  public void setHeader(int bodyBytesLen) {
    if(body == null) {
      throw new NullPointerException("not allocate space for bytebuffer");
    }
    body.putInt(bodyBytesLen);
  }
  
  public void setBody(byte[] array) {
    if(body == null) {
      throw new NullPointerException("not allocate space for bytebuffer");
    }
    body.put(array);
  }
  
  public int getBodyLength() {
    return header.size;
  }
  
  public void setBodyLength(int len) {
    header.size = len;
  }
  
  public void encodeHeader() {
    reserveBuffer();
  }
  
  public void decodeHeader() {
    reserveBuffer();
  }
  
  public void clear() {
    if(body != null) { 
      body.clear();
    }
  }
  
  public void flip() {
    if(body != null) {
      body.flip();
    }
  }
  
  public byte[] array() {
    if(body != null) {
      return body.array();
    }
    return null;
  }
  
  public boolean hasRemaining() {
    return body != null && body.hasRemaining();
  }
  
}
