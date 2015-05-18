
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.client;

import idgs.client.util.ProtoSerde;
import idgs.client.util.ProtoSerdeFactory;
import idgs.pb.PbRpcMessage.PayloadSerdes;
import idgs.pb.PbRpcMessage.RpcMessage;
import idgs.pb.PbRpcMessage.RpcMessage.NameValuePair;
import idgs.pb.PbRpcMessage.TransportChannel;

import java.util.Collection;
import java.util.Iterator;
import java.util.Map.Entry;

import com.google.common.collect.ArrayListMultimap;
import com.google.common.collect.Multimap;
import com.google.protobuf.ByteString;
import com.google.protobuf.Message;
import com.google.protobuf.Message.Builder;

public class ClientActorMessage {
  
  private RpcMessage.Builder rpcMsgBuilder;

  private Message payload = null;

  private Multimap<String, Message> attachments = ArrayListMultimap.create();

  private Multimap<String, ByteString> rawAttachments = ArrayListMultimap.create();

  /**
   * Construct sent ClientActorMessage
   */
  public ClientActorMessage() {
    this.rpcMsgBuilder = RpcMessage.getDefaultInstance().toBuilder();
  }
  
  /**
   * Construct ClientActorMessage by received RpcMessage 
   * @param builder
   */
  public ClientActorMessage(final RpcMessage.Builder builder) {
    if(builder == null) {
      throw new NullPointerException("rpc message builder can not be null");
    }
    this.rpcMsgBuilder = builder;
    // set rawAttachment
    final int size = rpcMsgBuilder.getAttachmentsCount();
    for (int i = 0; i < size; ++i) {
      NameValuePair pair = rpcMsgBuilder.getAttachments(i);
      rawAttachments.put(pair.getName(), pair.getValue());
    }
  }
  
  /**
   * encode the message to byte array.
   * @return
   */
  public byte[] toBuffer() {
    ProtoSerde serde = ProtoSerdeFactory.createSerde(getSerdesType().getNumber());
    // serialize payload into rpc message
    byte[] array = serde.serializeToByteArray(payload);
    rpcMsgBuilder.setPayload(ByteString.copyFrom(array));
    
    // serialize attachment into rpc message
    rpcMsgBuilder.clearAttachments(); // clear source attachments, re-put
    for(Iterator<Entry<String, Message>> it = attachments.entries().iterator(); it.hasNext(); ) {
      Entry<String, Message> entry = it.next();
      String key = entry.getKey();
      Message value = entry.getValue();
      NameValuePair.Builder builder = NameValuePair.newBuilder();
      ByteString byteString = ByteString.copyFrom(serde.serializeToByteArray(value));
      builder.setName(key).setValue(byteString);
      rpcMsgBuilder.addAttachments(builder);
      rawAttachments.put(key, byteString);
    }
    // serialize attachment to byte array
    return serde.serializeToByteArray(getRpcMessage());
  }
  /**
   * decode the payload message from rpc message
   * @param payloadBuilder
   * @return
   */
  public boolean parsePayload(Message.Builder payloadBuilder) {
    if (payloadBuilder == null) {
      return false;
    }
    ProtoSerde serde = ProtoSerdeFactory.createSerde(getSerdesType().getNumber());
    byte[] array = rpcMsgBuilder.getPayload().toByteArray();
    serde.deserializeFromByteArray(payloadBuilder, array);
    setPayload(payloadBuilder.build());
    return true;
  }

  /**
   * decode the message from attachments at first, if not exists, then decode
   * from rawAttachements secondly and put into attachments
   */
  public boolean parseAttachment(final String name, Message.Builder attachBuilder) {
    if (attachBuilder == null) {
      return false;
    }
    Collection<Message> attachValues = attachments.get(name);
    if (attachValues != null && attachValues.size() > 0) {
      attachBuilder.mergeFrom(attachValues.iterator().next());
      return true;
    }
    Collection<ByteString> rawAttachValues = rawAttachments.get(name);
    if (rawAttachValues == null || rawAttachValues.size() == 0) {
      return false;
    }
    ProtoSerde serde = ProtoSerdeFactory.createSerde(getSerdesType().getNumber());
    for(Iterator<ByteString> it = rawAttachValues.iterator(); it.hasNext() ; ) {
      serde.deserializeFromByteArray(attachBuilder, it.next().toByteArray());
      attachments.put(name, attachBuilder.build());
    }
    return true;
  }
  
  public String toString() {
    StringBuilder buf = new StringBuilder();
    buf.append("\r\n");
    buf.append("-------rpc message-------").append("\r\n");
    buf.append(getRpcMessage().toString()).append("\r\n");
    buf.append("-------payload-------").append("\r\n");
    if (payload != null) {
      buf.append(payload.toString()).append("\r\n");
    }
    buf.append("-------attachments-------").append("\r\n");
    Iterator<Message> iter = attachments.values().iterator();
    while(iter.hasNext()) {
      buf.append(iter.next().toString()).append("\r\n");
    }
    buf.append("-------rawattachments-------").append("\r\n");
    Iterator<ByteString> it = rawAttachments.values().iterator();
    while(it.hasNext()) {
      buf.append(it.next().toStringUtf8()).append("\r\n");
    }
    return buf.toString();
  }

  public RpcMessage getRpcMessage() {
    return rpcMsgBuilder.build();
  }
  
  public void setPayload(final Message payload) {
    if(payload == null) {
      throw new NullPointerException("payload cant not be null");
    }
    this.payload = payload;
  }
  
  public Message getPayload() {
    return this.payload;
  }

  public void setOperationName(final String op_name) {
    rpcMsgBuilder.setOperationName(op_name);
  }

  public String getOperationName() {
    return rpcMsgBuilder.getOperationName();
  }

  public int getSourceMemberId() {
    return rpcMsgBuilder.getSourceActorBuilder().getMemberId();
  }

  public void setSourceMemberId(int memberId) {
    rpcMsgBuilder.getSourceActorBuilder().setMemberId(memberId);
  }

  public String getSourceActorId() {
    return rpcMsgBuilder.getSourceActorBuilder().getActorId();
  }

  public void setSourceActorId(final String actorId) {
    rpcMsgBuilder.getSourceActorBuilder().setActorId(actorId);
  }

  public int getDestMemberId() {
    return rpcMsgBuilder.getDestActor().getMemberId();
  }

  public void setDestMemberId(int memberId) {
    rpcMsgBuilder.getDestActorBuilder().setMemberId(memberId);
  }

  public String getDestActorId() {
    return rpcMsgBuilder.getDestActorBuilder().getActorId();
  }

  public void setDestActorId(final String actorId) {
    rpcMsgBuilder.getDestActorBuilder().setActorId(actorId);
  }

  public void setChannel(TransportChannel value) {
    rpcMsgBuilder.setChannel(value);
  }

  public TransportChannel getChannel() {
    return rpcMsgBuilder.getChannel();
  }

  public void setSerdesType(final PayloadSerdes mode) {
    rpcMsgBuilder.setSerdesType(mode);
  }

  public PayloadSerdes getSerdesType() {
    return rpcMsgBuilder.getSerdesType();
  }

  public boolean parseProto(final byte[] protoString, Message msg) {
    if (msg == null) {
      return false;
    }
    ProtoSerde serde = ProtoSerdeFactory.createSerde(getSerdesType().getNumber());
    Builder builder = msg.toBuilder();
    serde.deserializeFromByteArray(builder, protoString);
    msg = builder.build();
    return true;
  }

  public Message getAttachement(final String name) {
    Collection<Message> attachValues = attachments.get(name);
    if (attachValues != null && attachValues.size() > 0) {
      return attachValues.iterator().next();
    }
    return null;
  }

  public Multimap<String, Message> getAttachements() {
    return attachments;
  }

  public void setAttachment(final String name, Message value) {
    attachments.put(name, value);
  }

  public void setRawAttachment(final String name, ByteString value) {
    rawAttachments.put(name, value);
  }
  
  public Multimap<String, ByteString> getRawAttachments() {
    return rawAttachments;
  }
}
